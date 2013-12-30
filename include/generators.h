#ifndef _GENERATORS_H
#define _GENERATORS_H

struct Mesh {
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;
	std::vector<GLuint> indices;
};


Mesh generateIcosahedron(void);
Mesh generateSphere(int numIterations);
Mesh generateCone(void);

Mesh loadOBJ(const char* path);

#endif
