#pragma once

#include <embree3/rtcore.h>
#include <embree3/rtcore_common.h>
#include "tiny_obj_loader.h"
#include "Mesh.h"

class Scene {
private:
	RTCScene rtc_scene;
	std::vector<Mesh*> meshes;

public:
	Scene() {};
	Scene(RTCDevice device);
	//Scene(std::string file_name, RTCDevice device);
	void addMeshFromObj(std::string file_name, RTCDevice device);
	void commitScene();
	void draw();
	RTCScene getRTCScene();
	~Scene();
};