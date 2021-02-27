// Copyright 2009-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "pch.h"
#include <iostream>

#include <ctime>
#include <windows.h>
#include "Utils.h"
#include "Camera.h"
#include "Mesh.h"
#include "ShaderProgram.h"

#include <embree3/rtcore.h>
#include <embree3/rtcore_common.h>
#include <stdio.h>
#include <math.h>
#include <limits>
#include <stdio.h>

#include "Scene.h"

#if defined(_WIN32)
#  include <conio.h>
#  include <windows.h>
#endif

using namespace std;

void init();
void initGL();
void draw();
void close();

SDL_Window* window = NULL;
SDL_GLContext context;

int WIDTH = 800;
int HEIGHT = 600;

/*
 * A minimal tutorial.
 *
 * It demonstrates how to intersect a ray with a single triangle. It is
 * meant to get you started as quickly as possible, and does not output
 * an image.
 *
 * For more complex examples, see the other tutorials.
 *
 * Compile this file using
 *
 *   gcc -std=c99 \
 *       -I<PATH>/<TO>/<EMBREE>/include \
 *       -o minimal \
 *       minimal.c \
 *       -L<PATH>/<TO>/<EMBREE>/lib \
 *       -lembree3
 *
 * You should be able to compile this using a C or C++ compiler.
 */

 /*
  * This is only required to make the tutorial compile even when
  * a custom namespace is set.
  */
#if defined(RTC_NAMESPACE_USE)
RTC_NAMESPACE_USE
#endif

/*
 * We will register this error handler with the device in initializeDevice(),
 * so that we are automatically informed on errors.
 * This is extremely helpful for finding bugs in your code, prevents you
 * from having to add explicit error checking to each Embree API call.
 */
	void errorFunction(void* userPtr, enum RTCError error, const char* str)
{
	printf("error %d: %s\n", error, str);
}

/*
 * Embree has a notion of devices, which are entities that can run
 * raytracing kernels.
 * We initialize our device here, and then register the error handler so that
 * we don't miss any errors.
 *
 * rtcNewDevice() takes a configuration string as an argument. See the API docs
 * for more information.
 *
 * Note that RTCDevice is reference-counted.
 */
RTCDevice initializeDevice()
{
	RTCDevice device = rtcNewDevice(NULL);

	if (!device)
		printf("error %d: cannot create device\n", rtcGetDeviceError(NULL));

	rtcSetDeviceErrorFunction(device, errorFunction, NULL);
	return device;
}

/*
 * Create a scene, which is a collection of geometry objects. Scenes are
 * what the intersect / occluded functions work on. You can think of a
 * scene as an acceleration structure, e.g. a bounding-volume hierarchy.
 *
 * Scenes, like devices, are reference-counted.
 */
Scene * initializeScene(RTCDevice device)
{
	Scene * new_scene = new Scene(device);
	new_scene->addMeshFromObj("models/cornell_box.obj", device);
	new_scene->commitScene();
	return new_scene;
}

/*
 * Cast a single ray with origin (ox, oy, oz) and direction
 * (dx, dy, dz).
 */
void castRay(RTCScene scene,
	float ox, float oy, float oz,
	float dx, float dy, float dz)
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
	rayhit.ray.org_x = ox;
	rayhit.ray.org_y = oy;
	rayhit.ray.org_z = oz;
	rayhit.ray.dir_x = dx;
	rayhit.ray.dir_y = dy;
	rayhit.ray.dir_z = dz;
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

	printf("%f, %f, %f: ", ox, oy, oz);
	if (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID)
	{
		/* Note how geomID and primID identify the geometry we just hit.
		 * We could use them here to interpolate geometry information,
		 * compute shading, etc.
		 * Since there is only a single triangle in this scene, we will
		 * get geomID=0 / primID=0 for all hits.
		 * There is also instID, used for instancing. See
		 * the instancing tutorials for more information */
		printf("Found intersection on geometry %d, primitive %d at tfar=%f\n",
			rayhit.hit.geomID,
			rayhit.hit.primID,
			rayhit.ray.tfar);
	}
	else
		printf("Did not find any intersection.\n");
}

void waitForKeyPressedUnderWindows()
{
#if defined(_WIN32)
	HANDLE hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(hStdOutput, &csbi)) {
		printf("GetConsoleScreenBufferInfo failed: %d\n", GetLastError());
		return;
	}

	/* do not pause when running on a shell */
	if (csbi.dwCursorPosition.X != 0 || csbi.dwCursorPosition.Y != 0)
		return;

	/* only pause if running in separate console window. */
	printf("\n\tPress any key to exit...\n");
	int ch = _getch();
#endif
}


/* -------------------------------------------------------------------------- */

void init() {
	SDL_Init(SDL_INIT_VIDEO);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	window = SDL_CreateWindow("OpenGL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT,
		SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	context = SDL_GL_CreateContext(window);
	glewExperimental = GL_TRUE;
	glewInit();
	initGL();
}

void initGL() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	//OpenGL attribs
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glDisable(GL_CULL_FACE);
}

void draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void close() {
	SDL_DestroyWindow(window);
	window = NULL;
	SDL_Quit();
}

int main(int argc, char* argv[]) {
	init();
	Camera cam = Camera(WIDTH, HEIGHT, 45, window);
	ShaderProgram* pass = new ShaderProgram("assets/shaders/pass.vert", "assets/shaders/pass.frag");
	bool exit = false;

	bool wireframe = false;

	SDL_Event event;

	double frameTime = 1000.0f / 65.0f;

	RTCDevice device = initializeDevice();
	Scene * scene = initializeScene(device);

	std::clock_t start;
	while (!exit) {
		start = clock();
		while (SDL_PollEvent(&event) != 0) {
			switch (event.type) {
			case SDL_QUIT:
				exit = true;
				break;
			case SDL_KEYDOWN: {
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					exit = true;
					break;
				}
				else if (event.key.keysym.sym == SDLK_m) {
					cam.moveCam();
					break;
				}
			}
							  break;
			case SDL_MOUSEBUTTONDOWN: {
			}
									  break;
			}
		}
		cam.update();
		draw();
		pass->bind();
		GLuint worldTransformID = glGetUniformLocation(pass->getId(), "worldTransform");
		glUniformMatrix4fv(worldTransformID, 1, GL_FALSE, &cam.modelViewProjectionMatrix[0][0]);
		GLuint vistaID = glGetUniformLocation(pass->getId(), "vista");
		glUniform3fv(vistaID, 1, &((cam.ref - cam.pos)[0]));
		scene->draw();
		pass->unbind();

		/* Cast rays */
		castRay(scene->getRTCScene(), 0, 0, -1, 0, 0, 1);
		castRay(scene->getRTCScene(), 1, 1, -1, 0, 0, 1);

		double dif = frameTime - ((clock() - start) * (1000.0 / double(CLOCKS_PER_SEC)));
		if (dif > 0) {
			Sleep(int(dif));
		}
		SDL_GL_SwapWindow(window);
	}

	delete(scene);
	/* Though not strictly necessary in this example, you should
	/* always make sure to release resources allocated through Embree. */
	rtcReleaseDevice(device);
	/* wait for user input under Windows when opened in separate window */
	waitForKeyPressedUnderWindows();

	close();
	return 0;
}

