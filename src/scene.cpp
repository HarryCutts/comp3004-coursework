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

#define CAMERA_START_POSITION glm::vec3(-115, 19.5, 11.6)
#define CAMERA_START_YAW -23.1
#define CAMERA_START_PITCH 0.13

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

	glm::vec3 moveBy;
	glm::vec3 rotateBy;

	float duration;

	float secondsComplete;
	public:
		Motion(DisplayObject*, float);
		Motion(DisplayObject *myTarget, float myDuration, glm::vec3 myMoveBy, glm::vec3 myRotateBy);

		void start(void);
		bool perform(float timePassed);
};

Motion::Motion(DisplayObject *myTarget, float myDuration) {
	target = myTarget;
	duration = myDuration;
}

Motion::Motion(DisplayObject *myTarget, float myDuration, glm::vec3 myMoveBy, glm::vec3 myRotateBy) {
	target = myTarget;
	duration = myDuration;
	moveBy = myMoveBy;
	rotateBy = myRotateBy;
}

void Motion::start(void) {
	secondsComplete = 0;
}

bool Motion::perform(float timePassed) {
	if (duration == 0) {
		// This is a "set" motion
		target->location = moveBy;
		target->rotation = rotateBy;
		updateModelMatrix(*target);
		return true;
	}

	glm::vec3 movement = (moveBy / duration) * timePassed;
	glm::vec3 rotation = (rotateBy / duration) * timePassed;
	target->location += movement;
	target->rotation += rotation;
	updateModelMatrix(*target);

	secondsComplete += timePassed;
	return secondsComplete >= duration;
}

static DisplayObject *camera;
static DisplayObject landscape, spaceship, clanger;
static std::vector<Motion> motions;

void setupScene(std::vector<DisplayObject*> &objects, DisplayObject &cameraObject) {
	glm::vec3 spaceshipEndLocation = glm::vec3(10, 0, 12);
	glm::vec3 clangerLocation = glm::vec3(4.29, -1.0, -30);

	// Initial camera position //
	camera = &cameraObject;
	camera->location = CAMERA_START_POSITION;
	camera->rotation = glm::vec3(CAMERA_START_PITCH, CAMERA_START_YAW, 0);

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
	clanger.rotation = glm::vec3(0, 0, 0);
	updateModelMatrix(clanger);
	objects.push_back(&clanger);

	// Animation //
	glm::vec3 zero = glm::vec3(0, 0, 0);
	glm::vec3 spaceshipStartLocation = glm::vec3(51.2, 80.0, -320);

	Motion cameraSet(camera, 0, glm::vec3(18.940031, 2.809827, -32.122253), glm::vec3(-0.050559, -26.546928, 0));
	Motion spaceshipSet(&spaceship, 0, spaceshipStartLocation, glm::vec3(15, 180, 0));

	Motion clangerMotion1(&clanger, 2, zero, glm::vec3(0, 75, 0));
	Motion clangerMotion2(&clanger, 4, zero, glm::vec3(0, -150, 0));
	Motion clangerMotion3(&clanger, 3, zero, glm::vec3(0, 150, 0));
	Motion clangerPause(&clanger, 1);
	Motion clangerMotion4(&clanger, 0.3, glm::vec3(0, 0.4, 0), zero);
	Motion clangerMotion5(&clanger, 0.3, glm::vec3(0, -3.4, 0), zero);

	Motion spaceshipMotion(&spaceship, 5, spaceshipEndLocation - spaceshipStartLocation, glm::vec3(-18, 0, 0));

	Motion clangerEndMotion1(&clanger, 0.01, zero, glm::vec3(0, -150, 0));
	Motion clangerEndMotion2(&clanger, 5, glm::vec3(0, 2.5, 0), zero);

	motions.push_back(cameraSet);
	motions.push_back(spaceshipSet);
	motions.push_back(clangerMotion1);
	motions.push_back(clangerMotion2);
	motions.push_back(clangerMotion3);
	motions.push_back(clangerPause);
	motions.push_back(clangerMotion4);
	motions.push_back(clangerMotion5);
	motions.push_back(spaceshipMotion);
	motions.push_back(clangerEndMotion1);
	motions.push_back(clangerEndMotion2);
}

static bool tourRunning = false;
static unsigned int motionIndex;

static void startMotion(unsigned int index) {
	motionIndex = index;
	motions[motionIndex].start();
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
		bool motionFinished = motions[motionIndex].perform(timePassed);

		if (motionFinished) {
			nextMotion();
		}
	}
}
