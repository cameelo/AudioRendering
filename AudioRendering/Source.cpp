#include "Source.h"
#include <GL/glew.h>

Source::Source(glm::vec3 position, float radius) {
	this->pos = position;
	this->sphere_radius = radius;
}
void Source::draw() {
	GLUquadricObj *pObj = gluNewQuadric();
	glPushMatrix();
	glTranslatef(this->pos.x, this->pos.y, this->pos.z);
	gluSphere(pObj, this->sphere_radius, 25, 25);
	glPopMatrix();
}

Source::~Source() {

}