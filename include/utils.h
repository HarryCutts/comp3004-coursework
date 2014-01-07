#ifndef _UTILS_H
#define _UTILS_H

void checkForError(const char* where);

char* fileToBuffer(const char* path);

GLuint loadTGA(const char *imagePath);

#endif
