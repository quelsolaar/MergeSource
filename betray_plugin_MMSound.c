

#include "betray_plugin_api.h"
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <math.h>

#pragma comment(lib, "WinMM.lib")
typedef unsigned int uint;


extern uint betray_audio_util_sound_create(uint type, uint stride, uint length, uint frequency, void *data, char *name);
extern void betray_audio_util_sound_destroy(uint sound);
extern uint betray_audio_util_sound_play(uint sound, float *pos, float *vector, float speed, float volume, boolean loop, boolean ambient, boolean auto_delete);
extern void betray_audio_util_sound_set(uint source, float *pos, float *vector, float speed, float volume, boolean loop, boolean ambient);
extern boolean betray_audio_util_sound_is_playing(uint source);
extern void betray_audio_util_sound_stop(uint source);
extern uint	betray_audio_util_stream_create(uint frequency, float *pos, float *vector,  float volume, boolean ambient);
extern void	betray_audio_util_stream_destroy(uint stream);
extern void	betray_audio_util_stream_feed(uint stream, uint type, uint stride, uint length, void *data);
extern uint	betray_audio_util_stream_buffer_left(uint stream);
extern void	betray_audio_util_stream_set(uint stream, float *pos, float *vector,  float volume, boolean ambient);
extern void betray_audio_util_update_callback(void *data, uint length, uint padding, float *vec);
extern void betray_audio_util_time_callback(uint length);
extern void betray_audio_util_listener(float *pos, float *vector, float *forward, float *side, float scale, float speed_of_sound);
extern void betray_audio_util_master_volume_set(float volume);
extern float betray_audio_util_master_volume_get();
extern void betray_audio_util_master_volume_silence_cutoff(float volume);

uint controller_setting_id;

#define BETRAY_AUDIO_BLOCK_COUNT 8
#define BETRAY_AUDIO_BLOCK_BYTE_SIZE 4096

static HWAVEIN	b_audio_win_device_in;
static uint		b_audio_win_next_free_in = 0;
static uint		b_audio_win_blocks_free_in = BETRAY_AUDIO_BLOCK_COUNT;

static uint		b_audio_win_next_collected_in = 0;
static uint		b_audio_win_blocks_collected_in = 0;
static uint		b_audio_win_collected_in_progress = 0;

static HWAVEOUT	b_audio_win_device_out;
static uint		b_audio_win_next_free_out = 0;
static uint		b_audio_win_blocks_free_out = BETRAY_AUDIO_BLOCK_COUNT;


static CRITICAL_SECTION b_audio_win_mutex;

static WAVEHDR	b_audio_win_headers_in[BETRAY_AUDIO_BLOCK_COUNT];
static WAVEHDR	b_audio_win_headers_out[BETRAY_AUDIO_BLOCK_COUNT];
static void		*b_audio_win_blocks_in[BETRAY_AUDIO_BLOCK_COUNT];
static void		*b_audio_win_blocks_out[BETRAY_AUDIO_BLOCK_COUNT];

void betray_audio_update_callback(void *data, uint length, uint padding, float *vec);
void betray_audio_time_callback(uint length);

