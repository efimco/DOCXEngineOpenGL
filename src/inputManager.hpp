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

	void processInput(float deltaTime, ViewportState viewportState, uint32_t pickingTexture);
	void cameraMovementCallback(GLFWwindow* window, float deltaTime, ViewportState viewportState);
	void cameraResetCallback(GLFWwindow* window, float deltaTime);

private:
	GLFWwindow* window;
	Camera& camera; 

	float lastX;
	float lastY;
	bool firstMouse;
	double mousePosx;
	double mousePosy;

	void scrollCallback(ViewportState viewportState);
	void pixelReadBack(ViewportState viewportState, uint32_t pickingTexture);
	void exitCallback();
	void wireframeToggleCallback();
};
