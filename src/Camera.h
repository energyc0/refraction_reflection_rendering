#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Camera {
    glm::vec3 pos;
    const float sensetivity;
public:
    double xCursorPos;
    double yCursorPos;
    bool isLeftButtonPressed;
    bool isRightButtonPressed;
    bool isMiddleButtonPressed;
    float scrollSpeed;
    glm::quat cameraRotation;
public:
    Camera();
    glm::mat4 getCameraView() const;
    glm::vec3 getPos() const ;
    void cameraScroll(double yOffset);
    void rotateCamera(const glm::vec2& deltaXY);
};