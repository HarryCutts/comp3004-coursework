#version 330 core
in vec3 vertexPosition_modelspace;
out vec3 vertexPosition;

void main() {
	vertexPosition = vertexPosition_modelspace;
}
