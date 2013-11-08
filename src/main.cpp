#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/glfw.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "utils.h"

#define PI 3.14159265

#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 480
#define WINDOW_TITLE  "Graphics coursework 1, Harry Cutts"

static const GLfloat vertices[] = {
	-1.0f, -1.0f, 0.0f,
	 1.0f, -1.0f, 0.0f,
	 0.0f,  1.0f, 0.0f,
};

static GLuint shaderProgram;
static glm::mat4 MVP;

void setupShaders(void) {
	// TODO: error checking
	GLuint vertexShader   = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	char* vertexShaderSource   = fileToBuffer("shaders/vertex.glsl");
	char* fragmentShaderSource = fileToBuffer("shaders/fragment.glsl");

	glShaderSource(vertexShader, 1, (const GLchar**)&vertexShaderSource, NULL);
	glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentShaderSource, NULL);

	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);

	// Create the program
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	// Bind attributes to variables
	glBindAttribLocation(shaderProgram, 0, "vertexPosition_modelspace");

	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void setupGeometry(void) {
	// Create a VAO
	GLuint vertexArray;
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

	// Create a VBO
	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Set attributes
	glEnableVertexAttribArray(0);
	//glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

void setupMVP(void) {
	glm::mat4 projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	glm::mat4 view       = glm::lookAt(glm::vec3(3,5,3), glm::vec3(0,0,0), glm::vec3(0,1,0));
	glm::mat4 model      = glm::mat4(1.0f);
	glm::mat4 MVP        = projection * view * model;

	GLuint matrixID = glGetUniformLocation(shaderProgram, "MVP");
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

	glfwSetWindowTitle(WINDOW_TITLE);

	setupShaders();
	setupGeometry();
	setupMVP();

	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Main loop
	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers();
	} while (glfwGetKey(GLFW_KEY_ESC) != GLFW_PRESS && glfwGetWindowParam(GLFW_OPENED));
}
