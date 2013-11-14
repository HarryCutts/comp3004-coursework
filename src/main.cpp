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

#include "utils.h"
#include "generators.h"

#define PI 3.14159265

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600
#define WINDOW_TITLE  "Graphics coursework 1, Harry Cutts"

#define NUM_SPHERE_ITERATIONS 3

#define ROTATION_SPEED -8

struct DisplayObject {
	int numVertices;
	int numIndices;
	GLuint vao;
	GLfloat *mvp;
};

static GLuint prgDefault, prgNormals, prgShaded;
static GLuint prgCurrent;

static glm::mat4 MVP, icosahedronMVP, lowSphereMVP, highSphereMVP, coneForEMVP;

static DisplayObject sphere, cone, icosahedron, lowSphere, highSphere, coneForE;
static std::vector<DisplayObject*> objects;

static bool showNormals = false;

static bool rotating = false;
static GLfloat currentRotation = 0.0f;

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
	glBindAttribLocation(prgProgram, 0, "vertexPosition");
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
	// Standard program
	GLuint shdVertex   = createShader(GL_VERTEX_SHADER,   "shaders/vertex.glsl");
	GLuint shdFragment = createShader(GL_FRAGMENT_SHADER, "shaders/fragment.glsl");

	prgDefault = createProgram(shdVertex, 0, shdFragment);

	glDeleteShader(shdVertex);
	glDeleteShader(shdFragment);

	// Normals program
	GLuint shdNormalVertex   = createShader(GL_VERTEX_SHADER,   "shaders/normals/vertex.glsl");
	GLuint shdNormalGeometry = createShader(GL_GEOMETRY_SHADER, "shaders/normals/geometry.glsl");
	GLuint shdNormalFragment = createShader(GL_FRAGMENT_SHADER, "shaders/normals/fragment.glsl");

	prgNormals = createProgram(shdNormalVertex, shdNormalGeometry, shdNormalFragment);
	glUseProgram(prgNormals);
	GLuint uniMVP = glGetUniformLocation(prgNormals, "MVP");
	glUniformMatrix4fv(uniMVP, 1, GL_FALSE, &MVP[0][0]);

	glDeleteShader(shdNormalGeometry);
	glDeleteShader(shdNormalVertex);
	glDeleteShader(shdNormalFragment);

	// Shaded program
	GLuint shdShadedVertex   = createShader(GL_VERTEX_SHADER,   "shaders/shaded/vertex.glsl");
	GLuint shdShadedFragment = createShader(GL_FRAGMENT_SHADER, "shaders/shaded/fragment.glsl");

	prgShaded = createProgram(shdShadedVertex, 0, shdShadedFragment);

	GLuint uniMaterialColor = glGetUniformLocation(prgShaded, "materialColor");
	//GLuint uniAmbientColor  = glGetUniformLocation(prgShaded, "ambientColor");
	GLuint uniLightColor    = glGetUniformLocation(prgShaded, "lightColor");
	GLuint uniLightVector   = glGetUniformLocation(prgShaded, "lightVector");
	glProgramUniform3f(prgShaded, uniMaterialColor, 0.2f, 0.5f, 0.2f);
	//glProgramUniform3f(prgShaded, uniAmbientColor,  0.7f, 0.7f, 0.7f);
	glProgramUniform3f(prgShaded, uniLightColor,    1.0f, 1.0f, 1.0f);
	glProgramUniform3f(prgShaded, uniLightVector,   3.0f, 3.0f, 3.0f);
	// TODO: put these values in a common location

	glDeleteShader(shdShadedVertex);
	glDeleteShader(shdShadedFragment);
}

DisplayObject createDisplayObject(const Mesh &mesh, GLfloat *mvp) {
	// Create a VAO
	GLuint vaoVAO;
	glGenVertexArrays(1, &vaoVAO);
	glBindVertexArray(vaoVAO);

	// Vertex VBO
	GLuint vboVertex;
	glGenBuffers(1, &vboVertex);
	glBindBuffer(GL_ARRAY_BUFFER, vboVertex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.vertices.size(), mesh.vertices.data(), GL_STATIC_DRAW);

	// Set attributes
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Indices VBO
	GLuint vboIndices;
	glGenBuffers(1, &vboIndices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh.indices.size(), mesh.indices.data(), GL_STATIC_DRAW);

	// DisplayObject
	DisplayObject obj;
	obj.vao = vaoVAO;
	obj.numVertices = mesh.vertices.size();
	obj.numIndices  = mesh.indices.size();
	obj.mvp = mvp;
	return obj;
}

glm::mat4 createMVP(GLfloat x, GLfloat y, GLfloat z, GLfloat theta) {
	glm::mat4 projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	glm::mat4 view       = glm::lookAt(glm::vec3(3,3,3), glm::vec3(0,0,0), glm::vec3(0,1,0));
	glm::mat4 rotateY    = glm::rotate(glm::mat4(1.), theta, glm::vec3(0,1,0));
	glm::mat4 translate  = glm::translate(glm::mat4(1.), glm::vec3(x, y, z));
	GLfloat zRotation = -theta * 1.3;
	glm::mat4 rotateZ = glm::rotate(glm::mat4(1.), zRotation, glm::vec3(0,0,1));
	glm::mat4 model = rotateY * translate * rotateZ;
	return projection * view * model;
}

