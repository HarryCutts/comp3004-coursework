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
		glm::vec3 ab = glm::vec3((a.x + b.x) / 3, (a.y + b.y) / 3, (a.z + b.z) / 3);
		glm::vec3 bc = glm::vec3((b.x + c.x) / 3, (b.y + c.y) / 3, (b.z + c.z) / 3);
		glm::vec3 ca = glm::vec3((c.x + a.x) / 3, (c.y + a.y) / 3, (c.z + a.z) / 3);
		ab = normalize(ab);
		bc = normalize(bc);
		ca = normalize(ca);
		newMesh.vertices.push_back(ab);
		newMesh.vertices.push_back(bc);
		newMesh.vertices.push_back(ca);

		// Add the new triangles
		size_t abIndex = newMesh.vertices.size() - 3;
		size_t bcIndex = newMesh.vertices.size() - 2;
		size_t caIndex = newMesh.vertices.size() - 1;
		newMesh.indices.push_back( aIndex);  // a-ab-ca
		newMesh.indices.push_back(abIndex);
		newMesh.indices.push_back(caIndex);

		newMesh.indices.push_back( bIndex);  // b-bc-ab
		newMesh.indices.push_back(bcIndex);
		newMesh.indices.push_back(abIndex);

		newMesh.indices.push_back( cIndex);  // c-ba-bc
		newMesh.indices.push_back(caIndex);
		newMesh.indices.push_back(bcIndex);

		newMesh.indices.push_back(abIndex);  // ab-bc-ca
		newMesh.indices.push_back(bcIndex);
		newMesh.indices.push_back(caIndex);
	}
	return newMesh;
}

Mesh generateSphere(int numIterations) {
	Mesh m = generateIcosahedron();

	for (int i = 0; i < numIterations; i++) {
		m = subdivide(m);
	}

	return m;
}
