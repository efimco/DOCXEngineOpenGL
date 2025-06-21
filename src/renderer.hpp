#pragma once
#include <GLFW/glfw3.h>
#include <cstdint>

#include "camera.hpp"
#include "cubemap.hpp"
#include "light.hpp"
#include "pickingPass.hpp"
#include "shadowMap.hpp"
#include "uiManager.hpp"
#include "scene/scene.hpp"
#include "gBuffer.hpp"
#include "FXAAPass.hpp"

class Renderer
{
public:
	Renderer(GLFWwindow* window);
	~Renderer();

	float deltaTime;

	void render(GLFWwindow* window);
	Scene* scene;

private:
	float m_deltaTime, m_lastFrameTime;
	Camera m_camera;
	uint32_t m_fullFrameQuadVAO, m_fullFrameQuadVBO;
	uint32_t m_debugQuadVAO, m_debugQuadVBO;
	uint32_t m_deferedFBO, m_deferedRBO, m_deferedScreenTexture;

	int m_nMipLevels;
	ShadowMap* m_shadowMap;
	UIManager* m_uiManager;
	Cubemap* m_cubemap;
	InputManager* m_inputManager;
	PickingPass* m_pickingPass;
	GBuffer* m_gBufferPass;
	FXAAPass* m_FXAAPass;

	glm::mat4 m_view;
	glm::mat4 m_projection;

	void initScreenQuad();
	void createOrResizeFrameBufferAndRenderTarget();
	void checkFrameBufeerSize();
	void deferedPass();
};
