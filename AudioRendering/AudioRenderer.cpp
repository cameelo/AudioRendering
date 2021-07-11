#include "AudioRenderer.h"
#include <mutex>
#include <thread>
#include <fstream>
#include <iomanip>

std::mutex outputBufferMutex;

//void rowColumnProduct(int begin, int end, void * outputBuffer, audioCallbackData * renderData) {
//	for (int i = begin; i <= end; i++) {
//		SAMPLE_TYPE output_value = 0;
//		for (int j = 0; j < renderData->samplesRecordBufferSize - i; j++) {
//			output_value += (*renderData->Rs)[j] * renderData->samplesRecordBuffer->getElement(renderData->samplesRecordBufferSize - 1 - i - j);
//		}
//		outputBufferMutex.lock();
//		((SAMPLE_TYPE*)outputBuffer)[renderData->bufferFrames - 1 - i] = output_value;
//		outputBufferMutex.unlock();
//	}
//}

//Callback that processes audio frames
int processAudio(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
	double streamTime, RtAudioStreamStatus status, void *data)
{
	if (status) std::cout << "Stream over/underflow detected." << std::endl;

	audioCallbackData *renderData = (audioCallbackData *)data;
	memset(outputBuffer, 0, renderData->bufferFrames * sizeof(SAMPLE_TYPE));
	//First we copy the new samples into our buffer
	renderData->samplesRecordBuffer->insert((SAMPLE_TYPE*)inputBuffer, renderData->bufferFrames);
	if (renderData->samplesRecordBuffer->full) {
		//Then we process the frames
		/*int frames_per_thread = round(renderData->bufferFrames / 4);
		renderData->pool->push_task(rowColumnProduct, 0, frames_per_thread - 1, outputBuffer, renderData);
		renderData->pool->push_task(rowColumnProduct, frames_per_thread, frames_per_thread * 2 - 1, outputBuffer, renderData);
		renderData->pool->push_task(rowColumnProduct, frames_per_thread * 2, frames_per_thread * 3 - 1, outputBuffer, renderData);
		renderData->pool->push_task(rowColumnProduct, frames_per_thread * 3, renderData->bufferFrames - 1, outputBuffer, renderData);
		
		renderData->pool->wait_for_tasks();*/

		unsigned int RvIndex;

		for (int i = 0; i < renderData->bufferFrames; i++) {
			SAMPLE_TYPE output_value = 0;
			for (int j = 0; j < renderData->samplesRecordBufferSize - i; j++) {
				output_value += (*renderData->Rs)[j] * renderData->samplesRecordBuffer->getElement(renderData->samplesRecordBufferSize - 1 - i - j);
			}
			outputBufferMutex.lock();
			//Output has 2 channels
			RvIndex = (renderData->bufferFrames * 2) - 1 - (i * 2);
			((SAMPLE_TYPE*)outputBuffer)[RvIndex] = output_value;
			((SAMPLE_TYPE*)outputBuffer)[RvIndex - 1] = output_value;
			outputBufferMutex.unlock();
		}

		//For every element in Rs
		//for (int i = 0; i < renderData->samplesRecordBufferSize; i++) {
		//	//for each element in rho's column
		//	for (int j = 0; j < renderData->bufferFrames && j < renderData->samplesRecordBufferSize - i; j++) {
		//		float value = (*renderData->Rs)[i] * renderData->samplesRecordBuffer->getElement(renderData->samplesRecordBufferSize - 1 - i - j);
		//		RvIndex = (renderData->bufferFrames * 2) - 1 - (j * 2);
		//		((SAMPLE_TYPE*)outputBuffer)[RvIndex] += value;
		//		((SAMPLE_TYPE*)outputBuffer)[RvIndex - 1] += value;
		//	}
		//	
		//}
	}
	return 0;
}

int inout(void *outputBuffer, void *inputBuffer, unsigned int /*nBufferFrames*/,
	double /*streamTime*/, RtAudioStreamStatus status, void *data)
{
	// Since the number of input and output channels is equal, we can do
	// a simple buffer copy operation here.
	if (status) std::cout << "Stream over/underflow detected." << std::endl;

	memcpy(outputBuffer, inputBuffer, 512);
	return 0;
}

