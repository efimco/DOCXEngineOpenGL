#pragma once
#include <GLFW/glfw3.h>
#include "ImGui/imgui.h"

#include "camera.hpp"
#include "viewportState.hpp"

class InputManager
{
public:
	InputManager(GLFWwindow* window, Camera& camera);
	~InputManager() = default;

	void processInput(float deltaTime, ViewportState viewportState);
	void cameraMovementCallback(GLFWwindow* window, float deltaTime);
	void cameraResetCallback(GLFWwindow* window, float deltaTime);

private:
	GLFWwindow* window;
	Camera& camera; 
	ImVec2 m_windowPos;

	float lastX;
	float lastY;
	bool firstMouse;
	double mousePosx;
	double mousePosy;

	void scrollCallback(ViewportState viewportState);
	void exitCallback();
	void wireframeToggleCallback();
};
