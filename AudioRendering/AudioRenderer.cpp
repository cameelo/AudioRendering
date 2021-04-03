#include "AudioRenderer.h"
#include <mutex>

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
	glm::vec3 listener_pos,
	rayHistory history,
	audioPaths * paths)
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

	rtcIntersect1(scene, &context, &rayhit);

	//Check if ray interescts listener
	float distance_to_source = raySphereIntersection(origin, dir, listener_pos, LISTENER_SPHERE_RADIUS);
	//Check if ray intersects room
	if (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID)
	{
		//If listener intersection found
		if (distance_to_source >= 0) {
			//If listener intersection is before room interection
			if (distance_to_source < rayhit.ray.tfar) {
				//Add distance_to_source to overall distance
				//Calculate parameters for transfer function (e.g. absorption from specular reflections)
				//Return parameters and traveled distance to add to the histogram
				//printf("Found intersection with listener. %i\n", history.reflection_num);
				//Add path to paths
				audioPath newAudioPath = { history.travelled_distance + distance_to_source, history.remaining_energy_factor };
				paths->mutex->lock();
				paths->size++;
				paths->ptr = (audioPath*)realloc(paths->ptr, paths->size * sizeof(audioPath));
				paths->ptr[paths->size - 1] = newAudioPath;
				paths->mutex->unlock();
				return;
			}
			else {
				//printf("Intersection blocked by geometry. %i\n", history.reflection_num);
			}
		}

		//Calculate remaining energy if less than something also return
		if (history.reflection_num > 10) {
			//printf("Ray exahusted.\n");
			return;
		}
		//Reflect ray with geometry normal
		glm::vec3 new_dir = glm::reflect(dir, glm::vec3(rayhit.hit.Ng_x, rayhit.hit.Ng_y, rayhit.hit.Ng_z));
		//New origin is obtained by moving tfar in the ray direction from the current origin
		glm::vec3 new_origin = origin + dir * rayhit.ray.tfar;
		//When casting new ray new origin must me moved delta in the new direction to avoid numeric errors. (Ray begining inside the geometry)
		history.reflection_num++;
		history.remaining_energy_factor*0.5;
		history.travelled_distance += rayhit.ray.tfar;
		castRay(scene, new_origin + new_dir * 0.01f, new_dir, listener_pos, history, paths);

		/* Note how geomID and primID identify the geometry we just hit.
		 * We could use them here to interpolate geometry information,
		 * compute shading, etc. */
	}
	else {
		if (distance_to_source >= 0) {
			//Add distance_to_source to overall distance
			//Calculate parameters for transfer function (e.g. absorption from specular reflections)
			//Return parameters and traveled distance to add to the histogram
			//printf("Found intersection with listener. %i\n", history.reflection_num);
			audioPath newAudioPath = { history.travelled_distance + distance_to_source, history.remaining_energy_factor };
			paths->mutex->lock();
			paths->size++;
			paths->ptr = (audioPath*)realloc(paths->ptr, paths->size * sizeof(audioPath));
			paths->ptr[paths->size - 1] = newAudioPath;
			paths->mutex->unlock();
			return;
		}
	}
	//printf("No intersection with listener found.\n");
	return;
}

void OmnidirectionalUniformSphereRayCast(Scene * scene, 
	Camera * camera, 
	Source * source, 
	audioPaths * paths)
{
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::mt19937 generator(seed);
	std::uniform_real_distribution<double> uniform01(0.0, 1.0);

	for (int i = 0; i < NUMBER_OF_RAYS; ++i) {
		double theta = 2 * M_PI * uniform01(generator);
		double phi = acos(1 - 2 * uniform01(generator));
		double dx = sin(phi) * cos(theta);
		double dy = sin(phi) * sin(theta);
		double dz = cos(phi);
		glm::vec3 dir = glm::normalize(glm::vec3(dx, dy, dz));
		rayHistory new_ray_history = { 0.0f, 1.0f, 0 };
		castRay(scene->getRTCScene(), source->pos, dir, camera->pos, new_ray_history, paths);
	}

	////srand(time(NULL));
	//float rnd1 = uniform01(generator);
	//float rnd2 = uniform01(generator);

	//int n = 20;		//The number of subdivisions in the sphere is n^2
	//for (int i = 0; i < n; ++i) {
	//	for (int j = 0; j < n; ++j) {
	//		float dx = 2 * sqrtf((i + rnd1) / n - pow((i + rnd1) / n, 2)) * cos(2 * M_PI*(j + rnd2) / n);
	//		float dy = 2 * sqrtf((i + rnd1) / n - pow((i + rnd1) / n, 2)) * sin(2 * M_PI*(j + rnd2) / n);
	//		float dz = 1 - 2 * (i + rnd1) / n;
	//		glm::vec3 dir = glm::normalize(glm::vec3(dx, dy, dz));
	//		castRay(scene->getRTCScene(), source->pos, dir, camera->pos);
	//	}
	//}
}

