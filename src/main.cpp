#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
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

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600
#define WINDOW_TITLE  "Graphics coursework 3, Harry Cutts"

#define LIGHT_POSITION 5.f, 5.f, 3.f

#define CAMERA_ACCELERATION 2
#define CAMERA_ROTATION_SPEED 0.4

static GLuint prgNormals;
static GLuint prgShaded;

static DisplayObject camera;
static std::vector<DisplayObject*> objects;

static GLfloat cameraSpeed = 0;
static glm::mat4 VP, V, P;

static bool showNormals = false;

GLuint createShader(GLenum type, const char* path) {
	GLuint shdShader = glCreateShader(type);
	char* source = fileToBuffer(path);
	glShaderSource(shdShader, 1, (const GLchar**)&source, NULL);
	glCompileShader(shdShader);

	// Check for errors
	GLint result;
	glGetShaderiv(shdShader, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		int length;
		char message[1000];
		glGetShaderInfoLog(shdShader, 1000, &length, reinterpret_cast<GLchar*>(&message));
		if (length > 0) {
			fprintf(stderr, "Error(s) in %s:\n%s\n--end of errors--\n", path, message);
		}
	}

	return shdShader;
}

GLuint createProgram(GLuint shdVertex, GLuint shdGeometry, GLuint shdFragment) {
	GLuint prgProgram = glCreateProgram();
	if (shdVertex)   glAttachShader(prgProgram, shdVertex);
	if (shdGeometry) glAttachShader(prgProgram, shdGeometry);
	if (shdFragment) glAttachShader(prgProgram, shdFragment);
	glBindAttribLocation(prgProgram, 0, "msPosition");
	glBindAttribLocation(prgProgram, 1, "msNormal");
	glBindAttribLocation(prgProgram, 2, "uv");
	glBindFragDataLocation(prgProgram, 0, "color");
	glLinkProgram(prgProgram);

	// Check for errors
	GLint result;
	glGetProgramiv(prgProgram, GL_LINK_STATUS, &result);
	if (result == GL_FALSE) {
		int length;
		char message[1000];
		glGetProgramInfoLog(prgProgram, 1000, &length, reinterpret_cast<GLchar*>(&message));
		if (length > 0) {
			fprintf(stderr, "Error(s) in shader program:\n%s\n--end of errors--\n", message);
		}
	}

	return prgProgram;
}

// Setup methods //

void setupShaders(void) {
	// Normals program
	GLuint shdNormalVertex   = createShader(GL_VERTEX_SHADER,   SHADER("normals-vertex.glsl"));
	GLuint shdNormalGeometry = createShader(GL_GEOMETRY_SHADER, SHADER("normals-geometry.glsl"));
	GLuint shdNormalFragment = createShader(GL_FRAGMENT_SHADER, SHADER("normals-fragment.glsl"));

	prgNormals = createProgram(shdNormalVertex, shdNormalGeometry, shdNormalFragment);

	glDeleteShader(shdNormalGeometry);
	glDeleteShader(shdNormalVertex);
	glDeleteShader(shdNormalFragment);

	// Shaded program
	GLuint shdShadedVertex   = createShader(GL_VERTEX_SHADER,   SHADER("vertex.glsl"));
	GLuint shdShadedFragment = createShader(GL_FRAGMENT_SHADER, SHADER("fragment.glsl"));

	prgShaded = createProgram(shdShadedVertex, 0, shdShadedFragment);

	GLuint uni_diffuseColor    = glGetUniformLocation(prgShaded, "diffuseColor"),
	       uni_specularColor   = glGetUniformLocation(prgShaded, "specularColor"),
	       uni_lightColor      = glGetUniformLocation(prgShaded, "lightColor"),
	       uni_wsLightPosition = glGetUniformLocation(prgShaded, "wsLightPosition"),
	       uni_diffuseTexture  = glGetUniformLocation(prgShaded, "diffuseTexture");
	glProgramUniform3f(prgShaded, uni_diffuseColor,  0.2f, 0.5f, 0.2f);
	glProgramUniform3f(prgShaded, uni_specularColor, 0.5f, 0.5f, 0.5f);
	glProgramUniform3f(prgShaded, uni_lightColor,    1.0f, 1.0f, 1.0f);
	glProgramUniform3f(prgShaded, uni_wsLightPosition, LIGHT_POSITION);
	// TODO: put these values in a common location

	glProgramUniform1i(prgShaded, uni_diffuseTexture, 0);

	glDeleteShader(shdShadedVertex);
	glDeleteShader(shdShadedFragment);

	glUseProgram(prgShaded);
}

void moveCamera(float timePassed) {
	// Code from http://opengl-tutorial.org/beginners-tutorials/tutorial-6-keyboard-and-mouse/
	glm::vec3 direction = glm::vec3(
		cos(camera.rotation[0]) * sin(camera.rotation[1]),
		sin(camera.rotation[0]),
		cos(camera.rotation[0]) * cos(camera.rotation[1])
	);
	glm::vec3 right = glm::vec3(
		sin(camera.rotation[1] - PI/2.0f),
		0,
		cos(camera.rotation[1] - PI/2.0f)
	);
	glm::vec3 up = glm::cross(right, direction);
	glm::vec3 movement = direction * cameraSpeed * timePassed;
	camera.location += movement;

	glm::vec3 target = camera.location + direction;
	V = glm::lookAt(camera.location, target, up);
	P = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f);

	VP = P * V;
}

