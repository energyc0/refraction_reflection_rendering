#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#define CAMERA_UP glm::vec3(0.f,1.f,0.f)

class CameraBase {
protected:
	glm::vec3 pos;
	glm::vec3 forwardVector;
	const float sensetivity;
	double xCursorPos;
	double yCursorPos;
public:
	CameraBase(glm::vec3 eye, glm::vec3 center, float sense) :
		pos(eye),
		forwardVector(glm::normalize(center - eye)),
		sensetivity(sense),
		xCursorPos(0.0), yCursorPos(0.0){}
	inline glm::mat4 getCameraView() const { return glm::lookAt(pos, pos + forwardVector, glm::vec3(0.f, 1.f, 0.f)); };
	inline glm::vec3 getPos() const { return pos; }
	inline void setCursorPos(double x, double y) { xCursorPos = x; yCursorPos = y; };
	virtual void rotateCamera(double xPos, double yPos, int windowWidth, int windowHeight) = 0;
	virtual void cameraProcess(float deltaTime) = 0;
};