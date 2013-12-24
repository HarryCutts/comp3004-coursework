#version 330 core
in vec3 msPosition;
in vec3 msNormal;
out vec3 csEyeDirection;
out vec3 csLightDirection;
out vec3 csNormal;

uniform mat4 MVP;
uniform mat4 M;
uniform mat4 V;

uniform vec3 wsLightPosition;

void main() {
	// Code adapted from http://opengl-tutorial.org/beginners-tutorials/tutorial-8-basic-shading/
	gl_Position = MVP * vec4(msPosition, 1);

	//wsPosition = (M * vec4(msPosition, 1)).xyz;

	vec3 csPosition = (V * M * vec4(msPosition, 1)).xyz;
	csEyeDirection = vec3(0,0,0) - csPosition;

	vec3 csLightPosition = (V * vec4(wsLightPosition, 1)).xyz;
	csLightDirection = csLightPosition + csEyeDirection;

	csNormal = (V * M * vec4(msNormal, 0)).xyz;
		// Only correct if ModelMatrix does not scale the model! Use its inverse transpose if not.
}
