#version 330 core
in vec3 msPosition;
out vec3 msPoint;    // The point which goes into the geometry shader

void main() {
	msPoint = msPosition;
}
