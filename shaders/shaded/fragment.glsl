#version 330 core
in vec3 normal;
out vec3 color;

uniform vec3 materialColor;
uniform vec3 lightColor;
uniform vec3 lightVector;

void main() {
	color = materialColor * lightColor * dot(lightVector, normal);
}
