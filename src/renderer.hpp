#include <cstdint>
#include <GLFW/glfw3.h>

#include "pickingBuffer.hpp"
#include "depthBuffer.hpp"
#include "uiManager.hpp"
#include "cubemap.hpp"
#include "camera.hpp"
class Renderer
{
public:
	Renderer(GLFWwindow* window);
	~Renderer();

	float deltaTime;

	void render();

private:
	float m_deltaTime, m_lastFrameTime;
	Camera m_camera;

	uint32_t m_fullFrameQuadVAO, m_fullFrameQuadVBO;
	uint32_t m_debugQuadVAO, m_debugQuadVBO;
	uint32_t m_mainFbo, m_mainRbo, m_screenTexture;
	uint32_t m_lightsSSBO;

	int m_nMipLevels;
	PickingBuffer m_pickingbuffer;
	DepthBuffer m_depthBuffer;
	UIManager m_uiManager;
	Cubemap m_cubemap;

	glm::mat4 m_view;
	glm::mat4 m_projection;

	void initScreenQuad();
	void initDebugQuad();
	void initMainFrameBufferAndRenderTarget();
	void checkFrameBufeerSize();
	void createLightsSSBO();
};

float fullFrameQuadVertices[] = { 
	// positions   // texCoords
	-1.0f,  1.0f,  0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	1.0f, -1.0f,  1.0f, 0.0f,

	-1.0f,  1.0f,  0.0f, 1.0f,
	1.0f, -1.0f,  1.0f, 0.0f,
	1.0f,  1.0f,  1.0f, 1.0f
};

float debugQuadVertices[] = { 
	// positions   // texCoords
	0.75f,  1.0f,  0.0f, 1.0f,	// right top
	0.75f, 0.75f,  0.0f, 0.0f,	//left bottom
	1.0f, 0.75f,  1.0f, 0.0f,	//right bottom

	0.75f,  1.0f,  0.0f, 1.0f,	// right top
	1.0f, 0.75f,  1.0f, 0.0f,	//right bottom
	1.0f,  1.0f,  1.0f, 1.0f	//left top
};