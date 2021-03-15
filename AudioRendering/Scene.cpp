#include "Scene.h"
#include "OBJLoader.h"

Scene::Scene(RTCDevice device) {
	this->rtc_scene = rtcNewScene(device);
}

void Scene::addObjectFromOBJ(std::string file_name, glm::vec3 pos, float size, RTCDevice * device) {
	//Create mesh in scene
	OBJProperites props = loadOBJ(file_name);

	SceneObject * object = new SceneObject(pos,size, props);
	this->objects.push_back(object);
	/*Mesh * mesh = new Mesh(props.vertices, props.indices, props.normals);
	this->meshes.push_back(mesh);*/

	if (device) {
		RTCGeometry geom = rtcNewGeometry(*device, RTC_GEOMETRY_TYPE_TRIANGLE);

		float* vertices = (float*)rtcSetNewGeometryBuffer(geom,
			RTC_BUFFER_TYPE_VERTEX,
			0,
			RTC_FORMAT_FLOAT3,
			3 * sizeof(float),
			props.vertices.size() / 3); //VERTEX COUNT (3 floats represent 1 vertex)

		std::copy(props.vertices.begin(), props.vertices.end(), vertices);

		unsigned* indices = (unsigned*)rtcSetNewGeometryBuffer(geom,
			RTC_BUFFER_TYPE_INDEX,
			0,
			RTC_FORMAT_UINT3,
			3 * sizeof(unsigned),
			props.indices.size() / 3); //FACE COUNT (3 indices are counted as 1 item since they represent a single triangle)

		std::copy(props.indices.begin(), props.indices.end(), indices);


		rtcCommitGeometry(geom);

		rtcAttachGeometry(rtc_scene, geom);
		rtcReleaseGeometry(geom);
	}
}

void Scene::commitScene() {
	rtcCommitScene(this->rtc_scene);
}

//void Scene::draw() {
//	for (int i = 0; i < this->meshes.size(); ++i) {
//		this->meshes[i]->draw();
//	}
//}

RTCScene Scene::getRTCScene() {
	return this->rtc_scene;
}

Scene::~Scene() {
	for (int i = 0; i < this->objects.size(); ++i) {
		delete(this->objects[i]);
	}
	rtcReleaseScene(this->rtc_scene);
}