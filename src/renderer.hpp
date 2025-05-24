#pragma once
#include <cstdint>
#include <GLFW/glfw3.h>

#include "shadowMap.hpp"
#include "uiManager.hpp"
#include "cubemap.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "pickingPass.hpp"
class Renderer
{
public:
	Renderer(GLFWwindow* window);
	~Renderer();

	float deltaTime;

	void render(GLFWwindow* window);

private:
	float m_deltaTime, m_lastFrameTime;
	Camera m_camera;
	uint32_t m_fullFrameQuadVAO, m_fullFrameQuadVBO;
	uint32_t m_debugQuadVAO, m_debugQuadVBO;
	uint32_t m_mainFbo, m_mainRbo, m_screenTexture;
	uint32_t m_composedFbo, m_composedTexture;
	uint32_t m_lightsSSBO;

	int m_nMipLevels;
	ShadowMap* m_shadowMap;
	UIManager* m_uiManager;
	Cubemap* m_cubemap;
	InputManager* m_inputManager;
	PickingPass* m_pickingPass;

	glm::mat4 m_view;
	glm::mat4 m_projection;

	void initScreenQuad();
	void createOrResizeFrameBufferAndRenderTarget();
	void checkFrameBufeerSize();
	void mainPass();
	void composedPass();
	void createLightsSSBO();
	void checkLightBuffer();
	void addLight(Light& light);
	void updateLights();
};

