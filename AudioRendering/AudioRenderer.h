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

//Returns the distance to the intersection if there is one, -1 if not.
float raySphereIntersection(glm::vec3 origin, glm::vec3 dir, glm::vec3 center, float radius) {
	//The following is obtained from solving the ecuation system given by the ray and sphere
	//The result is a second degree ecuation: at^2 + bt + c = 0
	float a = glm::dot(dir, dir);
	float b = 2 * glm::dot(dir, origin - center);
	float c = glm::dot(origin - center, origin - center) - pow(radius, 2);
	float discriminant = (pow(b, 2) - 4 * a*c);
	if (discriminant < 0) {
		return -1;
	}
	else {
		float t1 = (-b - sqrt(discriminant)) / (2.0*a);
		float t2 = (-b + sqrt(discriminant)) / (2.0*a);
		if (t1 > 0 && t2 > 0) {
			return std::min(t1, t2);
		}
		return -1;
	}
}

/*
 * Cast a single ray with origin (ox, oy, oz) and direction
 * (dx, dy, dz).
 */
//This function needs to do the intersection with the sound source and the reflection of the ray if it collides with geometry
void castRay(RTCScene scene, 
	glm::vec3 origin, 
	glm::vec3 dir,
	Source * source)
{
	/*
	 * The intersect context can be used to set intersection
	 * filters or flags, and it also contains the instance ID stack
	 * used in multi-level instancing.
	 */
	struct RTCIntersectContext context;
	rtcInitIntersectContext(&context);

	/*
	 * The ray hit structure holds both the ray and the hit.
	 * The user must initialize it properly -- see API documentation
	 * for rtcIntersect1() for details.
	 */
	struct RTCRayHit rayhit;
	rayhit.ray.org_x = origin.x;
	rayhit.ray.org_y = origin.y;
	rayhit.ray.org_z = origin.z;
	rayhit.ray.dir_x = dir.x;
	rayhit.ray.dir_y = dir.y;
	rayhit.ray.dir_z = dir.z;
	rayhit.ray.tnear = 0;
	rayhit.ray.tfar = std::numeric_limits<float>::infinity();
	rayhit.ray.mask = -1;
	rayhit.ray.flags = 0;
	rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
	rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

	/*
	 * There are multiple variants of rtcIntersect. This one
	 * intersects a single ray with the scene.
	 */
	rtcIntersect1(scene, &context, &rayhit);

	float distance_to_source = raySphereIntersection(origin, dir, source->pos, source->sphere_radius);
	if (distance_to_source >= 0) {
		printf("intersection with source was found\n");
	}

	//printf("%f, %f, %f: ", ox, oy, oz);
	if (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID)
	{
		//Ray intersects source
		if (distance_to_source >= 0 && distance_to_source > rayhit.ray.tfar) {
			printf("intersection blocked by object\n");
		}

		//Reflect ray and continue
		/* Note how geomID and primID identify the geometry we just hit.
		 * We could use them here to interpolate geometry information,
		 * compute shading, etc. */
		/*printf("Found intersection on geometry %d, primitive %d at tfar=%f\n",
			rayhit.hit.geomID,
			rayhit.hit.primID,
			rayhit.ray.tfar);*/
	}
	//else
	//	printf("Did not find any intersection.\n");
}

void OmnidirectionalUniformSphereRayCast(Scene * scene, Camera * camera, Source * source) {
	srand(time(NULL));
	float rnd1 = (float)rand() / RAND_MAX;
	float rnd2 = (float)rand() / RAND_MAX;

	int n = 20;		//The number of subdivisions in the sphere is n^2
	
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j) {
			float dx = 2 * sqrtf((i + rnd1) / n - pow((i + rnd1) / n, 2)) * cos(2 * M_PI*(j + rnd2) / n);
			float dy = 2 * sqrtf((i + rnd1) / n - pow((i + rnd1) / n, 2)) * sin(2 * M_PI*(j + rnd2) / n);
			float dz = 1 - 2 * (i + rnd1) / n;
			glm::vec3 dir = glm::normalize(glm::vec3(dx, dy, dz));
			castRay(scene->getRTCScene(), camera->pos, dir, source);
		}
	}
}