#include "CentredCamera.h"

extern bool isMiddleButtonPressed;

CentredCamera::CentredCamera(glm::vec3 eye, glm::vec3 center, float sense, float scroll_speed) :
    CameraBase(eye, center, sense),
    scrollSpeed(scroll_speed){}
void CentredCamera::cameraScroll(double yOffset) {
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
void CentredCamera::rotateCamera(double xPos, double yPos, int windowWidth, int windowHeight) {
    if (isMiddleButtonPressed) {
        float deltaX = static_cast<float>(xPos - xCursorPos) / static_cast<float>(windowHeight),
            deltaY = static_cast<float>(yPos - yCursorPos) / static_cast<float>(windowHeight);
        glm::quat cameraRotation =
            glm::angleAxis(glm::radians(deltaX * 360.f * sensetivity),
                CAMERA_UP);
        cameraRotation *=
            glm::angleAxis(glm::radians(deltaY * 360.f * sensetivity),
                glm::cross(glm::normalize(-pos), CAMERA_UP));
        pos = glm::vec4(pos, 1.0f) * glm::mat4_cast(cameraRotation);
    }
    xCursorPos = xPos;
    yCursorPos = yPos;
}
void CentredCamera::cameraProcess(float deltaTime){}