#version 330 core

layout(location = 0) out vec4 outColor;

in vec3 passPosition;

uniform vec3 vista;

const vec3 lightPos = vec3(0,0,5);
const float refFactor = 0.5; 
const vec3 lightColor = vec3(1,1,1);
const vec3 baseColor = vec3(0.5,0.5,0.5);
const vec3 normal = vec3(0,0,1);

void main(){
	vec3 lightDir = normalize(passPosition-lightPos);
	vec3 reflected = normalize(reflect(lightDir,normal)); //Reflected is (-symmetric)
	vec3 diffColor = max(dot(-lightDir,normal),0)*baseColor;
	vec3 specColor = pow(max(dot(reflected,-normalize(vista)),0),10)*lightColor*refFactor;
	outColor = vec4( (specColor+diffColor).xyz , 1.0 );
}