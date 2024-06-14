#include "FreeCamera.h"
#include <array>
#include <vector>

#define GLFW_KEY_W 87
#define GLFW_KEY_A 65
#define GLFW_KEY_S 83
#define GLFW_KEY_D 68
#define GLFW_KEY_SPACE 32
#define GLFW_KEY_LEFT_CONTROL 341

extern bool isMiddleButtonPressed;
extern std::array<bool, 1024> keys;

FreeCamera::FreeCamera(glm::vec3 eye, glm::vec3 center, float sense,float spd) :
	CameraBase(eye, center, sense),
	speed(spd),
	yaw(0.f),
	pitch((90.f * glm::dot(glm::normalize(forwardVector), CAMERA_UP))),
	rightVector(glm::cross(forwardVector, CAMERA_UP)){}
void FreeCamera::rotateCamera(double xPos, double yPos, int windowWidth, int windowHeight) {
	if (isMiddleButtonPressed) {
		float deltaX = static_cast<float>(xPos - xCursorPos) / static_cast<float>(windowHeight) * 360.f * sensetivity,
			deltaY = static_cast<float>(yPos - yCursorPos) / static_cast<float>(windowHeight) * 360.f * sensetivity;
		yaw += deltaX;
		float temp = pitch + deltaY;
		if (temp > 180.f) {
			temp = 179.9f;
			deltaY = 180.0f - pitch;
		}
		else if (temp < -180.f) {
			temp = -179.9f;
			deltaY = -180.0f - pitch;
		}
		pitch = temp;
		glm::quat cameraRotation = glm::angleAxis(glm::radians(deltaX), CAMERA_UP);
		cameraRotation *= glm::angleAxis(glm::radians(deltaY), glm::cross(forwardVector, CAMERA_UP));
		forwardVector = glm::normalize(glm::vec4(forwardVector, 0.0f) * glm::mat4_cast(cameraRotation));
		rightVector = glm::cross(forwardVector, CAMERA_UP);
	}
	xCursorPos = xPos;
	yCursorPos = yPos;
}
void FreeCamera::cameraProcess(float deltaTime) {
	glm::vec3 velocity(0.f);
	float v = speed * deltaTime;
	if (keys[GLFW_KEY_W])
		velocity += forwardVector * v;
	if (keys[GLFW_KEY_S])
		velocity -= forwardVector * v;
	if (keys[GLFW_KEY_D])
		velocity += rightVector * v;
	if (keys[GLFW_KEY_A])
		velocity -= rightVector * v;
	if (keys[GLFW_KEY_SPACE])
		velocity += CAMERA_UP * v;
	if (keys[GLFW_KEY_LEFT_CONTROL])
		velocity -= CAMERA_UP * v;
	pos += velocity;
}