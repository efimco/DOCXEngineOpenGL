#pragma once
#include <GLFW/glfw3.h>
#include <cstdint>

#include "camera.hpp"
#include "cubemap.hpp"
#include "deferedPass.hpp"
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
	AppConfig& m_appConfig;
	float m_deltaTime, m_lastFrameTime;
	Camera m_camera;
	uint32_t m_fullFrameQuadVAO, m_fullFrameQuadVBO;

	int m_nMipLevels;

	//PASSES
	ShadowMap* m_shadowMap;
	UIManager* m_uiManager;
	Cubemap* m_cubemap;
	InputManager* m_inputManager;
	PickingPass* m_pickingPass;
	GBuffer* m_gBufferPass;
	FXAAPass* m_FXAAPass;
	DeferedPass* m_deferedPass;

	glm::mat4 m_view;
	glm::mat4 m_projection;

	void initScreenQuad();
	void createOrResizeFrameBufferAndRenderTarget();
	void checkFrameBuferSize();
	void deferedPass();
};
