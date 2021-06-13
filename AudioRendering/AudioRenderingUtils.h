#pragma once

#include <mutex>
#include <glm/glm.hpp>

#include "Scene.h"
#include "Camera.h"
#include "Source.h"

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

typedef struct rayHistory {
	float travelled_distance;
	float remaining_energy_factor;
	int reflection_num;
} rayHistory;

typedef struct audioPath {
	float travelled_distance;
	float remaining_energy_factor;
} audioPath;

typedef struct audioPaths {
	audioPath * ptr;
	size_t size;
	//paths are used by audio library thread and main thread. (And possibly multiple rayCasting threads)
	std::mutex * mutex;
} audioPaths;


//Returns the distance to the intersection if there is one, -1 if not.
float raySphereIntersection(glm::vec3 origin, glm::vec3 dir, glm::vec3 center, float radius);

/*
 * Cast a single ray with origin (ox, oy, oz) and direction
 * (dx, dy, dz).
 */
 //This function needs to do the intersection with the sound source and the reflection of the ray if it collides with geometry
void castRay(RTCScene scene,
	glm::vec3 origin,
	glm::vec3 dir,
	glm::vec3 listener_pos,
	rayHistory history,
	audioPaths * paths);

void OmnidirectionalUniformSphereRayCast(Scene * scene,
	glm::vec3 listener_pos,
	glm::vec3 source_pos,
	audioPaths * paths);

void viewDirRayCast(Scene * scene, Camera * camera, Source * source);