static void CALLBACK waveOutProc(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{

	if(uMsg != WOM_DONE)
		return;
    EnterCriticalSection(&b_audio_win_mutex);
    b_audio_win_blocks_free_out++;
    LeaveCriticalSection(&b_audio_win_mutex);
}

static void CALLBACK waveInProc( HWAVEIN hwi, UINT uMsg, DWORD *dwInstance, DWORD *dwParam1, DWORD *dwParam2)
{
    if(uMsg != WIM_DATA)
		return;
    EnterCriticalSection(&b_audio_win_mutex);
    
	b_audio_win_blocks_collected_in++;
    LeaveCriticalSection(&b_audio_win_mutex);
}

void controller_plugin_callback_main(BInputState *input)
{
	float left[3] = {-1, 0, 0};
	float right[3] = {1, 0, 0};
	float volume;

	uint in_blocks, out_blocks;

    EnterCriticalSection(&b_audio_win_mutex);
	in_blocks = b_audio_win_blocks_free_in;
    b_audio_win_blocks_free_in = 0;
	out_blocks = b_audio_win_blocks_free_out;
	b_audio_win_blocks_free_out = 0;
    LeaveCriticalSection(&b_audio_win_mutex);

	volume = betray_settings_slider_get(controller_setting_id);
	volume = 1;

//	return;
	while(out_blocks > 0)
	{		
		if(b_audio_win_headers_out[b_audio_win_next_free_out].dwFlags & WHDR_PREPARED) 
			 waveOutUnprepareHeader(b_audio_win_device_out, &b_audio_win_headers_out[b_audio_win_next_free_out], sizeof(WAVEHDR));
		ZeroMemory(&b_audio_win_headers_out[b_audio_win_next_free_out], sizeof(WAVEHDR));
		b_audio_win_headers_out[b_audio_win_next_free_out].dwBufferLength = BETRAY_AUDIO_BLOCK_BYTE_SIZE;
		b_audio_win_headers_out[b_audio_win_next_free_out].lpData = b_audio_win_blocks_out[b_audio_win_next_free_out];
		betray_audio_util_update_callback(&((short *)b_audio_win_blocks_out[b_audio_win_next_free_out])[1], BETRAY_AUDIO_BLOCK_BYTE_SIZE / 4, 2, left);
		betray_audio_util_update_callback(((short *)b_audio_win_blocks_out[b_audio_win_next_free_out]), BETRAY_AUDIO_BLOCK_BYTE_SIZE / 4, 2, right);
		betray_audio_util_time_callback(BETRAY_AUDIO_BLOCK_BYTE_SIZE / 4);
		waveOutPrepareHeader(b_audio_win_device_out, &b_audio_win_headers_out[b_audio_win_next_free_out], sizeof(WAVEHDR));
		waveOutWrite(b_audio_win_device_out, &b_audio_win_headers_out[b_audio_win_next_free_out], sizeof(WAVEHDR));
		b_audio_win_next_free_out = (b_audio_win_next_free_out + 1) % BETRAY_AUDIO_BLOCK_COUNT;
		out_blocks--;
	}
	while(in_blocks > 0)
	{		
		uint i;
		if(b_audio_win_headers_in[b_audio_win_next_free_in].dwFlags & WHDR_PREPARED) 
			 waveInUnprepareHeader(b_audio_win_device_in, &b_audio_win_headers_in[b_audio_win_next_free_in], sizeof(WAVEHDR));
		ZeroMemory(&b_audio_win_headers_in[b_audio_win_next_free_in], sizeof(WAVEHDR));
		b_audio_win_headers_in[b_audio_win_next_free_in].dwBufferLength = BETRAY_AUDIO_BLOCK_BYTE_SIZE;
		b_audio_win_headers_in[b_audio_win_next_free_in].lpData = b_audio_win_blocks_in[b_audio_win_next_free_in];
		waveInPrepareHeader(b_audio_win_device_in, &b_audio_win_headers_in[b_audio_win_next_free_in], sizeof(WAVEHDR));
		waveInAddBuffer(b_audio_win_device_in, &b_audio_win_headers_in[b_audio_win_next_free_in], sizeof(WAVEHDR));
		b_audio_win_next_free_in = (b_audio_win_next_free_in + 1) % BETRAY_AUDIO_BLOCK_COUNT;
		in_blocks--;
	}
}
/*
void writeAudioBlock(HWAVEOUT hWaveOut, LPSTR block, DWORD size)
{
    WAVEHDR header;
    ZeroMemory(&header, sizeof(WAVEHDR));
    header.dwBufferLength = size;
    header.lpData = block;
    waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
    Sleep(500);
    while(waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING)
    Sleep(100);
}*/

/*
MMRESULT waveInAddBuffer(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh
);
*/

//static void CALLBACK waveOutProc(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)

uint betray_audio_read_func(uint8 *data, uint type, uint buffer_size)
{
	int8 *pint8;
	int16 *pint16, *read;
	int32 *pint32;
	float *preal32;
	uint i;
	buffer_size *= 2;
	for(i = 0; i < buffer_size && b_audio_win_blocks_collected_in != 0;)
	{
		
		read = b_audio_win_blocks_in[b_audio_win_next_collected_in];
		switch(type)
		{
			case BETRAY_TYPE_INT8 :
				pint8 = data;
				for(; i < buffer_size && b_audio_win_collected_in_progress < BETRAY_AUDIO_BLOCK_BYTE_SIZE / 2; i++)
				{
				pint8[i] = (uint8)(read[b_audio_win_collected_in_progress] / 256);
					b_audio_win_collected_in_progress++;
				}
			break;
			case BETRAY_TYPE_INT16 :
				pint16 = data;
				for(; i < buffer_size && b_audio_win_collected_in_progress < BETRAY_AUDIO_BLOCK_BYTE_SIZE / 2; i++)
				{
					pint16[i] = read[b_audio_win_collected_in_progress];
					b_audio_win_collected_in_progress++;
				}
			break;
			case BETRAY_TYPE_INT32 :
				pint32 = data;
				for(; i < buffer_size && b_audio_win_collected_in_progress < BETRAY_AUDIO_BLOCK_BYTE_SIZE / 2; i++)
				{
					pint32[i] = (int32)read[b_audio_win_collected_in_progress] * 256 * 256;
					b_audio_win_collected_in_progress++;
				}
			break;
			case BETRAY_TYPE_FLOAT32 :
				preal32 = data;
				for(; i < buffer_size && b_audio_win_collected_in_progress < BETRAY_AUDIO_BLOCK_BYTE_SIZE / 2; i++)
				{
					preal32[i] = (int32)read[b_audio_win_collected_in_progress] / (128.0 * 256.0 - 1.0);
					b_audio_win_collected_in_progress++;
				}
			break;
		}

	/*	for(; i < buffer_size && b_audio_win_collected_in_progress < BETRAY_AUDIO_BLOCK_BYTE_SIZE / 2; i++)
		{
			d[i * channel] = read[b_audio_win_collected_in_progress];
			b_audio_win_collected_in_progress++;
			d[i * channel + 1] = read[b_audio_win_collected_in_progress + 1];
			b_audio_win_collected_in_progress++;
		}*/
		if(b_audio_win_collected_in_progress == BETRAY_AUDIO_BLOCK_BYTE_SIZE / 2)
		{
			/* next block */
			b_audio_win_next_collected_in = (b_audio_win_next_collected_in + 1) % BETRAY_AUDIO_BLOCK_COUNT;
		    EnterCriticalSection(&b_audio_win_mutex);
			b_audio_win_blocks_collected_in--;
		    LeaveCriticalSection(&b_audio_win_mutex);
			b_audio_win_blocks_free_in++;
			b_audio_win_collected_in_progress = 0;
		}
	}
	return i / 2;
}
/*
void test_init_fun()
{
	unsigned long result;
	HWAVEIN       inHandle;
	WAVEFORMATEX  settings;
	settings.nSamplesPerSec = 44100;
	settings.wBitsPerSample = 16;
	settings.nChannels = 2;
	settings.cbSize = 0;
	settings.wFormatTag = WAVE_FORMAT_PCM;
	settings.nBlockAlign = settings.nChannels * (settings.wBitsPerSample / 8);
	settings.nAvgBytesPerSec = settings.nSamplesPerSec * settings.nBlockAlign;
	if(waveInOpen(&b_audio_win_device_in, WAVE_MAPPER, &settings, waveInProc, 0, CALLBACK_FUNCTION))
		printf("There was an error opening the preferred Digital Audio In device!\r\n");
	waveInStart(b_audio_win_device_in);
}*/

void betray_plugin_init(void)
{
	float vectors[6] = {-1, 0, 0, 1, 0, 0};
    WAVEFORMATEX settings; /* look this up in your documentation */
    MMRESULT result;/* for waveOut return values */
	uint i, audio_unit;
    settings.nSamplesPerSec = 44100; /* sample rate */
    settings.wBitsPerSample = 16; /* sample size */
    settings.nChannels = 2; /* channels*/
    settings.cbSize = 0; /* size of _extra_ info */
    settings.wFormatTag = WAVE_FORMAT_PCM;
    settings.nBlockAlign = (settings.wBitsPerSample >> 3) * settings.nChannels;
    settings.nAvgBytesPerSec = settings.nBlockAlign * settings.nSamplesPerSec;

    if(waveOutOpen(&b_audio_win_device_out, WAVE_MAPPER, &settings, waveOutProc, 0, CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
	{
		fprintf(stderr, "Failed to open audio device\n");
		return;
    }

/*	settings.nSamplesPerSec = 44100;
	settings.wBitsPerSample = 16;
	settings.nChannels = 2;
	settings.cbSize = 0;
	settings.wFormatTag = WAVE_FORMAT_PCM;
	settings.nBlockAlign = settings.nChannels * (settings.wBitsPerSample / 8);
	settings.nAvgBytesPerSec = settings.nSamplesPerSec * settings.nBlockAlign;
*/
	audio_unit = betray_plugin_audio_unit_create();

	/* Open the preferred Digital Audio In device */
	if(waveInOpen(&b_audio_win_device_in, WAVE_MAPPER, &settings, waveInProc, 0, CALLBACK_FUNCTION))
		printf("There was an error opening the preferred Digital Audio In device!\r\n");
	else
	{
		waveInStart(b_audio_win_device_in);
		betray_plugin_callback_set_audio_read(audio_unit, betray_audio_read_func, 2, vectors);
	}


	InitializeCriticalSection(&b_audio_win_mutex);
	for(i = 0; i < BETRAY_AUDIO_BLOCK_COUNT; i++)
	{
		ZeroMemory(&b_audio_win_headers_in[i], sizeof(WAVEHDR));
		ZeroMemory(&b_audio_win_headers_out[i], sizeof(WAVEHDR));
		b_audio_win_blocks_in[i] = malloc(BETRAY_AUDIO_BLOCK_BYTE_SIZE);
		b_audio_win_blocks_out[i] = malloc(BETRAY_AUDIO_BLOCK_BYTE_SIZE);
	}

	controller_setting_id = betray_settings_create(BETRAY_ST_SLIDER, "MM Sound Volume", 0, NULL);
	betray_settings_slider_set(controller_setting_id, 1.0);
	betray_plugin_callback_set_main(controller_plugin_callback_main);

	betray_plugin_callback_set_audio_sound_create(audio_unit, betray_audio_util_sound_create);
	betray_plugin_callback_set_audio_sound_destroy(audio_unit, betray_audio_util_sound_destroy);
	betray_plugin_callback_set_audio_sound_play(audio_unit, betray_audio_util_sound_play);
	betray_plugin_callback_set_audio_sound_set(audio_unit, betray_audio_util_sound_set);
	betray_plugin_callback_set_audio_sound_is_playing(audio_unit, betray_audio_util_sound_is_playing);
	betray_plugin_callback_set_audio_sound_stop(audio_unit, betray_audio_util_sound_stop);

	betray_plugin_callback_set_audio_stream_create(audio_unit, betray_audio_util_stream_create);
	betray_plugin_callback_set_audio_stream_destroy(audio_unit, betray_audio_util_stream_destroy);
	betray_plugin_callback_set_audio_stream_feed(audio_unit, betray_audio_util_stream_feed);
	betray_plugin_callback_set_audio_stream_buffer_left(audio_unit, betray_audio_util_stream_buffer_left);
	betray_plugin_callback_set_audio_stream_set(audio_unit, betray_audio_util_stream_set);

	betray_plugin_callback_set_audio_listener(audio_unit, betray_audio_util_listener);
}
