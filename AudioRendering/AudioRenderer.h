#pragma once
/*Audio samples contain values of amplitude (loudness). If we have a sample rate of 44100Hz, 
frequency is derived from the variations in these amplitudes considering that two 
contiguous samples are 1/44100 seconds apart.*/

#include "RtAudio.h"
#include <embree3/rtcore.h>
#include <embree3/rtcore_common.h>
#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include "Scene.h"
#include "Camera.h"
#include "Source.h"

#include<random>
#include<cmath>
#include<chrono>
#include <mutex>

#define LISTENER_SPHERE_RADIUS 2.0f
#define NUMBER_OF_RAYS 10000
//Speed of sound in the air at 20 °C in m/s
#define SPEED_OF_SOUND 343
#define SAMPLE_FORMAT RTAUDIO_SINT16

typedef signed short SAMPLE_TYPE;

typedef struct rayHistory{
	float travelled_distance;
	float remaining_energy_factor;
	int reflection_num;
} rayHistory;

typedef struct audioPath {
	float travelled_distance;
	float remaining_energy_factor;
} audioPath;

typedef struct audioPaths{
	audioPath * ptr;
	size_t * size;
	//paths are used by audio library thread and main thread. (And possibly multiple rayCasting threads)
	std::mutex * mutex;
} audioPaths;

typedef struct audioCallbackData {
	unsigned int * pos;
	/*A buffer to store old samples to use in posterior calculations.
	To store 1 second of samples size should be: (sampleRate x num_channels x sample_num_bytes)*/
	unsigned int * samplesRecordBufferSize;
	SAMPLE_TYPE * samplesRecordBuffer;
	audioPaths * paths;
} audioCallbackData;

typedef struct streamParameters {
	RtAudio::StreamParameters *iParams, *oParams;
	unsigned int * bufferFrames;
	RtAudio::StreamOptions * options;
} streamParameters;

class AudioRenderer {
public:
	//buffer to store all frames up to n seconds
	RtAudio * audioApi;
	audioPaths * currentPaths; //paths result of the audio render
	streamParameters * streamParams;
	unsigned int * bufferBytes;
	//audioCallbackData * audioData;
public:
	AudioRenderer();
	void render(Scene * scene, Camera * camera, Source * source);
	~AudioRenderer();
};