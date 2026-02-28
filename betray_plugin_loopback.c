

#include "betray_plugin_api.h"
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <avrt.h>

// loopback-capture.cpp

/*
DWORD WINAPI LoopbackCaptureThreadFunction(void *pContext)
{
    LoopbackCaptureThreadFunctionArguments *pArgs = (LoopbackCaptureThreadFunctionArguments*)pContext;

    pArgs->hr = CoInitialize(NULL);
    if (FAILED(pArgs->hr)) {
        ERR(L"CoInitialize failed: hr = 0x%08x", pArgs->hr);
        return 0;
    }
    CoUninitializeOnExit cuoe;

    pArgs->hr = LoopbackCapture(
        pArgs->pMMDevice,
        pArgs->hFile,
        pArgs->bInt16,
        pArgs->hStartedEvent,
        pArgs->hStopEvent,
        &pArgs->nFrames
    );

    return 0;
}*/

HRESULT LoopbackCapture(IMMDevice *pMMDevice, HMMIO hFile, boolean bInt16, HANDLE hStartedEvent,  HANDLE hStopEvent, PUINT32 pnFrames)
{
    HRESULT hr;
    // activate an IAudioClient
    IAudioClient *pAudioClient;
    hr = pMMDevice->Activate( __uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);

    if(FAILED(hr))
	{
        ERR(L"IMMDevice::Activate(IAudioClient) failed: hr = 0x%08x", hr);
        return hr;
    }

//    ReleaseOnExit releaseAudioClient(pAudioClient);
    
    // get the default device periodicity
    REFERENCE_TIME hnsDefaultDevicePeriod;
    hr = pAudioClient->GetDevicePeriod(&hnsDefaultDevicePeriod, NULL);
    if(FAILED(hr)
	{
        ERR(L"IAudioClient::GetDevicePeriod failed: hr = 0x%08x", hr);
        return hr;
    }

    // get the default device format
    WAVEFORMATEX *pwfx;
    hr = pAudioClient->GetMixFormat(&pwfx);
    if(FAILED(hr))
	{
        ERR(L"IAudioClient::GetMixFormat failed: hr = 0x%08x", hr);
        return hr;
    }
 //   CoTaskMemFreeOnExit freeMixFormat(pwfx);

    if(bInt16)
	{
        // coerce int-16 wave format
        // can do this in-place since we're not changing the size of the format
        // also, the engine will auto-convert from float to int for us
        switch (pwfx->wFormatTag) {
            case WAVE_FORMAT_IEEE_FLOAT:
                pwfx->wFormatTag = WAVE_FORMAT_PCM;
                pwfx->wBitsPerSample = 16;
                pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
                pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
                break;

            case WAVE_FORMAT_EXTENSIBLE:
                {
                    // naked scope for case-local variable
                    PWAVEFORMATEXTENSIBLE pEx = reinterpret_cast<PWAVEFORMATEXTENSIBLE>(pwfx);
                    if (IsEqualGUID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, pEx->SubFormat)) {
                        pEx->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
                        pEx->Samples.wValidBitsPerSample = 16;
                        pwfx->wBitsPerSample = 16;
                        pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
                        pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
                    } else {
                        ERR(L"%s", L"Don't know how to coerce mix format to int-16");
                        return E_UNEXPECTED;
                    }
                }
                break;

            default:
                ERR(L"Don't know how to coerce WAVEFORMATEX with wFormatTag = 0x%08x to int-16", pwfx->wFormatTag);
                return E_UNEXPECTED;
        }
    }

    MMCKINFO ckRIFF = {0};
    MMCKINFO ckData = {0};


    // create a periodic waitable timer
 /*   HANDLE hWakeUp = CreateWaitableTimer(NULL, FALSE, NULL);
    if (NULL == hWakeUp) {
        DWORD dwErr = GetLastError();
        ERR(L"CreateWaitableTimer failed: last error = %u", dwErr);
        return HRESULT_FROM_WIN32(dwErr);
    }
    CloseHandleOnExit closeWakeUp(hWakeUp);*/

    UINT32 nBlockAlign = pwfx->nBlockAlign;
    *pnFrames = 0;
    
    // call IAudioClient::Initialize
    // note that AUDCLNT_STREAMFLAGS_LOOPBACK and AUDCLNT_STREAMFLAGS_EVENTCALLBACK
    // do not work together...
    // the "data ready" event never gets set
    // so we're going to do a timer-driven loop
    hr = pAudioClient->Initialize( AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK,  0, 0, pwfx, 0);
    if(FAILED(hr))
	{
        ERR(L"IAudioClient::Initialize failed: hr = 0x%08x", hr);
        return hr;
    }

    // activate an IAudioCaptureClient
    IAudioCaptureClient *pAudioCaptureClient;
    hr = pAudioClient->GetService( __uuidof(IAudioCaptureClient),(void**)&pAudioCaptureClient);
    if(FAILED(hr))
	{
        ERR(L"IAudioClient::GetService(IAudioCaptureClient) failed: hr = 0x%08x", hr);
        return hr;
    }
    ReleaseOnExit releaseAudioCaptureClient(pAudioCaptureClient);
    
    // register with MMCSS
    DWORD nTaskIndex = 0;
    HANDLE hTask = AvSetMmThreadCharacteristics(L"Audio", &nTaskIndex);
    if(NULL == hTask)
	{
        DWORD dwErr = GetLastError();
        ERR(L"AvSetMmThreadCharacteristics failed: last error = %u", dwErr);
        return HRESULT_FROM_WIN32(dwErr);
    }
    AvRevertMmThreadCharacteristicsOnExit unregisterMmcss(hTask);

    // set the waitable timer
    LARGE_INTEGER liFirstFire;
    liFirstFire.QuadPart = -hnsDefaultDevicePeriod / 2; // negative means relative time
    LONG lTimeBetweenFires = (LONG)hnsDefaultDevicePeriod / 2 / (10 * 1000); // convert to milliseconds
    BOOL bOK = SetWaitableTimer(hWakeUp, &liFirstFire, lTimeBetweenFires, NULL, NULL, FALSE);
    if(!bOK)
	{
        DWORD dwErr = GetLastError();
        ERR(L"SetWaitableTimer failed: last error = %u", dwErr);
        return HRESULT_FROM_WIN32(dwErr);
    }
    CancelWaitableTimerOnExit cancelWakeUp(hWakeUp);
    
    // call IAudioClient::Start
    hr = pAudioClient->Start();
    if (FAILED(hr)) {
        ERR(L"IAudioClient::Start failed: hr = 0x%08x", hr);
        return hr;
    }
    AudioClientStopOnExit stopAudioClient(pAudioClient);

    SetEvent(hStartedEvent);
    
    // loopback capture loop
    HANDLE waitArray[2] = { hStopEvent, hWakeUp };
    DWORD dwWaitResult;

    bool bDone = false;
    bool bFirstPacket = true;
    for (UINT32 nPasses = 0; !bDone; nPasses++) {
        // drain data while it is available
        UINT32 nNextPacketSize;
        for (
            hr = pAudioCaptureClient->GetNextPacketSize(&nNextPacketSize);
            SUCCEEDED(hr) && nNextPacketSize > 0;
            hr = pAudioCaptureClient->GetNextPacketSize(&nNextPacketSize)
        ) {
            // get the captured data
            BYTE *pData;
            UINT32 nNumFramesToRead;
            DWORD dwFlags;

            hr = pAudioCaptureClient->GetBuffer(
                &pData,
                &nNumFramesToRead,
                &dwFlags,
                NULL,
                NULL
                );
            if (FAILED(hr)) {
                ERR(L"IAudioCaptureClient::GetBuffer failed on pass %u after %u frames: hr = 0x%08x", nPasses, *pnFrames, hr);
                return hr;
            }

            if (bFirstPacket && AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY == dwFlags) {
                LOG(L"%s", L"Probably spurious glitch reported on first packet");
            } else if (0 != dwFlags) {
                LOG(L"IAudioCaptureClient::GetBuffer set flags to 0x%08x on pass %u after %u frames", dwFlags, nPasses, *pnFrames);
                return E_UNEXPECTED;
            }

            if (0 == nNumFramesToRead) {
                ERR(L"IAudioCaptureClient::GetBuffer said to read 0 frames on pass %u after %u frames", nPasses, *pnFrames);
                return E_UNEXPECTED;
            }

            LONG lBytesToWrite = nNumFramesToRead * nBlockAlign;
#pragma prefast(suppress: __WARNING_INCORRECT_ANNOTATION, "IAudioCaptureClient::GetBuffer SAL annotation implies a 1-byte buffer")
            LONG lBytesWritten = mmioWrite(hFile, reinterpret_cast<PCHAR>(pData), lBytesToWrite);
            if (lBytesToWrite != lBytesWritten) {
                ERR(L"mmioWrite wrote %u bytes on pass %u after %u frames: expected %u bytes", lBytesWritten, nPasses, *pnFrames, lBytesToWrite);
                return E_UNEXPECTED;
            }
            *pnFrames += nNumFramesToRead;

            hr = pAudioCaptureClient->ReleaseBuffer(nNumFramesToRead);
            if (FAILED(hr)) {
                ERR(L"IAudioCaptureClient::ReleaseBuffer failed on pass %u after %u frames: hr = 0x%08x", nPasses, *pnFrames, hr);
                return hr;
            }

            bFirstPacket = false;
        }

        if (FAILED(hr)) {
            ERR(L"IAudioCaptureClient::GetNextPacketSize failed on pass %u after %u frames: hr = 0x%08x", nPasses, *pnFrames, hr);
            return hr;
        }

        dwWaitResult = WaitForMultipleObjects(
            ARRAYSIZE(waitArray), waitArray,
            FALSE, INFINITE
        );

        if (WAIT_OBJECT_0 == dwWaitResult) {
            LOG(L"Received stop event after %u passes and %u frames", nPasses, *pnFrames);
            bDone = true;
            continue; // exits loop
        }

        if (WAIT_OBJECT_0 + 1 != dwWaitResult) {
            ERR(L"Unexpected WaitForMultipleObjects return value %u on pass %u after %u frames", dwWaitResult, nPasses, *pnFrames);
            return E_UNEXPECTED;
        }
    } // capture loop

    hr = FinishWaveFile(hFile, &ckData, &ckRIFF);
    if (FAILED(hr)) {
        // FinishWaveFile does it's own logging
        return hr;
    }
    
    return hr;
}


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
