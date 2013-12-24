#version 330 core
in vec3 csNormal;
in vec3 csLightDirection;
out vec3 color;

uniform vec3 materialDiffuseColor;
uniform vec3 lightColor;
//uniform vec3 lightVector;

void main() {
	// Code adapted from http://opengl-tutorial.org/beginners-tutorials/tutorial-8-basic-shading/
	vec3 n = normalize(csNormal);
	vec3 l = normalize(csLightDirection);

	float cosTheta = dot(n, l);

	color = materialDiffuseColor * lightColor * cosTheta;
	// TODO: make the light fade by distance to the source?
}
