#include <stdio.h>
#include <math.h>
#include <vector>
#include <GL/glew.h>
#include <GL/glfw.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "paths.h"
#include "utils.h"
#include "generators.h"

#include "scene.hpp"

#define PI 3.14159265

#define CAMERA_START_POSITION glm::vec3(-115, 19.5, 11.6)
#define CAMERA_START_YAW -23.1
#define CAMERA_START_PITCH 0.13

#define GROUND_SHAKE_MAGNITUDE 0.5
#define GROUND_SHAKE_DURATION 0.05
#define NUM_GROUND_SHAKES 3

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

void checkForFinish(void);

struct MotionSequence {
	std::vector<Motion> motions;
	unsigned int index;
	bool finished;

	public:
		MotionSequence(void);
		void startMotion(unsigned int newIndex);
		void nextMotion(void);
};

MotionSequence::MotionSequence(void) { finished = false; }

void MotionSequence::startMotion(unsigned int newIndex) {
	index = newIndex;
	finished = false;
	motions[index].start();
}

void MotionSequence::nextMotion(void) {
	if (index + 1 < motions.size()) {
		startMotion(index + 1);
	} else {
		finished = true;
		checkForFinish();
	}
}

static std::vector<MotionSequence> sequences;
static bool tourRunning = false;

void checkForFinish(void) {
	bool finished = true;
	for (unsigned int i = 0; i < sequences.size(); i++) {
		if (!sequences[i].finished) {
			finished = false;
			break;
		}
	}
	if (finished) {
		printf("Tour finished.\n");
		tourRunning = false;
	}
}

void startTour(void) {
	tourRunning = true;
	for (unsigned int i = 0; i < sequences.size(); i++) {
		sequences[i].startMotion(0);
	}
}

bool isTourRunning(void) {
	return tourRunning;
}

void animate(float timePassed) {
	if (tourRunning) {
		for (unsigned int i = 0; i < sequences.size(); i++) {
			if (sequences[i].finished) continue;
			bool motionFinished = sequences[i].motions[sequences[i].index].perform(timePassed);

			if (motionFinished) {
				sequences[i].nextMotion();
			}
		}
	}
}

static DisplayObject *camera;
static DisplayObject landscape, spaceship, clanger;

