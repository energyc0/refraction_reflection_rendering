#include "Camera.h"

Camera::Camera() : sensetivity(1.0f){
    pos = glm::vec3(-350.5f, 0.0f, -350.5f);
    xCursorPos = 0.f;
    yCursorPos = 0.f;
    isLeftButtonPressed = false;
    isRightButtonPressed = false;
    isMiddleButtonPressed = false;
    scrollSpeed = 50.0f;
    cameraRotation = glm::quat(glm::vec3());
}
glm::mat4 Camera::getCameraView() const{
    return glm::lookAt(pos, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
}
void Camera::cameraScroll(double yOffset) {
    glm::vec3 velocity = glm::normalize(pos) * static_cast<float>(yOffset) * scrollSpeed;
    glm::vec3 temp = (pos + velocity) / pos;
    if (temp.x > 0 && temp.y > 0 && temp.z > 0) {
        pos = pos + velocity;
    }
    else {
        pos.x = temp.x > 0 ? pos.x : pos.x > 0 ? 0.1f : -0.1f;
        pos.y = temp.y > 0 ? pos.y : pos.y > 0 ? 0.1f : -0.1f;
        pos.z = temp.z > 0 ? pos.z : pos.z > 0 ? 0.1f : -0.1f;
    }
}
void Camera::rotateCamera(const glm::vec2& deltaXY) {
    cameraRotation =
        glm::angleAxis(glm::radians(deltaXY.x * 360.f * sensetivity),
            glm::vec3(0.0f, 1.0f, 0.0f));
    cameraRotation *=
        glm::angleAxis(glm::radians(deltaXY.y * 360.f),
            glm::cross(glm::normalize(-pos), glm::vec3(0.0f, 1.0f, 0.0f)));
    pos = glm::vec4(pos, 1.0f) * glm::mat4_cast(cameraRotation);
}
glm::vec3 Camera::getPos() const{
    return pos;
}