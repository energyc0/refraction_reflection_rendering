#pragma once
#include "CameraBase.h"

class FreeCamera : public CameraBase{
	const float speed;
	float yaw;
	float pitch;
	glm::vec3 rightVector;
public:
	FreeCamera(glm::vec3 eye, glm::vec3 center, float sense, float speed);
	void rotateCamera(double xPos, double yPos, int windowWidth, int windowHeight);
	void cameraProcess(float deltaTime);
};