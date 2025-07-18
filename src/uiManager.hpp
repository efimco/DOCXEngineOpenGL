#pragma once
#include "ImGui/imgui.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <memory>

#include "camera.hpp"
#include "inputManager.hpp"
#include "viewportState.hpp"
#include "scene/sceneNode.hpp"
#include "gBuffer.hpp"

enum class FileType
{
	IMAGE,
	MODEL,
	UNKNOWN
};

class UIManager
{
public:
	UIManager(GLFWwindow* window, Camera& camera, SceneNode* scene);
	~UIManager();

	Camera& camera;
	GLFWwindow* window;
	SceneNode* scene;

	bool viewportHovered;
	bool viewportSizeSetteled;

	ViewportState getViewportState();
	glm::vec2 getViewportSize();

	void setScreenTexture(uint32_t texId);
	void setPickingTexture(uint32_t texId);
	void setShadowMapTexture(uint32_t texId);
	void setGBuffer(GBuffer* gBuffer);
	void setFrameCounterPointer(int* frameCounter);
	void draw(float deltaTime);

private:
	AppConfig& m_appConfig;
	ViewportState m_viewportState;
	ImVec2 m_viewportPos;
	GBuffer* m_gBuffer;
	uint32_t m_screenTexture;
	uint32_t m_pickingTexture;
	uint32_t m_shadowMapTexture;
	ImVec2 m_vpSize;
	Tex* m_noTexture;
	int* m_frameCounter;

	std::string OpenFileDialog(FileType type);

	void showCameraTransforms();
	void showLights();
	void showObjectInspector();
	void showTools();
	void displaySceneNode(SceneNode* node) const;
	void showOutliner();
	void showMaterialBrowser();
	void getScroll();
	void getViewportPos();
	void getCursorPos();
	void showViewport(float deltaTime);
};