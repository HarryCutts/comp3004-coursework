#include <stdio.h>
#include <stdlib.h>
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

#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 480
#define WINDOW_TITLE  "Graphics coursework 1, Harry Cutts"

static GLuint shaderProgram, normalsProgram;
static glm::mat4 MVP;

static Mesh mesh;

// Setup methods //

GLuint createShader(GLenum type, const char* path) {
	GLuint shader = glCreateShader(type);
	char* source = fileToBuffer(path);
	glShaderSource(shader, 1, (const GLchar**)&source, NULL);
	glCompileShader(shader);

	// Check for errors
	GLint result;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		int length;
		char message[1000];
		glGetShaderInfoLog(shader, 1000, &length, reinterpret_cast<GLchar*>(&message));
		if (length > 0) {
			fprintf(stderr, "Error(s) in %s:\n%s\n--end of errors--\n", path, message);
		}
	}

	return shader;
}

GLuint createProgram(GLuint vertexShader, GLuint geometryShader, GLuint fragmentShader) {
	GLuint program = glCreateProgram();
	if (vertexShader)   glAttachShader(program, vertexShader);
	if (geometryShader) glAttachShader(program, geometryShader);
	if (fragmentShader) glAttachShader(program, fragmentShader);
	glBindAttribLocation(program, 0, "vertexPosition");
	glLinkProgram(program);

	// Check for errors
	GLint result;
	glGetProgramiv(program, GL_LINK_STATUS, &result);
	if (result == GL_FALSE) {
		int length;
		char message[1000];
		glGetProgramInfoLog(program, 1000, &length, reinterpret_cast<GLchar*>(&message));
		if (length > 0) {
			fprintf(stderr, "Error(s) in shader program:\n%s\n--end of errors--\n", message);
		}
	}

	return program;
}

void setupShaders(void) {
	// Standard program
	GLuint vertexShader   = createShader(GL_VERTEX_SHADER, "shaders/vertex.glsl");
	GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, "shaders/fragment.glsl");

	shaderProgram = createProgram(vertexShader, 0, fragmentShader);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// Normals program
	GLuint normalVertexShader   = createShader(GL_VERTEX_SHADER, "shaders/normals/vertex.glsl");
	GLuint geometryShader       = createShader(GL_GEOMETRY_SHADER, "shaders/normals/geometry.glsl");
	GLuint normalFragmentShader = createShader(GL_FRAGMENT_SHADER, "shaders/normals/fragment.glsl");

	normalsProgram = createProgram(normalVertexShader, geometryShader, normalFragmentShader);

	glDeleteShader(geometryShader);
	glDeleteShader(normalVertexShader);
	glDeleteShader(normalFragmentShader);
}

void setupGeometry(void) {
	// Create a VAO
	GLuint vertexArray;
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

	// Vertex VBO
	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	mesh = generateSphere(3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.vertices.size(), mesh.vertices.data(), GL_STATIC_DRAW);

	// Set attributes
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Indices VBO
	GLuint indicesBuffer;
	glGenBuffers(1, &indicesBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh.indices.size(), mesh.indices.data(), GL_STATIC_DRAW);
}

void setupMVP(void) {
	glm::mat4 projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	glm::mat4 view       = glm::lookAt(glm::vec3(3,3,3), glm::vec3(0,0,0), glm::vec3(0,1,0));
	glm::mat4 model      = glm::mat4(1.0f);
	glm::mat4 MVP        = projection * view * model;

	glUseProgram(shaderProgram);
	GLuint matrixID = glGetUniformLocation(shaderProgram, "MVP");
	glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);

	glUseProgram(normalsProgram);
	matrixID = glGetUniformLocation(normalsProgram, "MVP");
	glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
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

	setupShaders();
	checkForError("After shader setup");
	setupGeometry();
	checkForError("After geometry setup");
	setupMVP();
	checkForError("After MVP setup");

	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // Display wireframes

	// Main loop
	printf("Entering main loop.\n");
	do {
		glUseProgram(shaderProgram);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, NULL);

		glUseProgram(normalsProgram);
		glDrawArrays(GL_POINTS, 0, mesh.vertices.size());

		glfwSwapBuffers();
	} while (glfwGetKey(GLFW_KEY_ESC) != GLFW_PRESS && glfwGetWindowParam(GLFW_OPENED));
}
