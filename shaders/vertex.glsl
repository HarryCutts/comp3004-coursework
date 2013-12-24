#version 330 core
in vec3 msPosition;
uniform mat4 MVP;

void main() {
	vec4 v = vec4(msPosition, 1);
	gl_Position = MVP * v;
}
