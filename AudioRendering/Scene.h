#pragma once

#include <embree3/rtcore.h>
#include <embree3/rtcore_common.h>
#include "Mesh.h"
#include "SceneObject.h"

class Scene {
public:
	RTCScene rtc_scene;
	//std::vector<Mesh*> meshes;
	std::vector<SceneObject*> objects;

public:
	Scene() {};
	Scene(RTCDevice device);
	//Scene(std::string file_name, RTCDevice device);
	void addObjectFromOBJ(std::string file_name, glm::vec3 pos, float size, RTCDevice * device);
	void commitScene();
	//void draw();
	RTCScene getRTCScene();
	~Scene();
};