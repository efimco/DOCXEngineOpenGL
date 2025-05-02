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
	UIManager(GLFWwindow* window, Camera& camera, InputManager* inputManager);
	~UIManager();

	void draw(float deltaTime);
	ImVec2 getWindowPos();
	void showCameraTransforms();
	void showLights();
	void showObjectInspector();
	void showTools();
	void showMaterialBrowser();
	bool wantCaptureInput() const;
	void showFramebufferViewport(float deltaTime);
	glm::vec2 getViewportSize();
	void setScreenTexture(uint32_t texId);
	void setPickingTexture(uint32_t texId);
	void setShadowMapTexture(uint32_t texId);
	bool viewportHovered;

private:
	ImVec2 m_viewportPos;
	uint32_t m_screenTexture;
	uint32_t m_pickingTexture;
	uint32_t m_shadowMapTexture;

	ImVec2 m_vpSize;
	std::string OpenFileDialog();
};