#include "Scene.h"

Scene::Scene(RTCDevice device) {
	this->rtc_scene = rtcNewScene(device);
}

void Scene::addMeshFromObj(std::string file_name, RTCDevice device) {
	RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

	tinyobj::ObjReader reader = tinyobj::ObjReader();
	bool res = reader.ParseFromFile(file_name);

	//VERTICES -----------------------------------------------------------------------

	auto obj_vertices = reader.GetAttrib().GetVertices();

	float* vertices = (float*)rtcSetNewGeometryBuffer(geom,
		RTC_BUFFER_TYPE_VERTEX,
		0,
		RTC_FORMAT_FLOAT3,
		3 * sizeof(float),
		obj_vertices.size() / 3); //VERTEX COUNT (3 floats represent 1 vertex)

	std::copy(obj_vertices.begin(), obj_vertices.end(), vertices);

	//INDICES ------------------------------------------------------------------------

	auto shapes = reader.GetShapes();

	size_t shape_offset = 0;
	std::vector<unsigned int> obj_indices;
	std::vector<unsigned int> normal_indices;
	for (size_t s = 0; s < shapes.size(); s++) {
		size_t face_offset = 0;
		//for each face in mesh
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			int fv = shapes[s].mesh.num_face_vertices[f];
			//for each vertex in face. Number of vertices per face is given by obj file,
			//but if triangulate is specifyed in reader config then is set to 3 everywhere. 
			for (size_t v = 0; v < fv; v++) {
				//idx is the index in the vertices array
				tinyobj::index_t idx = shapes[s].mesh.indices[face_offset + v];
				obj_indices.push_back((unsigned int)idx.vertex_index);
				//indices[shape_offset + face_offset + v] = idx.vertex_index;
				normal_indices.push_back((unsigned int)idx.normal_index);
			}
			face_offset += fv;
		}
		shape_offset += face_offset;
	}

	unsigned* indices = (unsigned*)rtcSetNewGeometryBuffer(geom,
		RTC_BUFFER_TYPE_INDEX,
		0,
		RTC_FORMAT_UINT3,
		3 * sizeof(unsigned),
		obj_indices.size() / 3); //FACE COUNT (3 indices are counted as 1 item since they represent a single triangle)

	std::copy(obj_indices.begin(), obj_indices.end(), indices);

	//NORMALS ------------------------------------------------------------------------
	/*Embree calculates normals for triangle meshes, so there is no need to create a normal buffer.
	Normals follow right hand rule from vertex order.*/

	auto obj_normals = reader.GetAttrib().normals;
	/*OBJs have different indices for vertices, UVs and normals.
	We need to create a new normal vector that stores the normal for vector[i] in the i position*/
	//Note this method is inneficient since it writes the same memory more than one time in some cases.
	float * all_normals = (float*)malloc(obj_vertices.size()*sizeof(float));
	for (int i = 0; i < normal_indices.size(); ++i) {
		auto vertex_index = obj_indices[i];
		auto normal_index = normal_indices[i];

		memcpy(&all_normals[vertex_index * 3],
			&obj_normals[normal_index * 3], 
			sizeof(float) * 3);
	}

	//Create mesh in scene
	Mesh * mesh = new Mesh(obj_vertices, obj_indices, all_normals);
	this->meshes.push_back(mesh);

	rtcCommitGeometry(geom);

	rtcAttachGeometry(rtc_scene, geom);
	rtcReleaseGeometry(geom);
}

void Scene::commitScene() {
	rtcCommitScene(this->rtc_scene);
}

void Scene::draw() {
	for (int i = 0; i < this->meshes.size(); ++i) {
		this->meshes[i]->draw();
	}
}

RTCScene Scene::getRTCScene() {
	return this->rtc_scene;
}

Scene::~Scene() {
	for (int i = 0; i < this->meshes.size(); ++i) {
		delete(this->meshes[i]);
	}
	rtcReleaseScene(this->rtc_scene);
}