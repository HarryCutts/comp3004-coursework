#include <vector>
#include <GL/glew.h>
#include <GL/glfw.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "paths.h"
#include "utils.h"
#include "generators.h"

#include "scene.hpp"

void updateModelMatrix(DisplayObject &object) {
	glm::mat4 rotateX = glm::rotate(glm::mat4(1.), object.rotation[0], glm::vec3(1, 0, 0));
	glm::mat4 rotateY = glm::rotate(glm::mat4(1.), object.rotation[1], glm::vec3(0, 1, 0));
	glm::mat4 rotateZ = glm::rotate(glm::mat4(1.), object.rotation[2], glm::vec3(0, 0, 1));
	glm::mat4 scale = glm::scale(glm::mat4(1.), glm::vec3(object.scale, object.scale, object.scale));
	glm::mat4 translate  = glm::translate(glm::mat4(1.), object.location);
	glm::mat4 matrix = translate * scale * rotateZ * rotateY * rotateX;

	object.modelMatrix = matrix;
}

/** Creates a Vertex Buffer Object, fills it with the given items, and binds it
 * as a Vertex Attribute Array.
 * @param index         The index of the generic vertex attribute to be modified.
 * @param numComponents The number of components per generic vertex attribute.
 * @param items         A std::vector containing the data.
 * @tparam T The type of the items in the data.
 * @return the index of the Vertex Buffer Object (prefix `vbo`).
 */
template <class T>
static GLuint createVertexAttribVBO(GLuint index, GLint numComponents, const std::vector<T> &items) {
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(T) * items.size(), items.data(), GL_STATIC_DRAW);

	// Bind as vertex attribute array
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, numComponents, GL_FLOAT, GL_FALSE, 0, 0);

	return vbo;
}

static DisplayObject createDisplayObject(const Mesh &mesh, const char *texturePath) {
	// Create a VAO
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create vertex attribute VBOs
	createVertexAttribVBO<glm::vec3>(0, 3, mesh.vertices);
	createVertexAttribVBO<glm::vec3>(1, 3, mesh.normals);
	createVertexAttribVBO<glm::vec2>(2, 2, mesh.texCoords);

	// Indices VBO
	GLuint vboIndices;
	glGenBuffers(1, &vboIndices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh.indices.size(), mesh.indices.data(), GL_STATIC_DRAW);

	// DisplayObject
	DisplayObject obj;
	obj.vao = vao;
	obj.numVertices = mesh.vertices.size();
	obj.numIndices  = mesh.indices.size();
	obj.tex = loadTGA(texturePath);  // TODO: prevent textures being loaded twice
	obj.location = glm::vec3(0., 0., 0.);
	obj.rotation = glm::vec3(0., 0., 0.);
	obj.scale = 1;

	return obj;
}

static DisplayObject landscape, crate, clanger;

void setupScene(std::vector<DisplayObject*> &objects) {
	glm::vec3 clangerLocation = glm::vec3(4.29, -1.0, -30);

	objects.clear();
	Mesh landscapeMesh = loadOBJ(MODEL("landscape.obj"));
	landscape = createDisplayObject(landscapeMesh, TEXTURE("landscape.tga"));
	landscape.scale = 33;
	updateModelMatrix(landscape);
	objects.push_back(&landscape);

	Mesh clangerMesh = loadOBJ(MODEL("clanger.obj"));
	clanger = createDisplayObject(clangerMesh, TEXTURE("clanger.tga"));
	clanger.location = clangerLocation;
	clanger.rotation = glm::vec3(0, -90, 0);
	updateModelMatrix(clanger);
	objects.push_back(&clanger);
}

void animate(float timePassed) {
}
