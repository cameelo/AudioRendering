#pragma once

#include <embree3/rtcore.h>
#include <embree3/rtcore_common.h>
#include "tiny_obj_loader.h"

class Scene {
private:
	RTCScene rtc_scene;

public:
	Scene() {};
	Scene(std::string file_name, RTCDevice device);
	RTCScene getRTCScene();
	~Scene();
};