AudioRenderer::AudioRenderer() {
	//Init audio stream
	this->audioApi = new RtAudio();

	if (this->audioApi->getDeviceCount() < 1) {
		std::cout << "\nNo audio devices found!\n";
		exit(0);
	}

	unsigned int bufferBytes, bufferFrames = 512, sampleRate = SAMPLE_RATE, input_channles = 1, output_channels = 2;

	//Set up stream parameters they need to be in heap since audio api will use them in separate thread
	this->streamParams = new streamParameters();
	this->streamParams->iParams = new RtAudio::StreamParameters();
	this->streamParams->iParams->deviceId = this->audioApi->getDefaultInputDevice();
	this->streamParams->iParams->nChannels = input_channles;
	this->streamParams->iParams->firstChannel = 0;
	this->streamParams->oParams = new RtAudio::StreamParameters();
	this->streamParams->oParams->deviceId = this->audioApi->getDefaultOutputDevice();
	this->streamParams->oParams->nChannels = output_channels;
	this->streamParams->oParams->firstChannel = 0;
	this->streamParams->bufferFrames = new unsigned int(bufferFrames);
	this->streamParams->options = new RtAudio::StreamOptions();

	this->bufferBytes = new unsigned int(bufferFrames * input_channles * sizeof(SAMPLE_TYPE));

	//This whole struct will be accessed from the audio api thread, so it needs to be in heap.
	this->audioData = new audioCallbackData();
	//Total frames in a buffer for all channels
	this->audioData->bufferFrames = bufferFrames * input_channles;
	this->audioData->pos = 0;
	//1 second's worth of samples.
	this->audioData->samplesRecordBufferSize = sampleRate * input_channles;
	this->audioData->samplesRecordBuffer = new CircularBuffer<SAMPLE_TYPE>(this->audioData->samplesRecordBufferSize);
	this->audioData->paths = new audioPaths();
	this->audioData->paths->ptr = NULL;
	this->audioData->paths->size = 0;
	this->audioData->paths->mutex = new std::mutex();
	//this->audioData->pool = new thread_pool(4);

	try {
		this->audioApi->openStream(this->streamParams->oParams, this->streamParams->iParams, SAMPLE_FORMAT, sampleRate,
			this->streamParams->bufferFrames, &processAudio, (void *)this->audioData, this->streamParams->options);
	}
	catch (RtAudioError& e) {
		e.printMessage();
		exit(0);
	}

	try {
		this->audioApi->startStream();
	}
	catch (RtAudioError& e) {
		e.printMessage();
		exit(0);
	}

	this->currentPaths = new audioPaths();
	this->currentPaths->ptr = NULL;
	this->currentPaths->size = 0;
	this->currentPaths->mutex = new std::mutex;

	//Create Rs vector
	this->audioData->Rs = new std::vector<float>(this->audioData->samplesRecordBufferSize);
}

void AudioRenderer::render(Scene * scene, Camera * camera, Source * source) {
	RayTracer rt = RayTracer(scene, camera->pos, LISTENER_SPHERE_RADIUS, source->pos, SOURCE_POWER, this->currentPaths);
	rt.OmnidirectionalUniformSphereRayCast();
	
	//Initialize Rs
	std::fill(this->audioData->Rs->begin(), this->audioData->Rs->end(), 0.0);

	//Paths store the distance, to get the corresponding cell in vector Rs we need to find the elapsed time
	for (int i = 0; i < this->currentPaths->size; i++) {
		float distance = this->currentPaths->ptr[i].travelled_distance;
		float remaining_factor = this->currentPaths->ptr[i].remaining_energy_factor;
		float elapsed_time = distance / SPEED_OF_SOUND;
		//The elapsed time is then converted to a position in the array by multiplying the time by the samples per second
		//This way a path that takes 1s to reach the listener will ocuppy the last position in the array.
		unsigned int array_pos = round(elapsed_time * SAMPLE_RATE);
		if (array_pos < this->audioData->samplesRecordBufferSize && array_pos >= 0) {
			(*this->audioData->Rs)[array_pos] += remaining_factor;
		}
	}
	std::ofstream rs_file("rs.txt");
	rs_file << std::setprecision(7);
	float received_energy = 0;
	for (int i = 0; i < this->audioData->samplesRecordBufferSize; i++) {
		rs_file << (*this->audioData->Rs)[i] << ",";
		received_energy += (*this->audioData->Rs)[i];
	}
	rs_file << std::endl << received_energy;
	rs_file.close();
}

AudioRenderer::~AudioRenderer() {

}