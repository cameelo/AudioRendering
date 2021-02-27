#include "pch.h"
#include "Mesh.h"
#include <exception>
#include <iostream>
#include<fstream>
#include<string>
#include<iterator>

#include <glm/gtc/type_ptr.hpp>

int MESH_WIDTH = 500;
int MESH_LENGTH = 500;

Mesh::Mesh(){

	std::vector<float> positions;

	positions.push_back(-5.0f); positions.push_back(-5.0f); positions.push_back(0.0f);
	positions.push_back(5.0f); positions.push_back(-5.0f); positions.push_back(0.0f);
	positions.push_back(5.0f); positions.push_back(5.0f); positions.push_back(0.0f);
	positions.push_back(5.0f); positions.push_back(5.0f); positions.push_back(0.0f);
	positions.push_back(-5.0f); positions.push_back(5.0f); positions.push_back(0.0f);
	positions.push_back(-5.0f); positions.push_back(-5.0f); positions.push_back(0.0f);

	glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);


	glGenBuffers(1, &verticesID);
	glBindBuffer(GL_ARRAY_BUFFER, this->verticesID);
	glBufferData(GL_ARRAY_BUFFER, positions.size()*sizeof(float), &positions[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	textured = false;
}

void Mesh::draw() {
	glBindVertexArray(vaoID);
	glEnableVertexAttribArray(0);
	if(textured) glEnableVertexAttribArray(1);
	glDrawArrays(GL_TRIANGLES,0,6);
	glDisableVertexAttribArray(0);
	if(textured) glDisableVertexAttribArray(1);
}

void Mesh::addTexture(std::vector<float> textcoords) {
	glBindVertexArray(vaoID);
	glGenBuffers(1, &textureID);
	glBindBuffer(GL_ARRAY_BUFFER, this->textureID);
	glBufferData(GL_ARRAY_BUFFER, textcoords.size() * sizeof(float), &textcoords[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	textured = true;
}

void Mesh::setShader(GLuint shaderID) {
	this->shaderID = shaderID;
}

Mesh::~Mesh() {
	glDeleteBuffers(1, &this->verticesID);
	glDeleteBuffers(1, &this->indicesID);
	if(textured)glDeleteBuffers(1, &this->textureID);
	glDeleteVertexArrays(1, &vaoID);
}
