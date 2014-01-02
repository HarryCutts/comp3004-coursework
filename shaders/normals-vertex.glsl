#version 330 core
in vec3 msPosition;
in vec3 msNormal;
out vec3 msPosition2;    // The point which goes into the geometry shader
out vec3 msNormal2;

void main() {
	msPosition2 = msPosition;
	msNormal2   = msNormal;
}
