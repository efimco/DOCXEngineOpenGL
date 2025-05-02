#pragma once
#include <GLFW/glfw3.h>
#include "camera.hpp"
#include "pickingBuffer.hpp"
#include "ImGui/imgui.h"

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

	PickingBuffer* m_pickingBuffer;
	ImVec2 m_windowPos;

public:
	InputManager(GLFWwindow* window, Camera& camera);

	void processInput(float deltaTime, bool viewportHovered);
	void scrollCallback();
	void mouseCallback();
	void processExitCallback();
	void processWireframeToggleCallback();
	void processObjectPickingCallback();
	void setPickingBuffer(PickingBuffer* pickingBuffer);
	void setWindowPos(ImVec2 windowPos);
	void processCameraMovementCallback(GLFWwindow* window, float deltaTime);
	void processCameraResetCallback(GLFWwindow* window, float deltaTime);
};
