#ifndef _ANIMATION_H
#define _ANIMATION_H

#include "generators.h"

struct DisplayObject {
    int numVertices;
    int numIndices;
    GLuint vao;

    GLuint tex;

    glm::vec3 location;
    glm::vec3 rotation;
    GLfloat scale;

    glm::mat4 modelMatrix;
};

void updateModelMatrix(DisplayObject &object);

void setupScene(std::vector<DisplayObject*> &objects);

void startTour(void);
bool isTourRunning(void);

void animate(float timePassed);

#endif
