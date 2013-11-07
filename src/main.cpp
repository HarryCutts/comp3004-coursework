#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/glfw.h>

#include "utils.h"

#define PI 3.14159265

#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 480
#define WINDOW_TITLE  "Graphics coursework 1, Harry Cutts"

int main(void) {
	if (!glfwInit()) exit(EXIT_FAILURE);

	// Set up
	//glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	//glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);

	if (!glfwOpenWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, 0, 0, 0, 0, GLFW_WINDOW)) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glewInit();

	glfwSetWindowTitle(WINDOW_TITLE);

	//setupShaders();
	//setupGeometry();

	// Main loop
	do {
		// TODO: do something

		glfwSwapBuffers();
	} while (glfwGetKey(GLFW_KEY_ESC) != GLFW_PRESS && glfwGetWindowParam(GLFW_OPENED));
}
