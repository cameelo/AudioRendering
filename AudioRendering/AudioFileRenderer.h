#pragma once

#include <vector>
#include <fstream>
#include <iomanip>

#include "AudioRenderingUtils.h"
#include "Camera.h"
#include "Scene.h"
#include "Source.h"

#include "wavParser.h"

void renderAudioFile(Scene * scene, glm::vec3 camera_pos, glm::vec3 source_pos , const char * measurement_file_path) {

	audioPaths * paths = new audioPaths();
	paths->ptr = NULL;
	paths->size = 0;
	paths->mutex = new std::mutex;

	OmnidirectionalUniformSphereRayCast(scene, camera_pos, source_pos, paths);

	//The size of Rs will depend on the lenght of the IR I want to mesure and the subdivision of that time length.
	//This means that if I want an IR to match the Rs used for auralization then I will have to simulate a 1 second IR
	//and multiply it by SAMPLE_RATE. 
	//However any other combination can be used depending on the desired outcome. If I just want to compare the result
	//of the simulation with the measurement, then the size will depend on the size and sample rate of the measurement file.
	
	WavData * wav_data = new WavData();
	loadWavFile(measurement_file_path, wav_data);
	
	size_t size = 1000;
	std::vector<float> * rs = new std::vector<float>(size);

	//Initialize Rs
	std::fill(rs->begin(), rs->end(), 0.0);

	//Paths store the distance, to get the corresponding cell in vector Rs we need to find the elapsed time
	for (int i = 0; i < paths->size; i++) {
		float distance = paths->ptr[i].travelled_distance;
		float remaining_factor = paths->ptr[i].remaining_energy_factor;
		float elapsed_time = distance / SPEED_OF_SOUND;
		//The elapsed time is then converted to a position in the array by multiplying the time by the samples per second
		//This way a path that takes 1s to reach the listener will ocuppy the last position in the array.
		unsigned int array_pos = round(elapsed_time * SAMPLE_RATE);
		if (array_pos < size && array_pos >= 0) {
			(*rs)[array_pos] += remaining_factor;
		}
	}
	std::ofstream rs_file("rs.txt");
	rs_file << std::setprecision(7);
	float received_energy = 0;
	for (int i = 0; i < size; i++) {
		rs_file << (*rs)[i] << ",";
		received_energy += (*rs)[i];
	}
	rs_file << std::endl << received_energy;
	rs_file.close();
}