void viewDirRayCast(Scene * scene, Camera * camera, Source * source) {
	rayHistory new_ray_history = { 0.0f, 1.0f, 0 };
	castRay(scene->getRTCScene(), camera->pos, camera->ref - camera->pos, source->pos, new_ray_history, NULL);
}

//Callback that processes audio frames
int processAudio(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
	double streamTime, RtAudioStreamStatus status, void *data)
{
	// Since the number of input and output channels is equal, we can do
	// a simple buffer copy operation here.
	if (status) std::cout << "Stream over/underflow detected." << std::endl;

	unsigned long *bytes = (unsigned long *)data;
	memcpy(outputBuffer, inputBuffer, *bytes);
	return 0;
}

int inout(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
	double streamTime, RtAudioStreamStatus status, void *data)
{
	// Since the number of input and output channels is equal, we can do
	// a simple buffer copy operation here.
	if (status) std::cout << "Stream over/underflow detected." << std::endl;

	unsigned long *bytes = (unsigned long *)data;
	memcpy(outputBuffer, inputBuffer, *bytes);
	return 0;
}

AudioRenderer::AudioRenderer() {
	//Init audio stream
	this->audioApi = new RtAudio();

	if (this->audioApi->getDeviceCount() < 1) {
		std::cout << "\nNo audio devices found!\n";
		exit(0);
	}

	unsigned int bufferBytes, bufferFrames = 512, sampleRate = 44100, num_channles = 2;

	//Set up stream parameters they need to be in heap since audio api will use them in separate thread
	this->streamParams = new streamParameters();
	this->streamParams->iParams = new RtAudio::StreamParameters();
	this->streamParams->iParams->deviceId = this->audioApi->getDefaultInputDevice();
	this->streamParams->iParams->nChannels = num_channles;
	this->streamParams->iParams->firstChannel = 0;
	this->streamParams->oParams = new RtAudio::StreamParameters();
	this->streamParams->oParams->deviceId = this->audioApi->getDefaultOutputDevice();
	this->streamParams->oParams->nChannels = num_channles;
	this->streamParams->oParams->firstChannel = 0;
	this->streamParams->bufferFrames = new unsigned int(bufferFrames);
	this->streamParams->options = new RtAudio::StreamOptions();

	this->bufferBytes = new unsigned int(bufferFrames * num_channles * sizeof(SAMPLE_TYPE));

	//This whole struct will be accessed from the audio api thread, so it needs to be in heap.
	this->audioData = new audioCallbackData();
	this->audioData->pos = 0;
	this->audioData->samplesRecordBufferSize = sampleRate * num_channles * sizeof(SAMPLE_TYPE);
	this->audioData->samplesRecordBuffer = new SAMPLE_TYPE[this->audioData->samplesRecordBufferSize];
	this->audioData->paths = new audioPaths();
	this->audioData->paths->ptr = NULL;
	this->audioData->paths->size = 0;
	this->audioData->paths->mutex = new std::mutex();

	try {
		this->audioApi->openStream(this->streamParams->oParams, this->streamParams->iParams, SAMPLE_FORMAT, sampleRate,
			this->streamParams->bufferFrames, &inout, (void *)this->bufferBytes, this->streamParams->options);
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
}

void AudioRenderer::render(Scene * scene, Camera * camera, Source * source) {
	OmnidirectionalUniformSphereRayCast(scene, camera, source, this->currentPaths);

	////Update audio paths with paths found in this frame
	this->audioData->paths->mutex->lock();
	if (this->currentPaths->size > 0) {
 		this->audioData->paths->ptr = (audioPath*)realloc(this->audioData->paths->ptr, this->currentPaths->size * sizeof(audioPath));
		this->audioData->paths->size = this->currentPaths->size;
		memcpy(this->audioData->paths->ptr, this->currentPaths->ptr, this->currentPaths->size * sizeof(audioPath));
		free(this->currentPaths->ptr);
		this->currentPaths->ptr = NULL;
		this->currentPaths->size = 0;
	}
	else {
		free(this->audioData->paths->ptr);
		this->audioData->paths->ptr = NULL;
		this->audioData->paths->size = 0;
	}
	this->audioData->paths->mutex->unlock();
}

AudioRenderer::~AudioRenderer() {

}