// Main loop methods //

void drawObject(DisplayObject* obj) {
	glBindVertexArray(obj->vao);

	// Set the MVP
	glm::mat4 MVP = VP * obj->modelMatrix;

	GLuint uniMVP = glGetUniformLocation(prgShaded, "MVP"),
	       uniM   = glGetUniformLocation(prgShaded, "M"),
	       uniV   = glGetUniformLocation(prgShaded, "V"),
	       uniP   = glGetUniformLocation(prgShaded, "P");
	glUniformMatrix4fv(uniMVP, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(uniM,   1, GL_FALSE, &(obj->modelMatrix[0][0]));
	glUniformMatrix4fv(uniV,   1, GL_FALSE, &V[0][0]);
	glUniformMatrix4fv(uniP,   1, GL_FALSE, &P[0][0]);

	// Set the texture
	glBindTexture(GL_TEXTURE_2D, obj->tex);

	glDrawElements(GL_TRIANGLES, obj->numIndices, GL_UNSIGNED_INT, NULL);
}

static bool nPressed = false, dPressed = false;

bool processInput(float timePassed) {
	bool n = glfwGetKey(static_cast<int>('N'));
	if (n && !nPressed) {
		showNormals = !showNormals;
	}
	nPressed = n;

	bool d = glfwGetKey(static_cast<int>('D'));
	if (d && !dPressed) {
		printf("Camera position: (%f, %f, %f)\n", camera.location[0], camera.location[1], camera.location[2]);
		printf("Camera angle: %f horizontal, %f vertical\n", camera.rotation[1], camera.rotation[0]);
	}
	dPressed = d;

	bool t = glfwGetKey(static_cast<int>('T'));
	if (t && !isTourRunning()) {
		printf("Starting the tour.\n");
		cameraSpeed = 0;
		startTour();
	}

	// Camera movement (adapted from http://opengl-tutorial.org/beginners-tutorials/tutorial-6-keyboard-and-mouse/)
	// Speed
	if (glfwGetKey(GLFW_KEY_UP) == GLFW_PRESS) {
		cameraSpeed += CAMERA_ACCELERATION * timePassed;
	}
	if (glfwGetKey(GLFW_KEY_DOWN) == GLFW_PRESS) {
		cameraSpeed -= CAMERA_ACCELERATION * timePassed;
		if (cameraSpeed < 0) cameraSpeed = 0;
	}

	// Yaw
	if (glfwGetKey(GLFW_KEY_LEFT) == GLFW_PRESS) {
		camera.rotation[1] += CAMERA_ROTATION_SPEED * timePassed;
	}
	if (glfwGetKey(GLFW_KEY_RIGHT) == GLFW_PRESS) {
		camera.rotation[1] -= CAMERA_ROTATION_SPEED * timePassed;
	}

	// Pitch
	if (glfwGetKey(GLFW_KEY_HOME) == GLFW_PRESS) {
		camera.rotation[0] += CAMERA_ROTATION_SPEED * timePassed;
	}
	if (glfwGetKey(GLFW_KEY_END) == GLFW_PRESS) {
		camera.rotation[0] -= CAMERA_ROTATION_SPEED * timePassed;
	}

	return (glfwGetKey(GLFW_KEY_ESC) || glfwGetKey(static_cast<int>('Q')));
}

int main(void) {
	if (!glfwInit()) {
		fprintf(stderr, "Could not initialise GLFW. Terminating.\n");
		exit(EXIT_FAILURE);
	}

	// Set up
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);

	if (!glfwOpenWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, 0, 0, 0, 0, GLFW_WINDOW)) {
		glfwTerminate();
		fprintf(stderr, "Could not open window. Terminating.\n");
		exit(EXIT_FAILURE);
	}

	glewExperimental = GL_TRUE;
	glewInit();
	glGetError();

	glfwSetWindowTitle(WINDOW_TITLE);

	setupShaders();
	checkForError("After shader setup");

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	setupScene(objects, camera);
	moveCamera(0);
	checkForError("After scene setup");

	// Main loop
	printf("Entering main loop.\n");
	double lastTime = glfwGetTime();
	bool shouldExit = false;
	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (unsigned int i = 0; i < objects.size(); i++) {
			drawObject(objects[i]);
		}

		/*if (showNormals) {
			glUseProgram(prgNormals);
			GLuint uniMVP = glGetUniformLocation(prgNormals, "MVP");
			for (unsigned int i = 0; i < objects.size(); i++) {
				glUniformMatrix4fv(uniMVP, 1, GL_FALSE, &(objects[i]->mvpSet.mvp[0][0]));
				glDrawArrays(GL_POINTS, 0, objects[i]->numVertices);
			}
			glUseProgram(prgShaded);
		}*/

		glfwSwapBuffers();
		checkForError("main loop");

		double currentTime = glfwGetTime();
		float timePassed = float(currentTime - lastTime);
		animate(timePassed);

		shouldExit = processInput(timePassed);
		moveCamera(timePassed);
		/*if (rotating) {
			currentRotation += ROTATION_SPEED * timePassed;
			for (unsigned int i = 0; i < objects.size(); i++) {
				objects[i]->rotation = glm::vec3(0, currentRotation, 0);
				updateModelMatrix(*objects[i]);
			}
		}*/
		lastTime = currentTime;
	} while (!shouldExit && glfwGetWindowParam(GLFW_OPENED));
}
