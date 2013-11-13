#version 330 core
in vec3 vertexPosition;
out vec3 normal;

uniform mat4 MVP;

void main() {
	vec4 v = vec4(vertexPosition, 1);
	normal = vertexPosition;
	gl_Position = MVP * v;
}
