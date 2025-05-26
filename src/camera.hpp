#pragma once
#include "glm/glm.hpp"
#include <vector>

enum CameraMovement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

class Camera
{
public:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;
	glm::vec3 orbitPivot;
	std::vector<glm::vec3> defaultCameraMatrix;
	float defaultCameraRotation[2];
	bool cameraReseted;

	float yaw;
	float pitch;
	float zoom;
	float speed;
	float defaultSpeed;
	float increasedSpeed;
	float sensitivity;
	float distanceToOrbitPivot;

	Camera(glm::vec3 pos, glm::vec3 upv, float yaw, float pitch);

	glm::mat4 getViewMatrix();
	void processMouseScroll(float yOffset);
	void processPanning(float xOffset, float yOffset);
	void processOrbit(float deltaX, float deltaY);

private:
	void updateCameraVecotrs();
};