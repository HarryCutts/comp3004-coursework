#ifndef _ANIMATION_H
#define _ANIMATION_H

#include "generators.h"

#define CAMERA_START_POSITION glm::vec3(115, 30, 11.6)
#define CAMERA_START_YAW 23.1
#define CAMERA_START_PITCH -0.157394

#define SCREENSHOT_LOCATION CAMERA_START_POSITION
#define SCREENSHOT_YAW      CAMERA_START_YAW
#define SCREENSHOT_PITCH    CAMERA_START_PITCH

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

void setupScene(std::vector<DisplayObject*> &objects, DisplayObject &camera);

void startTour(void);
bool isTourRunning(void);

void animate(float timePassed);

#endif
