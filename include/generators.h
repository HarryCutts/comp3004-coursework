#ifndef _GENERATORS_H
#define _GENERATORS_H

struct Mesh {
	std::vector<glm::vec3> vertices;
	std::vector<GLuint> indices;
};


Mesh generateIcosahedron(void);
Mesh generateSphere(int numIterations);
Mesh generateCone(void);

#endif
