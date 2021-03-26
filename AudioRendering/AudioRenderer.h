#pragma once
/*Audio samples contain values of amplitude (loudness). If we have a sample rate of 44100Hz, 
frequency is derived from the variations in these amplitudes considering that two 
contiguous samples are 1/44100 seconds apart.*/

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

#define LISTENER_SPHERE_RADIUS 2.0f
#define NUMBER_OF_RAYS 10000
//Speed of sound in the air at 20 °C in m/s
#define SPEED_OF_SOUND 343

typedef struct rayHistory{
	float travelled_distance;
	float remaining_energy_factor;
	int reflection_num;
} rayHistory;

typedef struct path {
	float travelled_distance;
	float remaining_energy_factor;
	int reflection_num;
} path;

class AudioRenderer {
public:
	//buffer to store all frames up to n seconds
	std::vector<path> * paths; //paths result of the audio render
public:
	AudioRenderer();
	void render(Scene * scene, Camera * camera, Source * source);
	~AudioRenderer();
};