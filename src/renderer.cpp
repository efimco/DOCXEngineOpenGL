#include "renderer.hpp"
#include "appConfig.hpp"
#include "gltfImporter.hpp"
#include "sceneManager.hpp"

Renderer::Renderer(GLFWwindow* window)
	:m_camera(glm::vec3(-10.0f, 3.0f, 13.0f), glm::vec3(0.0f,1.0f,0.0f), -45.0f, 0.0f), 
	m_uiManager(window, m_camera),
	m_depthBuffer(2048,2048)
{
	m_view = glm::mat4(1.0f);
	m_projection = glm::mat4(1.0f);

	m_deltaTime = 0;
	m_lastFrameTime = 0;

	m_cubemap = Cubemap();
	m_pickingbuffer = PickingBuffer();

	initScreenQuad();
	initDebugQuad();
	initMainFrameBufferAndRenderTarget();
	m_nMipLevels = (int)floor(log2(std::max(AppConfig::WINDOW_WIDTH, AppConfig::WINDOW_HEIGHT))) + 1;

	AppConfig::initShaders();
	//import
	GLTFModel model(std::filesystem::absolute("..\\..\\res\\GltfModels\\Knight.glb").string(), AppConfig::baseShader);

	createLightsSSBO();

}

void Renderer::createLightsSSBO()
{
	glCreateBuffers(1, &m_lightsSSBO);
	glNamedBufferData(m_lightsSSBO, SceneManager::getLights().size() * sizeof(Light), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_lightsSSBO);
}

void Renderer::initScreenQuad()
{
	glCreateVertexArrays(1, &m_fullFrameQuadVAO);
	glCreateBuffers(1, &m_fullFrameQuadVBO);
	glNamedBufferData(m_fullFrameQuadVBO, sizeof(m_fullFrameQuadVAO), &m_fullFrameQuadVAO, GL_STATIC_DRAW);
	glVertexArrayVertexBuffer(m_fullFrameQuadVAO, 0, m_fullFrameQuadVBO, 0, sizeof(float) * 4);

	glEnableVertexArrayAttrib(m_fullFrameQuadVAO, 0);
	glVertexArrayAttribFormat(m_fullFrameQuadVAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(m_fullFrameQuadVAO, 0, 0);

	glEnableVertexArrayAttrib(m_fullFrameQuadVAO, 1);
	glVertexArrayAttribFormat(m_fullFrameQuadVAO, 1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2);
	glVertexArrayAttribBinding(m_fullFrameQuadVAO, 1, 0);

}

void Renderer::initDebugQuad()
{
	glCreateVertexArrays(1, &m_debugQuadVAO);
	glCreateBuffers(1, &m_debugQuadVBO);
	glNamedBufferData(m_debugQuadVBO, sizeof(debugQuadVertices), &debugQuadVertices, GL_STATIC_DRAW);
	glVertexArrayVertexBuffer(m_debugQuadVAO, 0, m_debugQuadVBO, 0, sizeof(float) * 4);

	glEnableVertexArrayAttrib(m_debugQuadVAO, 0);
	glVertexArrayAttribFormat(m_debugQuadVAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(m_debugQuadVAO, 0, 0);

	glEnableVertexArrayAttrib(m_debugQuadVAO, 1);
	glVertexArrayAttribFormat(m_debugQuadVAO, 1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2);
	glVertexArrayAttribBinding(m_debugQuadVAO, 1, 0);
}

void Renderer::initMainFrameBufferAndRenderTarget()
{
	glCreateFramebuffers(1, &m_mainFbo);

	glCreateRenderbuffers(1, &m_mainRbo);
	glNamedRenderbufferStorage(m_mainRbo, GL_DEPTH24_STENCIL8, AppConfig::WINDOW_WIDTH, AppConfig::WINDOW_HEIGHT);
	glNamedFramebufferRenderbuffer(m_mainFbo, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_mainRbo);


	// create a color attachment texture
	glCreateTextures(GL_TEXTURE_2D, 1, &m_screenTexture);
	int nMipLevels = (int)floor(log2(std::max(AppConfig::WINDOW_WIDTH, AppConfig::WINDOW_HEIGHT))) + 1;
	glTextureStorage2D(m_screenTexture, nMipLevels, GL_RGBA32F, AppConfig::WINDOW_WIDTH, AppConfig::WINDOW_HEIGHT);
	glTextureParameteri(m_screenTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(m_screenTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(m_screenTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_screenTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glNamedFramebufferTexture(m_mainFbo, GL_COLOR_ATTACHMENT0, m_screenTexture, 0);
}

void Renderer::checkFrameBufeerSize()
{
	if (!AppConfig::isFramebufferSizeSetted)
	{
		glDeleteTextures(1, &m_screenTexture); 
		glDeleteRenderbuffers(1, &m_mainRbo);
		glDeleteFramebuffers(1, &m_mainFbo);

		glCreateFramebuffers(1, &m_mainFbo);

		glCreateRenderbuffers(1, &m_mainRbo);
		glNamedRenderbufferStorage(m_mainRbo, GL_DEPTH24_STENCIL8, AppConfig::WINDOW_WIDTH, AppConfig::WINDOW_HEIGHT);
		glNamedFramebufferRenderbuffer(m_mainFbo, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_mainRbo);
		
		// create a color attachment texture
		glCreateTextures(GL_TEXTURE_2D, 1, &m_screenTexture);
		m_nMipLevels = (int)floor(log2(std::max(AppConfig::WINDOW_WIDTH, AppConfig::WINDOW_HEIGHT))) + 1;
		glTextureStorage2D(m_screenTexture, m_nMipLevels, GL_RGBA32F, AppConfig::WINDOW_WIDTH, AppConfig::WINDOW_HEIGHT);
		glTextureParameteri(m_screenTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_screenTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glNamedFramebufferTexture(m_mainFbo, GL_COLOR_ATTACHMENT0, m_screenTexture, 0);

		m_pickingbuffer.resize();

		glViewport(0, 0, AppConfig::WINDOW_WIDTH, AppConfig::WINDOW_HEIGHT);

		AppConfig::isFramebufferSizeSetted = true;
		
	}
}

void Renderer::render()
{
}
