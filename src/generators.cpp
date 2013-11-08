#include <stdlib.h>
#include <math.h>
#include <vector>

#include <GL/glfw.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

std::vector<glm::vec3> generateIcosahedron() {
	float x = 0.525731112119133606f;
	float z = 0.850650808352039932f;
	const GLfloat vertices[][3] = {
		{-x,0.0f,z}, {x,0.0f,z},  {-x,0.0f,-z}, {x,0.0f,-z},
		{0.0f,z,x},  {0.0f,z,-x}, {0.0f,-z,x},  {0.0f,-z,-x},
		{z,x,0.0f},  {-z,x,0.0f}, {z,-x,0.0f},  {-z,-x,0.0f}
	};
	const int indices[] = {
		1,4,0,     4,9,0,    4,5,9,    8,5,4,    1,8,4,
		1,10,8,    10,3,8,   8,3,5,    3,2,5,    3,7,2,
		3,10,7,    10,6,7,   6,11,7,   6,0,11,   6,1,0,
		10,1,6,    11,0,9,   2,11,9,   5,2,9,    11,2,7
	};

	std::vector<glm::vec3> triangles;
	for (int i = 0; i < sizeof(indices) / sizeof(int); i++) {
		triangles.push_back(glm::vec3(vertices[indices[i]][0],
		                              vertices[indices[i]][1],
									  vertices[indices[i]][2]));
	}
	return triangles;
}
