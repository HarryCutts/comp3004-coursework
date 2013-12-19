#version 330 core
in vec3 vertexPosition;
in vec3 normal;
out vec3 normalOut;
//out vec3 normal;

uniform mat4 MVP;

void main() {
	vec4 v = vec4(vertexPosition, 1);
	//normal = vertexPosition;
	normalOut = normal;
	gl_Position = MVP * v;
}