void setupMVPs(GLfloat rotation) {
	MVP = createMVP(0.0f, 0.0f, 0.0f, rotation);
	icosahedronMVP = createMVP(-1.3f, 0.0f,  1.3f, rotation);
	lowSphereMVP   = createMVP( 1.3f, 0.0f,  1.3f, rotation);
	highSphereMVP  = createMVP( 1.3f, 0.0f, -1.3f, rotation);
	coneForEMVP    = createMVP(-1.3f, 0.0f, -1.3f, rotation);
}

void setupGeometry(void) {
	setupMVPs(currentRotation);

	Mesh sphereMesh = generateSphere(NUM_SPHERE_ITERATIONS);
	Mesh coneMesh   = generateCone();
	sphere      = createDisplayObject(sphereMesh, &MVP[0][0]);
	cone        = createDisplayObject(coneMesh, &MVP[0][0]);

	icosahedron = createDisplayObject(generateIcosahedron(), &icosahedronMVP[0][0]);
	lowSphere   = createDisplayObject(generateSphere(1), &lowSphereMVP[0][0]);
	highSphere  = createDisplayObject(sphereMesh, &highSphereMVP[0][0]);
	coneForE    = createDisplayObject(coneMesh, &coneForEMVP[0][0]);
}

// Scenes //

void setDisplayObject(DisplayObject* obj) {
	objects.clear();
	objects.push_back(obj);
}

void drawObject(DisplayObject* obj) {
	glBindVertexArray(obj->vao);
	GLuint uniMVP = glGetUniformLocation(prgCurrent, "MVP");
	glUniformMatrix4fv(uniMVP, 1, GL_FALSE, obj->mvp);
	glDrawElements(GL_TRIANGLES, obj->numIndices, GL_UNSIGNED_INT, NULL);
}

void sceneA(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	setDisplayObject(&sphere);
	showNormals = false;
	rotating = false;
	currentRotation = 0;
	prgCurrent = prgDefault;
}

void sceneB(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	setDisplayObject(&cone);
	showNormals = false;
	rotating = false;
	currentRotation = 0;
	prgCurrent = prgDefault;
}

void sceneC(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	setDisplayObject(&sphere);
	showNormals = true;
	rotating = false;
	currentRotation = 0;
	prgCurrent = prgDefault;
}

void sceneD(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	setDisplayObject(&sphere);
	showNormals = false;
	rotating = false;
	currentRotation = 0;
	prgCurrent = prgShaded;
}

void sceneE(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	objects.clear();
	objects.push_back(&icosahedron);
	objects.push_back(&lowSphere);
	objects.push_back(&highSphere);
	objects.push_back(&coneForE);
	showNormals = false;
	rotating = true;
	prgCurrent = prgDefault;
}

void processInput(void) {
	if (glfwGetKey(static_cast<int>('A'))) {         // Wire-frame sphere
		sceneA();
	} else if (glfwGetKey(static_cast<int>('B'))) {  // Wire-frame cone
		sceneB();
	} else if (glfwGetKey(static_cast<int>('C'))) {  // Wire-frame sphere with normals
		sceneC();
	} else if (glfwGetKey(static_cast<int>('D'))) {  // Shaded sphere
		sceneD();
	} else if (glfwGetKey(static_cast<int>('E'))) {  // Animation
		sceneE();
	} else if (glfwGetKey(static_cast<int>('F'))) {  // Textured object
	} else if (glfwGetKey(static_cast<int>('R'))) {  // Toggle rotation
		rotating = !rotating;
	}
}


int main(void) {
	if (!glfwInit()) exit(EXIT_FAILURE);

	// Set up
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);

	if (!glfwOpenWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, 0, 0, 0, 0, GLFW_WINDOW)) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glewExperimental = GL_TRUE;
	glewInit();
	glGetError();

	glfwSetWindowTitle(WINDOW_TITLE);

	setupGeometry();
	checkForError("After geometry setup");
	setupShaders();
	checkForError("After shader setup");

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	sceneA();

	// Main loop
	printf("Entering main loop.\n");
	double lastTime = glfwGetTime();
	do {
		glUseProgram(prgCurrent);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (unsigned int i = 0; i < objects.size(); i++) {
			drawObject(objects[i]);
		}

		if (showNormals) {
			glUseProgram(prgNormals);
			glDrawArrays(GL_POINTS, 0, objects[0]->numVertices);
		}

		glfwSwapBuffers();
		checkForError("main loop");

		processInput();

		if (rotating) {
			double currentTime = glfwGetTime();
			double timePassed = currentTime - lastTime;
			currentRotation += ROTATION_SPEED * timePassed;
			setupMVPs(currentRotation);
			lastTime = currentTime;
		}
	} while (glfwGetKey(GLFW_KEY_ESC) != GLFW_PRESS && glfwGetWindowParam(GLFW_OPENED));
}
