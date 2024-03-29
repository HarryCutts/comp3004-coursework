#version 330 core
layout (points) in;
layout (line_strip, max_vertices = 2) out;

in vec3 msPosition2[];
in vec3 msNormal2[];
uniform mat4 MVP;

void main() {
	// Emit the vertex itself as the start of the line
	gl_Position = MVP * vec4(msPosition2[0], 1.0);
	EmitVertex();

	// Emit the end point of the line
	gl_Position = MVP * (vec4(msPosition2[0] + msNormal2[0], 1.0));
	EmitVertex();

	EndPrimitive();
}
