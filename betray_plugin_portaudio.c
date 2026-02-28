//
//  betray_plugin_portaudio.c
//  BetrayPlugins
//
//  Created by Kyle Fleming on 6/6/17.
//  Copyright © 2017 Symmetry Labs. All rights reserved.
//

#ifdef __APPLE_CC__
#include <CoreAudio/CoreAudio.h>
#include <TargetConditionals.h>
#if TARGET_RT_BIG_ENDIAN
#   define FourCC2Str(fourcc) (const char[]){*((char*)&fourcc), *(((char*)&fourcc)+1), *(((char*)&fourcc)+2), *(((char*)&fourcc)+3),0}
#else
#   define FourCC2Str(fourcc) (const char[]){*(((char*)&fourcc)+3), *(((char*)&fourcc)+2), *(((char*)&fourcc)+1), *(((char*)&fourcc)+0),0}
#endif

void logOSStatus(OSStatus status) {
	fprintf(stderr, ", error code: %s\n", FourCC2Str(status));
}
#endif

#include <stdlib.h>
#include <stdatomic.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#include <portaudio.h>
#include <pa_ringbuffer.h>

#include "betray_plugin_api.h"

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

#define SAMPLE_RATE (44100)
#define RING_BUFFER_FRAME_SIZE 4096

#define ENABLE_OUTPUT 0

#define INPUT_CHANNELS 2
#define OUTPUT_CHANNELS 0

bool initializationErrorOccurred = false;

PaStream *stream = NULL;

atomic_ulong status;

PaUtilRingBuffer inputRingBuffer;
PaUtilRingBuffer outputRingBuffer;

#if ENABLE_OUTPUT
size_t outputSampleSizeActual;

static size_t computeSampleSizeFromFormat( PaSampleFormat format )
{
	switch( format & (~paNonInterleaved) ) {
		case paFloat32: return 4;
		case paInt32: return 4;
		case paInt24: return 3;
		case paInt16: return 2;
		case paInt8: case paUInt8: return 1;
		default: return 0;
	}
}
#endif

static ring_buffer_size_t computeSampleSizeFromFormatPow2( PaSampleFormat format )
{
	switch( format & (~paNonInterleaved) ) {
		case paFloat32: return 4;
		case paInt32: return 4;
		case paInt24: return 4;
		case paInt16: return 2;
		case paInt8: case paUInt8: return 1;
		default: return 0;
	}
}

static int betrayPortAudioCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
	if (INPUT_CHANNELS > 0) {
		ring_buffer_size_t framesAvailable = PaUtil_GetRingBufferWriteAvailable(&inputRingBuffer);
		ring_buffer_size_t framesToTransfer;
		
		/* check for underflow */
		if (framesAvailable < framesPerBuffer) {
			atomic_fetch_or(&status, paInputOverflow);
			framesToTransfer = framesAvailable;
		} else {
			framesToTransfer = (ring_buffer_size_t)framesPerBuffer;
		}
		
		/* Copy the data from the audio input to the application ring buffer. */
//		printf("reading %d\n", framesToTransfer);
		ring_buffer_size_t framesTransferred = PaUtil_WriteRingBuffer(&inputRingBuffer, inputBuffer, framesToTransfer);
		assert( framesToTransfer == framesTransferred );
	}
	
#if ENABLE_OUTPUT
	if (OUTPUT_CHANNELS > 0) {
		ring_buffer_size_t framesAvailable = PaUtil_GetRingBufferReadAvailable(&outputRingBuffer);
		ring_buffer_size_t framesToTransfer;
		
		/* check for underflow */
		if (framesAvailable < framesPerBuffer) {
			/* zero out the end of the output buffer that we do not have data for */
			framesToTransfer = framesAvailable;
			
			size_t bytesPerFrame = outputSampleSizeActual * OUTPUT_CHANNELS;
			size_t offsetInBytes = framesToTransfer * bytesPerFrame;
			size_t countInBytes = (framesPerBuffer - framesToTransfer) * bytesPerFrame;
			bzero(((char *)outputBuffer) + offsetInBytes, countInBytes);
			
			atomic_fetch_or(&status, paOutputUnderflow);
			framesToTransfer = framesAvailable;
		} else {
			framesToTransfer = (ring_buffer_size_t)framesPerBuffer;
		}
		
		/* copy the data */
//		printf( "writing %d\n", framesToTransfer );
		ring_buffer_size_t framesTransferred = PaUtil_ReadRingBuffer(&outputRingBuffer, outputBuffer, framesToTransfer);
		assert(framesToTransfer == framesTransferred);
	}
