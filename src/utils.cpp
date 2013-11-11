#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <GL/glew.h>
#include <GL/glfw.h>

#include "utils.h"

void checkForError(const char* where) {
	std::string msg;
	int err = glGetError();
	if (!err) return;

	switch (err) {
	case GL_INVALID_ENUM:                  msg = "GL_INVALID_ENUM"; break;
	case GL_INVALID_VALUE:                 msg = "GL_INVALID_VALUE"; break;
	case GL_INVALID_OPERATION:             msg = "GL_INVALID_OPERATION"; break;
	case GL_INVALID_FRAMEBUFFER_OPERATION: msg = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
	case GL_OUT_OF_MEMORY:                 msg = "GL_OUT_OF_MEMORY"; break;
	default:                               msg = "Unknown Error"; break;
	}
	fprintf(stderr, "%s: %s\n", where, msg.c_str());
}

char* fileToBuffer(const char* path) {
    printf("Loading %s...", path);
    FILE *file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Could not open file %s.\n", path);
        return NULL;
    }

    // Find the length of the file
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Read the file
    char* buffer = (char*)malloc(length + 1);  // Extra byte for NULL terminator
    fread(buffer, length, 1, file);
    fclose(file);
    buffer[length] = 0;
    printf(" done.\n");
    return buffer;
}

