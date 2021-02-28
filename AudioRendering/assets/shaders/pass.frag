#version 330 core
in vec3 Normal;

out vec4 color;

uniform vec3 lightDir;  

void main(){
	//vec3 norm = normalize(Normal);
	float diffuse_factor = max(dot(Normal, -lightDir), 0.0);
	color = (0.2 + diffuse_factor)*vec4(1,1,1,1); 
}