#endif

	return paContinue;
}

uint betray_audio_read_func(void *data, uint type, uint buffer_size)
{
	if (type != BETRAY_TYPE_FLOAT32) { return 0; }
	
	if (buffer_size == 0) return 0;
	
	ring_buffer_size_t framesAvailable = PaUtil_GetRingBufferReadAvailable(&inputRingBuffer);
	ring_buffer_size_t framesToTransfer = (ring_buffer_size_t) framesAvailable < buffer_size ? framesAvailable : buffer_size;
	ring_buffer_size_t framesTransferred = PaUtil_ReadRingBuffer(&inputRingBuffer, data, framesToTransfer);
	
	return framesTransferred;
}


#ifdef __APPLE_CC__
AudioObjectID getDefaultInputDevice()
{
	AudioObjectID defaultInputDevice;
	AudioObjectPropertyAddress defaultInputDeviceProperty = {kAudioHardwarePropertyDefaultInputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster};
	UInt32 dataSize = sizeof(AudioObjectID);
	OSStatus status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &defaultInputDeviceProperty, 0, NULL, &dataSize, &defaultInputDevice);
	if (status != kAudioHardwareNoError) { fprintf(stderr, "Error: couldn't get default input device for audio system object"); logOSStatus(status); return kAudioObjectUnknown; }
	return defaultInputDevice;
}
AudioObjectID getDefaultOutputDevice()
{
	AudioObjectID defaultOutputDevice;
	AudioObjectPropertyAddress defaultOutputDeviceProperty = {kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster};
	UInt32 dataSize = sizeof(AudioObjectID);
	OSStatus status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &defaultOutputDeviceProperty, 0, NULL, &dataSize, &defaultOutputDevice);
	if (status != kAudioHardwareNoError) { fprintf(stderr, "Error: couldn't get default output device for audio system object"); logOSStatus(status); return kAudioObjectUnknown; }
	return defaultOutputDevice;
}
#endif

bool defaultInputDeviceChanged()
{
#ifdef __APPLE_CC__
	static AudioObjectID defaultInputDevice = kAudioObjectSystemObject;
	AudioObjectID inputDevice = getDefaultInputDevice();
	bool changed = defaultInputDevice != inputDevice && defaultInputDevice != kAudioObjectSystemObject;
	if (defaultInputDevice != inputDevice || defaultInputDevice == kAudioObjectSystemObject) {
		defaultInputDevice = inputDevice;
	}
	return changed;
#else
	return false;
#endif
}

bool defaultOutputDeviceChanged()
{
#ifdef __APPLE_CC__
	static AudioObjectID defaultOutputDevice = kAudioObjectSystemObject;
	AudioObjectID outputDevice = getDefaultOutputDevice();
	bool changed = defaultOutputDevice != outputDevice && defaultOutputDevice != kAudioObjectSystemObject;
	if (defaultOutputDevice != outputDevice || defaultOutputDevice == kAudioObjectSystemObject) {
		defaultOutputDevice = outputDevice;
	}
	return changed;
#else
	return false;
#endif
}

