#include <stdlib.h>
#include <math.h>
#include <vector>

#include <GL/glfw.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "generators.h"

Mesh generateIcosahedron() {
	float x = 0.525731112119133606f;
	float z = 0.850650808352039932f;
	const glm::vec3 vertices[] = {
		glm::vec3(-x,0.0f,z), glm::vec3(x,0.0f,z),  glm::vec3(-x,0.0f,-z), glm::vec3(x,0.0f,-z),
		glm::vec3(0.0f,z,x),  glm::vec3(0.0f,z,-x), glm::vec3(0.0f,-z,x),  glm::vec3(0.0f,-z,-x),
		glm::vec3(z,x,0.0f),  glm::vec3(-z,x,0.0f), glm::vec3(z,-x,0.0f),  glm::vec3(-z,-x,0.0f)
	};
	const GLuint indices[] = {
		1,4,0,     4,9,0,    4,5,9,    8,5,4,    1,8,4,
		1,10,8,    10,3,8,   8,3,5,    3,2,5,    3,7,2,
		3,10,7,    10,6,7,   6,11,7,   6,0,11,   6,1,0,
		10,1,6,    11,0,9,   2,11,9,   5,2,9,    11,2,7
	};
	Mesh m;
	m.vertices = std::vector<glm::vec3>(&vertices[0], &vertices[12]);
	m.indices  = std::vector<GLuint>(&indices[0], &indices[60]);
	return m;
}
