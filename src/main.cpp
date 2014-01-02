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
#define WINDOW_TITLE  "Graphics coursework 3, Harry Cutts"

#define LIGHT_POSITION 5.f, 5.f, 3.f

#define ROTATION_SPEED -8

struct MVPSet {
	glm::mat4 mvp;
	glm::mat4 m;
	glm::mat4 v;
	glm::mat4 p;
};

struct DisplayObject {
	int numVertices;
	int numIndices;
	GLuint vao;
	MVPSet mvpSet;
	GLuint tex;
};

static GLuint prgNormals;
static GLuint prgShaded;

static DisplayObject object;
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

/** Loads and binds the texture stored in the TGA file at the given path.
 * @return the ID of the loaded texture (prefix `tex`). */
GLuint loadTGA(const char *imagePath) {
	// Method from http://opengl-tutorial.org/beginners-tutorials/tutorial-5-a-textured-cube/#How_to_load_texture_with_GLFW
	GLuint tex;
	glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glfwLoadTexture2D(imagePath, 0);

    // Nice trilinear filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    return tex;
}

// Setup methods //

void setupShaders(void) {
	// Normals program
	GLuint shdNormalVertex   = createShader(GL_VERTEX_SHADER,   "shaders/normals/vertex.glsl");
	GLuint shdNormalGeometry = createShader(GL_GEOMETRY_SHADER, "shaders/normals/geometry.glsl");
	GLuint shdNormalFragment = createShader(GL_FRAGMENT_SHADER, "shaders/normals/fragment.glsl");

	prgNormals = createProgram(shdNormalVertex, shdNormalGeometry, shdNormalFragment);

	glDeleteShader(shdNormalGeometry);
	glDeleteShader(shdNormalVertex);
	glDeleteShader(shdNormalFragment);

	// Shaded program
	GLuint shdShadedVertex   = createShader(GL_VERTEX_SHADER,   "shaders/shaded/vertex.glsl");
	GLuint shdShadedFragment = createShader(GL_FRAGMENT_SHADER, "shaders/shaded/fragment.glsl");

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

DisplayObject createDisplayObject(const Mesh &mesh, MVPSet mvpSet, const char *texturePath) {
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
	obj.mvpSet = mvpSet;
	obj.tex = loadTGA(texturePath);  // TODO: prevent textures being loaded twice
	return obj;
}

MVPSet createMVP(GLfloat x, GLfloat y, GLfloat z, GLfloat theta) {
	MVPSet set;

	glm::mat4 rotateY    = glm::rotate(glm::mat4(1.), theta, glm::vec3(0,1,0));
	glm::mat4 translate  = glm::translate(glm::mat4(1.), glm::vec3(x, y, z));
	GLfloat zRotation = -theta * 1.3;
	glm::mat4 rotateZ = glm::rotate(glm::mat4(1.), zRotation, glm::vec3(0,0,1));
	set.m = translate * rotateZ * rotateY;

	set.v = glm::lookAt(glm::vec3(5,5,5), glm::vec3(0,0,0), glm::vec3(0,1,0));
	set.p = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);

	set.mvp = set.p * set.v * set.m;
	return set;
}

void setupGeometry(void) {
	MVPSet MVP = createMVP(0.0f, 0.0f, 0.0f, currentRotation);

	Mesh objectMesh = loadOBJ("crate.obj");
	object      = createDisplayObject(objectMesh, MVP, "textures/crate.tga");
}

// Scenes //

void setDisplayObject(DisplayObject* obj) {
	objects.clear();
	objects.push_back(obj);
}

void drawObject(DisplayObject* obj) {
	glBindVertexArray(obj->vao);

	// Set the MVPs
	GLuint uniMVP = glGetUniformLocation(prgShaded, "MVP"),
	       uniM   = glGetUniformLocation(prgShaded, "M"),
	       uniV   = glGetUniformLocation(prgShaded, "V"),
	       uniP   = glGetUniformLocation(prgShaded, "P");
	glUniformMatrix4fv(uniMVP, 1, GL_FALSE, &(obj->mvpSet.mvp[0][0]));
	glUniformMatrix4fv(uniM,   1, GL_FALSE, &(obj->mvpSet.m[0][0]));
	glUniformMatrix4fv(uniV,   1, GL_FALSE, &(obj->mvpSet.v[0][0]));
	glUniformMatrix4fv(uniP,   1, GL_FALSE, &(obj->mvpSet.p[0][0]));

	// Set the texture
	glBindTexture(GL_TEXTURE_2D, obj->tex);

	glDrawElements(GL_TRIANGLES, obj->numIndices, GL_UNSIGNED_INT, NULL);
}

void scene() {
	setDisplayObject(&object);
}

bool processInput(void) {
	// TODO: distinguish between key presses
	if (glfwGetKey(static_cast<int>('R'))) {  // Toggle rotation
		rotating = !rotating;
	} else if (glfwGetKey(static_cast<int>('N'))) {
		showNormals = !showNormals;
	} else if (glfwGetKey(GLFW_KEY_ESC) || glfwGetKey(static_cast<int>('Q'))) {
		return true;
	}

	return false;
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

	setupGeometry();
	checkForError("After geometry setup");
	setupShaders();
	checkForError("After shader setup");

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	scene();

	// Main loop
	printf("Entering main loop.\n");
	double lastTime = glfwGetTime();
	bool shouldExit = false;
	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (unsigned int i = 0; i < objects.size(); i++) {
			drawObject(objects[i]);
		}

		if (showNormals) {
			glUseProgram(prgNormals);
			GLuint uniMVP = glGetUniformLocation(prgNormals, "MVP");
			for (unsigned int i = 0; i < objects.size(); i++) {
				glUniformMatrix4fv(uniMVP, 1, GL_FALSE, &(objects[i]->mvpSet.mvp[0][0]));
				glDrawArrays(GL_POINTS, 0, objects[i]->numVertices);
			}
			glUseProgram(prgShaded);
		}

		glfwSwapBuffers();
		checkForError("main loop");

		shouldExit = processInput();

		if (rotating) {
			double currentTime = glfwGetTime();
			double timePassed = currentTime - lastTime;
			currentRotation += ROTATION_SPEED * timePassed;
			object.mvpSet = createMVP(0.0f, 0.0f, 0.0f, currentRotation);
			lastTime = currentTime;
		}
	} while (!shouldExit && glfwGetWindowParam(GLFW_OPENED));
}
