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

#include "Scene.h"
#include "AudioRenderer.h"

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
	new_scene->addObjectFromOBJ("models/suzanne.obj", glm::vec3(0.0f,0.0f,0.0f), 5.0f, &device);
	//new_scene->addMeshFromObj("models/teapot.obj", device);
	new_scene->commitScene();
	return new_scene;
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
	Source * source = new Source(glm::vec3(0, 0, -10), 2, "models/sphere.obj");
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
		//ViewProjectionMatrix
		GLuint worldTransformID = glGetUniformLocation(pass->getId(), "worldTransform");
		glUniformMatrix4fv(worldTransformID, 1, GL_FALSE, &cam.modelViewProjectionMatrix[0][0]);
		GLuint modelMatrixID = glGetUniformLocation(pass->getId(), "modelMatrix");
		GLuint vistaID = glGetUniformLocation(pass->getId(), "vista");
		glUniform3fv(vistaID, 1, &((cam.ref - cam.pos)[0]));
		//Directional light. To do a point light more shader code is needed.
		GLuint lightDirID = glGetUniformLocation(pass->getId(), "lightDir");
		glUniform3fv(lightDirID, 1, &(glm::vec3(1, 0, 0)[0]));
		for (int i = 0; i < scene->objects.size(); ++i) {
			glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &scene->objects[i]->getModelMatrix()[0][0]);
			scene->objects[i]->draw();
		}
		glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &source->getModelMatrix()[0][0]);
		source->draw();
		pass->unbind();

		//OmnidirectionalUniformSphereRayCast(scene, &cam, source);
		viewDirRayCast(scene, &cam, source);

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

