#include <stdlib.h>
#include <math.h>
#include <vector>

#include <GL/glfw.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "generators.h"

#include <stdio.h>

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

static glm::vec3 normalize(glm::vec3 v) {
	GLfloat length = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	return glm::vec3(v.x / length, v.y / length, v.z / length);
}

static Mesh subdivide(Mesh &mesh) {
	Mesh newMesh;
	newMesh.vertices = mesh.vertices;
	for (size_t i = 0; i < mesh.indices.size(); i += 3) {
		GLuint aIndex = mesh.indices[i],
		       bIndex = mesh.indices[i+1],
			   cIndex = mesh.indices[i+2];
		glm::vec3 a = mesh.vertices[aIndex];
		glm::vec3 b = mesh.vertices[bIndex];
		glm::vec3 c = mesh.vertices[cIndex];

		// Calculate the new vertex
		glm::vec3 n = glm::vec3((a.x + b.x + c.x) / 3, (a.y + b.y + c.y) / 3, (a.z + b.z + c.z) / 3);
		n = normalize(n);
		newMesh.vertices.push_back(n);

		// Add the new triangles
		size_t nIndex = newMesh.vertices.size() - 1;
		newMesh.indices.push_back(aIndex);  // abn
		newMesh.indices.push_back(bIndex);
		newMesh.indices.push_back(nIndex);

		newMesh.indices.push_back(bIndex);  // bcn
		newMesh.indices.push_back(cIndex);
		newMesh.indices.push_back(nIndex);

		newMesh.indices.push_back(aIndex);  // acn
		newMesh.indices.push_back(cIndex);
		newMesh.indices.push_back(nIndex);
	}
	return newMesh;
}

Mesh generateSphere(int numIterations) {
	const glm::vec3 vertices[] = {
		glm::vec3(0, 1, 0), glm::vec3(0, -1, 0),
		glm::vec3(1, 0, 0), glm::vec3(0, 0, -1), glm::vec3(-1, 0, 0), glm::vec3(0, 0, 1)
	};
	const GLuint indices[] = {
		0,2,3,  0,3,4,  0,4,5,  0,5,2,
		1,2,3,  1,3,4,  1,4,5,  1,5,2
	};

	Mesh m;
	m.vertices = std::vector<glm::vec3>(&vertices[0], &vertices[6]);
	m.indices  = std::vector<GLuint>(&indices[0], &indices[24]);

	for (int i = 0; i < numIterations; i++) {
		m = subdivide(m);
	}

	return m;
}
