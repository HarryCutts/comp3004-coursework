#include <stdio.h>
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

struct Motion {
	DisplayObject *target;

	glm::vec3 startLocation;
	glm::vec3 startRotation;

	glm::vec3 moveBy;
	glm::vec3 rotateBy;

	float duration;

	float secondsComplete;
};

static DisplayObject landscape, spaceship, clanger;
static std::vector<Motion> motions;

void setupScene(std::vector<DisplayObject*> &objects) {
	glm::vec3 spaceshipEndLocation = glm::vec3(10, 0, 12);
	glm::vec3 clangerLocation = glm::vec3(4.29, -1.0, -30);

	// Static part //
	objects.clear();
	Mesh landscapeMesh = loadOBJ(MODEL("landscape.obj"));
	landscape = createDisplayObject(landscapeMesh, TEXTURE("landscape.tga"));
	landscape.scale = 33;
	updateModelMatrix(landscape);
	objects.push_back(&landscape);

	Mesh spaceshipMesh = loadOBJ(MODEL("spaceship.obj"));
	spaceship = createDisplayObject(spaceshipMesh, TEXTURE("spaceship.tga"));
	spaceship.location = spaceshipEndLocation;
	spaceship.rotation = glm::vec3(0, 180, 0);
	spaceship.scale = 3;
	updateModelMatrix(spaceship);
	objects.push_back(&spaceship);

	Mesh clangerMesh = loadOBJ(MODEL("clanger.obj"));
	clanger = createDisplayObject(clangerMesh, TEXTURE("clanger.tga"));
	clanger.location = clangerLocation;
	clanger.rotation = glm::vec3(0, -90, 0);
	updateModelMatrix(clanger);
	objects.push_back(&clanger);

	// Animation //
	Motion spaceshipMotion;
	spaceshipMotion.target = &spaceship;
	spaceshipMotion.startLocation = glm::vec3(51.2, 80.0, -320);
	spaceshipMotion.startRotation = glm::vec3(15, 180, 0);

	spaceshipMotion.moveBy = spaceshipEndLocation - spaceshipMotion.startLocation;
	spaceshipMotion.rotateBy = glm::vec3(-18, 0, 0);

	spaceshipMotion.duration = 5;
	motions.push_back(spaceshipMotion);
}

static bool tourRunning = false;
static unsigned int motionIndex;

static void startMotion(unsigned int index) {
	motionIndex = index;
	Motion &motion = motions[index];
	motion.target->location = motion.startLocation;
	motion.target->rotation = motion.startRotation;
	updateModelMatrix(*motion.target);
	motion.secondsComplete = 0;
}

static void nextMotion(void) {
	if (motionIndex + 1 < motions.size()) {
		startMotion(motionIndex + 1);
	} else {
		printf("Tour finished.\n");
		tourRunning = false;
	}
}

void startTour(void) {
	tourRunning = true;
	startMotion(0);
}

bool isTourRunning(void) {
	return tourRunning;
}

void animate(float timePassed) {
	if (tourRunning) {
		// Perform the motion
		Motion &motion = motions[motionIndex];
		glm::vec3 movement = (motion.moveBy / motion.duration) * timePassed;
		glm::vec3 rotation = (motion.rotateBy / motion.duration) * timePassed;
		DisplayObject *target = motion.target;
		target->location += movement;
		target->rotation += rotation;
		updateModelMatrix(*target);

		// Check whether the motion has finished
		motion.secondsComplete += timePassed;
		if (motion.secondsComplete >= motion.duration) {
			printf("Motion finished.\n");
			nextMotion();
		}
	}
}
