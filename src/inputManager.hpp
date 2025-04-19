#pragma once
#include <GLFW/glfw3.h>
#include "camera.hpp"

class InputManager
{
private:
	GLFWwindow* window;
	float lastX;
	float lastY;
	bool firstMouse;
	double mousePosx;
	double mousePosy;
	Camera& camera; 
	
	bool wireframeKeyPressed;
	bool wasMouse1Pressed;
	bool rightKeyPressed;

public:
	InputManager(GLFWwindow* window, Camera& camera);

	void processInput(PickingBuffer& pickingBuffer, float& deltaTime);
	void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
	void mouseCallback(GLFWwindow* window, double xPos, double yPos);
	void processExitCallback();
	void processWireframeToggleCallback();
	void processObjectPickingCallback(PickingBuffer& pickingBuffer);
	void processCameraMovementCallback(GLFWwindow* window, float deltaTime);
	void processCameraResetCallback(GLFWwindow* window, float deltaTime);
};