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
#include "CircularBuffer.h"
//#include "thread_pool.hpp"

#include<random>
#include<cmath>
#include<chrono>
#include <mutex>

#define LISTENER_SPHERE_RADIUS 2.0f
#define NUMBER_OF_RAYS 1000000
#define SOURCE_POWER 100000.0f
//Speed of sound in the air at 20 °C in m/s
#define SPEED_OF_SOUND 343
//Time that an audio sample takes in seconds
#define SAMPLE_RATE 16000
#define SAMPLE_DELTA_T 1 / SAMPLE_RATE
//#define SAMPLE_FORMAT RTAUDIO_SINT16
#define SAMPLE_FORMAT RTAUDIO_FLOAT32

//typedef signed short SAMPLE_TYPE;
typedef float SAMPLE_TYPE;

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
	size_t size;
	//paths are used by audio library thread and main thread. (And possibly multiple rayCasting threads)
	std::mutex * mutex;
} audioPaths;

typedef struct audioCallbackData {
	unsigned int bufferFrames;
	unsigned int pos;
	/*A buffer to store old samples to use in posterior calculations.
	To store 1 second of samples size should be: n = sampleRate, to store 2 seconds n = sampleRate x 2*/
	unsigned int samplesRecordBufferSize;
	CircularBuffer<SAMPLE_TYPE> * samplesRecordBuffer;
	audioPaths * paths;
	/*Rs's type needs to be compatible with SAMPLE_TYPE.
	If Rs's values go from 0 to 1, then the SAMPLE_TYPE should allow decimal values*/
	std::vector<float> * Rs;
	//thread_pool * pool;
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
	audioCallbackData * audioData;
public:
	AudioRenderer();
	void render(Scene * scene, Camera * camera, Source * source);
	~AudioRenderer();
};