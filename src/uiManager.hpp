#pragma once
#include <glm/glm.hpp>
#include <string>
#include <GLFW/glfw3.h>
#include "camera.hpp"
#include "inputManager.hpp"
#include "ImGui/imgui.h"

class UIManager 
{
public:
	Camera& camera;
	GLFWwindow* window;
	InputManager* inputManager;
	bool viewportSizeSetteled;
	UIManager(GLFWwindow* window, Camera& camera);
	~UIManager();

	void draw(float deltaTime);
	void showCameraTransforms();
	void showLights();
	void showObjectInspector();
	void showTools();
	void showMaterialBrowser();
	bool wantCaptureInput() const;
	void showFramebufferViewport(float deltaTime);
	glm::vec2 getViewportSize();
	void setScreenTexture(uint32_t texId);

private:
	uint32_t m_screenTexture;
	bool m_viewportHovered;
	ImVec2 m_vpSize;
	std::string OpenFileDialog();
};