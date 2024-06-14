#pragma once
#include "CameraBase.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class CentredCamera : public CameraBase{
    float& const scrollSpeed;
public:
    CentredCamera(glm::vec3 eye, glm::vec3 center, float sense, float scroll_speed);
    void cameraScroll(double yOffset);
    void rotateCamera(double xPos, double yPos, int windowWidth, int windowHeight);
    void cameraProcess(float deltaTime);
};