PaError initializePortAudio()
{
	PaError err = Pa_Initialize();
	if (err != paNoError) { fprintf(stderr, "Error: failure initializing portaudio: %s\n", Pa_GetErrorText(err)); return err; }
	
	PaDeviceIndex currentInputDevice = Pa_GetDefaultInputDevice();
	if (currentInputDevice == paNoDevice) { fprintf(stderr, "Error: failure getting default portaudio input device\n"); return paInternalError; }
	
	const PaDeviceInfo *currentInputDeviceInfo = Pa_GetDeviceInfo(currentInputDevice);
	PaStreamParameters inputParameters = {currentInputDevice, INPUT_CHANNELS, paFloat32, currentInputDeviceInfo->defaultLowInputLatency, NULL};

	PaStreamParameters *outputParametersPtr = NULL;
#if ENABLE_OUTPUT
	PaStreamParameters outputParameters;
	outputParametersPtr = &outputParameters;
#endif
	
	err = Pa_OpenStream(&stream, &inputParameters, outputParametersPtr, SAMPLE_RATE, paFramesPerBufferUnspecified, paNoFlag, betrayPortAudioCallback, NULL);
	if (err != paNoError) { fprintf(stderr, "Error: failure opening default portaudio stream: %s\n", Pa_GetErrorText(err)); return err; }
	
	err = Pa_StartStream(stream);
	if (err != paNoError) { fprintf(stderr, "Error: failure starting portaudio stream: %s\n", Pa_GetErrorText(err)); return err; }
	
	return paNoError;
}

PaError terminatePortAudio()
{
	stream = NULL;
	PaError err = Pa_Terminate();
	if (err != paNoError) { fprintf(stderr, "Error: failure terminating portaudio: %s\n", Pa_GetErrorText(err)); return err; }
	return paNoError;
}

void controller_plugin_callback_main(BInputState *input)
{
	if (initializationErrorOccurred) { return; }
	if (defaultInputDeviceChanged() || defaultOutputDeviceChanged()) {
		if (terminatePortAudio() != paNoError || initializePortAudio() != paNoError) {
			assert(false);
			initializationErrorOccurred = true;
			return;
		}
	}
}

void betray_plugin_init(void)
{
	ring_buffer_size_t inputSampleSizePow2 = computeSampleSizeFromFormatPow2(paFloat32);
	void *data = calloc(RING_BUFFER_FRAME_SIZE, inputSampleSizePow2 * INPUT_CHANNELS);
	if (!data) { fprintf(stderr, "Error: failure allocating portaudio input ring buffer\n"); return; }
	PaError err = PaUtil_InitializeRingBuffer(&inputRingBuffer, inputSampleSizePow2 * INPUT_CHANNELS, RING_BUFFER_FRAME_SIZE, data);
	assert(!err);
	if (err != paNoError) { fprintf(stderr, "Error: failure initializing portaudio input ring buffer: %s\n", Pa_GetErrorText(err)); return; }
	
	PaUtil_FlushRingBuffer(&inputRingBuffer);
	bzero(inputRingBuffer.buffer, inputRingBuffer.bufferSize * inputRingBuffer.elementSizeBytes);
	
#if ENABLE_OUTPUT
	if (OUTPUT_CHANNELS > 0) {
		ring_buffer_size_t outputSampleSizePow2 = computeSampleSizeFromFormatPow2(paFloat32);
		data = calloc(RING_BUFFER_FRAME_SIZE, outputSampleSizePow2 * OUTPUT_CHANNELS);
		if (!data) { fprintf(stderr, "Error: failure allocating portaudio output ring buffer\n"); return; }
		err = PaUtil_InitializeRingBuffer(&outputRingBuffer, outputSampleSizePow2 * OUTPUT_CHANNELS, RING_BUFFER_FRAME_SIZE, data);
		assert(!err);
		if (err != paNoError) { fprintf(stderr, "Error: failure initializing portaudio output ring buffer: %s\n", Pa_GetErrorText(err)); return; }
		
		PaUtil_FlushRingBuffer(&outputRingBuffer);
		bzero(outputRingBuffer.buffer, outputRingBuffer.bufferSize * outputRingBuffer.elementSizeBytes);
		PaUtil_AdvanceRingBufferWriteIndex(&outputRingBuffer, RING_BUFFER_FRAME_SIZE);
		
		outputSampleSizeActual = computeSampleSizeFromFormat(paFloat32);
	}
#endif

	if (initializePortAudio() != paNoError) {
		assert(false);
		initializationErrorOccurred = true;
		return;
	}
	
	betray_plugin_callback_set_main(controller_plugin_callback_main);
	
	uint audio_unit = betray_plugin_audio_unit_create();
	
	float vectors[6] = {-1, 0, 0, 1, 0, 0};
	betray_plugin_callback_set_audio_read(audio_unit, betray_audio_read_func, 2, vectors);
	
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

void betray_plugin_deinit()
{
	terminatePortAudio();
}
