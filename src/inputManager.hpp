#pragma once
#include "ImGui/imgui.h"
#include "camera.hpp"
#include "viewportState.hpp"
#include "appConfig.hpp"
#include <GLFW/glfw3.h>


class InputManager
{
public:
	InputManager(GLFWwindow* window, Camera& camera);
	~InputManager() = default;

	void processInput(float deltaTime, ViewportState viewportState, uint32_t pickingTexture);
	void setFrameCounterPointer(int* frameCounter);

private:
	AppConfig& m_appConfig;
	GLFWwindow* window;
	Camera& camera;

	float lastX;
	float lastY;
	bool firstMouse;
	double mousePosx;
	double mousePosy;
	int* m_frameCounter;
	
	void scrollCallback(ViewportState viewportState);
	void cameraFocusCallback();
	void deleteObjectCallback();
	void pickObjectCallback(ViewportState viewportState, uint32_t pickingTexture);
	void exitCallback();
	void cameraMovementCallback(GLFWwindow* window, float deltaTime, ViewportState viewportState);
	void wireframeToggleCallback();
};
