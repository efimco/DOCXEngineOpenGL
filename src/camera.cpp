#include "camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.f;
const float SPEED = 1.0f;

Camera::Camera(glm::vec3 pos, glm::vec3 upv, float yaw, float pitch)
	: front(glm::vec3(0.0f, 0.0f, -1.0f)), speed(SPEED), zoom(ZOOM), sensitivity(SENSITIVITY), position(pos), worldUp(upv),
	cameraReseted(true), defaultSpeed(speed)
{

	this->yaw = yaw;
	this->pitch = pitch;
	increasedSpeed = defaultSpeed * 3;
	orbitPivot = glm::vec3(0.0f); // point to orbit around
	distanceToOrbitPivot = glm::length(position - orbitPivot);
	updateCameraVecotrs();
}

glm::mat4 Camera::getViewMatrix() { return glm::lookAt(position, position + front, up); }

void Camera::processZoom(float yOffset)
{
	// Blender-style zoom: more responsive and distance-independent
	float zoomSpeed = 0.1f;
	float zoomFactor = 1.0f + (yOffset * zoomSpeed);
	
	// Clamp to prevent getting too close or too far
	float newDistance = glm::clamp(distanceToOrbitPivot / zoomFactor, 0.01f, 1000.0f);
	distanceToOrbitPivot = newDistance;
	updateCameraVecotrs();
}

void Camera::processPanning(float xOffset, float yOffset, glm::vec2 winSize)
{
	// Blender-style panning: consistent speed regardless of distance
	float panSpeed = distanceToOrbitPivot * 0.002f; // More consistent scaling
	glm::vec3 rightMove = -right * xOffset * panSpeed;
	glm::vec3 upMove = up * -yOffset * panSpeed;
	orbitPivot += rightMove + upMove;
	updateCameraVecotrs();
}

void Camera::updateCameraVecotrs()
{
	glm::vec3 offset;
	offset.x = distanceToOrbitPivot * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	offset.y = distanceToOrbitPivot * -sin(glm::radians(pitch));
	offset.z = distanceToOrbitPivot * cos(glm::radians(pitch)) * sin(glm::radians(yaw));

	position = orbitPivot + offset;
	front = glm::normalize(orbitPivot - position);
	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));
}

void Camera::focusOn(Primitive* primitive)
{
	glm::vec3 minPos = primitive->boundingBox.first + glm::vec3(primitive->transform.position);
	glm::vec3 maxPos = primitive->boundingBox.second + glm::vec3(primitive->transform.position);
	glm::vec3 center = (minPos + maxPos) * 0.5f;
	float radius = glm::length(maxPos - minPos) * 0.5f;
	orbitPivot = center;
	
	// Position camera to see the entire bounding box (Blender-style framing)
	float distance = radius * 2.5f; // More breathing room like Blender
	distanceToOrbitPivot = distance;

	updateCameraVecotrs();
}

void Camera::processOrbit(float deltaX, float deltaY)
{
	// Blender-style orbit: consistent angular speed
	float orbitSensitivity = 0.5f; // Fixed sensitivity like Blender
	yaw += deltaX * orbitSensitivity;
	pitch += deltaY * orbitSensitivity;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	updateCameraVecotrs();
}