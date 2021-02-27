#version 330 core

layout (location = 0) in vec3 position;

out vec3 passPosition;

uniform mat4 worldTransform;

void main(){
	passPosition = position;
	gl_Position = worldTransform * vec4(position,1.0);
}