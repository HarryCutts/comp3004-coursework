#version 330 core
in vec3 csNormal;
in vec3 csLightDirection;
in vec3 csEyeDirection;
in vec2 uvTexCoord;
out vec3 color;

uniform sampler2D diffuseTexture;

uniform vec3 specularColor;
uniform vec3 lightColor;
//uniform vec3 lightVector;

void main() {
	// Code adapted from http://opengl-tutorial.org/beginners-tutorials/
	vec3 diffuseColor = texture2D(diffuseTexture, uvTexCoord).rgb;

	vec3 n = normalize(csNormal);
	vec3 l = normalize(csLightDirection);

	float cosTheta = clamp(dot(n, l), 0, 1);

	// Ambient lighting
	vec3 ambientColor = vec3(0.1, 0.1, 0.1) * diffuseColor;

	// Specular reflection
	vec3 E = normalize(csEyeDirection);
	vec3 R = reflect(-l, n);

	float cosAlpha = clamp(dot(E, R), 0, 1);

	color = ambientColor +
	        diffuseColor * lightColor * cosTheta +
	        specularColor * lightColor * pow(cosAlpha, 5);  // Increase 5 for a thinner lobe
	// TODO: make the light fade by distance to the source?
}
