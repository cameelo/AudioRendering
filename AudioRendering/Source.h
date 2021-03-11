#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

class Source {
public:
	glm::vec3 pos;
	float sphere_radius;

public:
	Source(glm::vec3 position, float radius);
	void draw();
	~Source();
};