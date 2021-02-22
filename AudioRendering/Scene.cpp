#include "Scene.h"

Scene::Scene(std::string file_name, RTCDevice device) {
	this->rtc_scene = rtcNewScene(device);

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

	size_t index_count = 0;
	for (size_t s = 0; s < shapes.size(); s++) {
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			index_count += shapes[s].mesh.num_face_vertices[f];
		}
	}

	unsigned* indices = (unsigned*)rtcSetNewGeometryBuffer(geom,
		RTC_BUFFER_TYPE_INDEX,
		0,
		RTC_FORMAT_UINT3,
		3 * sizeof(unsigned),
		index_count / 3); //FACE COUNT (3 indices are counted as 1 item since they represent a single triangle)

	size_t shape_offset = 0;
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
				indices[shape_offset + face_offset + v] = idx.vertex_index;
			}
			face_offset += fv;
		}
		shape_offset += face_offset;
	}

	rtcCommitGeometry(geom);

	rtcAttachGeometry(rtc_scene, geom);
	rtcReleaseGeometry(geom);

	rtcCommitScene(rtc_scene);
}

RTCScene Scene::getRTCScene() {
	return this->rtc_scene;
}

Scene::~Scene() {
	rtcReleaseScene(this->rtc_scene);
}