void setupScene(std::vector<DisplayObject*> &objects, DisplayObject &cameraObject) {
	glm::vec3 spaceshipEndLocation = glm::vec3(10, 0, 12);
	glm::vec3 spaceshipEndRotation = glm::vec3(-3, 180, 0);
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
	spaceship.rotation = spaceshipEndRotation;
	spaceship.scale = 3;
	updateModelMatrix(spaceship);
	objects.push_back(&spaceship);

	Mesh clangerMesh = loadOBJ(MODEL("clanger.obj"));
	clanger = createDisplayObject(clangerMesh, TEXTURE("clanger.tga"));
	clanger.location = clangerLocation;
	clanger.rotation = glm::vec3(0, 0, 0);
	updateModelMatrix(clanger);
	objects.push_back(&clanger);

	// Tour Animation: Models //
	glm::vec3 zero = glm::vec3(0, 0, 0);
	glm::vec3 spaceshipStartLocation = glm::vec3(128, 100, -800);
	glm::vec3 spaceshipStartRotation = glm::vec3(15, 180, 0);
	glm::vec3 spaceshipPath         = spaceshipEndLocation - spaceshipStartLocation;
	glm::vec3 spaceshipRotationPath = spaceshipEndRotation - spaceshipStartRotation;

	MotionSequence modelSequence;
	Motion spaceshipSet(&spaceship, 0, spaceshipStartLocation, spaceshipStartRotation);

	Motion clangerMotion1(&clanger, 2, zero, glm::vec3(0, 75, 0));
	Motion clangerMotion2(&clanger, 4, zero, glm::vec3(0, -150, 0));
	Motion clangerMotion3(&clanger, 3, zero, glm::vec3(0, 150, 0));
	Motion clangerPause1(&clanger, 1);
	Motion clangerMotion4(&clanger, 0.1, glm::vec3(0, 0.4, 0), zero);
	Motion clangerMotion5(&clanger, 0.1, glm::vec3(0, -3.4, 0), zero);
	Motion clangerPause2(&clanger, 0.4);

	Motion spaceshipMotion(&spaceship, 10, spaceshipPath, spaceshipRotationPath);
	Motion groundShakeUp(&landscape, GROUND_SHAKE_DURATION, glm::vec3(0, GROUND_SHAKE_MAGNITUDE, 0), zero);
	Motion groundShakeDown(&landscape, GROUND_SHAKE_DURATION, glm::vec3(0, -GROUND_SHAKE_MAGNITUDE, 0), zero);
	Motion groundShakePause(&landscape, 1);

	Motion clangerEndSet1(&clanger, 0, clangerLocation - glm::vec3(0, 3, 0), glm::vec3(0, -75, 0));
	Motion clangerEndMotion1(&clanger, 5, glm::vec3(0, 3, 0), zero);

	modelSequence.motions.push_back(spaceshipSet);
	modelSequence.motions.push_back(clangerMotion1);
	modelSequence.motions.push_back(clangerMotion2);
	modelSequence.motions.push_back(clangerMotion3);
	modelSequence.motions.push_back(clangerPause1);
	modelSequence.motions.push_back(clangerMotion4);
	modelSequence.motions.push_back(clangerMotion5);
	modelSequence.motions.push_back(clangerPause2);
	modelSequence.motions.push_back(spaceshipMotion);
	for (unsigned int i = 0; i < NUM_GROUND_SHAKES; i++) {
		modelSequence.motions.push_back(groundShakeDown);
		modelSequence.motions.push_back(groundShakeUp);
	}
	modelSequence.motions.push_back(groundShakePause);
	modelSequence.motions.push_back(clangerEndSet1);
	modelSequence.motions.push_back(clangerEndMotion1);

	sequences.push_back(modelSequence);

	// Tour Animation: Camera //
	MotionSequence cameraSequence;
	Motion clangerView1(camera, 0, glm::vec3(18.940031, 2.809827, -32.122253), glm::vec3(-0.050559, -26.546928, 0));
	Motion cameraPause1(camera, 10.6);

		// (spaceship starts moving)
	Motion cameraSet2(camera, 0, glm::vec3(27.815826, 6.820219, -65.316856), glm::vec3(0.183383, -27.983179, 0));
	Motion cameraPause2(camera, 5);
	Motion cameraSet3(camera, 0, spaceshipStartLocation + (0.5f * spaceshipPath) + glm::vec3(0, -3, 0),
			(float)(PI/180) * (spaceshipStartRotation - glm::vec3(0, 180, 0)));
	Motion cameraMotion1(camera, 4, 0.35f * spaceshipPath, 0.35f * (float)(PI/180) * spaceshipRotationPath);
	Motion cameraPause3(camera, 1);
		// (spaceship hits, ground starts shaking)
	Motion cameraSet4(camera, 0, glm::vec3(-27.063541, 4.131834, -82.903534), glm::vec3(-0.057055, -28.964642, 0));
			// View of hangar doors
	Motion groundShakeCameraPause(camera, GROUND_SHAKE_DURATION * NUM_GROUND_SHAKES + 1);
	Motion clangerView2(camera, 0, glm::vec3(38.303978, 4.071294, 1.528273), glm::vec3(-0.070362, -27.034525, 0));
			// View of clanger and crashed ship

	cameraSequence.motions.push_back(clangerView1);
	cameraSequence.motions.push_back(cameraPause1);
	cameraSequence.motions.push_back(cameraSet2);
	cameraSequence.motions.push_back(cameraPause2);
	cameraSequence.motions.push_back(cameraSet3);
	cameraSequence.motions.push_back(cameraMotion1);
	cameraSequence.motions.push_back(cameraPause3);
	cameraSequence.motions.push_back(cameraSet4);
	cameraSequence.motions.push_back(groundShakeCameraPause);
	cameraSequence.motions.push_back(clangerView2);
	sequences.push_back(cameraSequence);
}

