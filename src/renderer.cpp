#include <glad/gl.h>
#include <iostream>

#include "appConfig.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "gltfImporter.hpp"
#include "renderer.hpp"
#include "sceneManager.hpp"


static const float fullFrameQuadVertices[] = {
	// positions   // texCoords
	-1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,

	-1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f };

Renderer::Renderer(GLFWwindow* window) : m_camera(glm::vec3(-10.0f, 3.0f, 13.0f), glm::vec3(0.0f, 1.0f, 0.0f), -45.0f, 0.0f),
m_appConfig(AppConfig::get())
{
	m_view = glm::mat4(1.0f);
	m_projection = glm::mat4(1.0f);

	// import
	scene = new Scene("Main Scene");
	GLTFModel model(std::filesystem::absolute("..\\..\\res\\GltfModels\\Knight.glb").string());
	scene->addChild(std::move(model.getModel()));

	m_deltaTime = 0;
	m_lastFrameTime = 0;
	m_shadowMap = new ShadowMap(2048, 2048);
	m_cubemap = new Cubemap(m_camera, std::filesystem::absolute("..\\..\\res\\skybox\\studio_small_03_4k.hdr").string());
	m_inputManager = new InputManager(window, m_camera);
	m_uiManager = new UIManager(window, m_camera, scene);
	m_pickingPass = new PickingPass();
	m_gBufferPass = new GBuffer();
	m_deferedPass = new DeferedPass(m_camera);
	m_TAAPass = new TAAPass();
	m_postProcessPass = new PostProcessPass();
	initScreenQuad();

	m_appConfig.initShaders();
}

Renderer::~Renderer()
{
	glDeleteBuffers(1, &m_fullFrameQuadVBO);
	glDeleteVertexArrays(1, &m_fullFrameQuadVAO);
	delete m_shadowMap;
	delete m_cubemap;
	delete m_gBufferPass;
	delete m_inputManager;
	delete m_uiManager;
	delete m_pickingPass;
	delete m_deferedPass;
	glfwTerminate();
}


void Renderer::initScreenQuad()
{
	glCreateVertexArrays(1, &m_fullFrameQuadVAO);
	glCreateBuffers(1, &m_fullFrameQuadVBO);
	glNamedBufferData(m_fullFrameQuadVBO, sizeof(fullFrameQuadVertices), &fullFrameQuadVertices, GL_STATIC_DRAW);
	glVertexArrayVertexBuffer(m_fullFrameQuadVAO, 0, m_fullFrameQuadVBO, 0, sizeof(float) * 4);

	glEnableVertexArrayAttrib(m_fullFrameQuadVAO, 0);
	glVertexArrayAttribFormat(m_fullFrameQuadVAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(m_fullFrameQuadVAO, 0, 0);

	glEnableVertexArrayAttrib(m_fullFrameQuadVAO, 1);
	glVertexArrayAttribFormat(m_fullFrameQuadVAO, 1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2);
	glVertexArrayAttribBinding(m_fullFrameQuadVAO, 1, 0);
}

int jitterIndex = 0;
glm::vec2 jitter;
void Renderer::checkFrameBuferSize()
{
	if (!m_appConfig.isFramebufferSizeSet || !m_uiManager->viewportSizeSetteled)
	{
		jitterIndex = 0;
		m_appConfig.renderWidth = (int)m_uiManager->getViewportSize().x;
		m_appConfig.renderHeight = (int)m_uiManager->getViewportSize().y;
		m_appConfig.renderWidth += m_appConfig.renderWidth % 2;
		m_appConfig.renderHeight += m_appConfig.renderHeight % 2;
		m_nMipLevels = (int)floor(log2(std::max(m_appConfig.renderWidth, m_appConfig.renderHeight))) + 1;
		m_gBufferPass->createOrResize();
		m_pickingPass->createOrResize();
		m_cubemap->createOrResize();
		m_deferedPass->createOrResize(m_nMipLevels);
		m_TAAPass->createOrResize();
		m_postProcessPass->createOrResize();
		glViewport(0, 0, m_appConfig.renderWidth, m_appConfig.renderHeight);
		m_appConfig.isFramebufferSizeSet = true;
	}
}



float halton(int32_t index, int32_t base)
{
	float f = 1.0f, result = 0.0f;

	for (int32_t currentIndex = index; currentIndex > 0;) {

		f /= (float)base;
		result = result + f * (float)(currentIndex % base);
		currentIndex = (uint32_t)(floorf((float)(currentIndex) / (float)(base)));
	}

	return result;
}



void Renderer::render(GLFWwindow* window)
{
	while (!glfwWindowShouldClose(window))
	{
		const float time = (float)glfwGetTime();	
		m_deltaTime = time - m_lastFrameTime;
		m_lastFrameTime = time;

		checkFrameBuferSize();

		if (m_appConfig.renderWidth != 0 && m_appConfig.renderHeight != 0)
		{
			m_projection = glm::perspective(glm::radians(m_camera.zoom),
				float(m_appConfig.renderWidth) / float(m_appConfig.renderHeight),
				m_appConfig.nearPlane, m_appConfig.farPlane);
			m_view = m_camera.getViewMatrix();
		}

		if (m_appConfig.reloadCubeMap)
		{
			m_cubemap = new Cubemap(m_camera, m_appConfig.cubeMapPath);
			m_appConfig.reloadCubeMap = false;
		}
		const int accumulationLimit = 64;
		if (m_appConfig.isTAA)
		{
			const float basePhaseCount = float(accumulationLimit);
			int denominator = (m_appConfig.renderWidth == 0 ? 1 : m_appConfig.renderWidth);
			int32_t jitterPhaseCount = int32_t(basePhaseCount * pow((static_cast<double>(m_appConfig.windowWidth) / denominator), 2.0f));
			jitterPhaseCount = (jitterPhaseCount == 0.0f ? 1.0f : jitterPhaseCount);
			const float x = halton((jitterIndex % jitterPhaseCount) + 1, 2) - 0.5f;
			const float y = halton((jitterIndex % jitterPhaseCount) + 1, 3) - 0.5f;
			jitterIndex++;
			jitter.x = x / float(m_appConfig.renderWidth) * 2.0f;
			jitter.y = y / float(m_appConfig.renderHeight) * 2.0f;
			glm::mat4 jitteredTranslation = glm::translate(glm::mat4(1.0f), glm::vec3(jitter, 0.0f));
			m_projection = jitteredTranslation * m_projection;
		}
		else
		{
			// Reset jitter when TAA is disabled
			jitter = glm::vec2(0.0f, 0.0f);
			jitterIndex = 0;
		}


		m_cubemap->draw(m_projection);
		m_pickingPass->draw(m_projection, m_view);
		// m_shadowMap->draw(m_camera);
		m_gBufferPass->setJitter(jitter);
		m_gBufferPass->draw(m_projection, m_view, m_camera.distanceToOrbitPivot);
		m_deferedPass->draw(m_fullFrameQuadVAO, m_gBufferPass, m_cubemap, m_shadowMap, m_pickingPass);
		m_TAAPass->setCurrrentTexture(m_deferedPass->deferedTexture);
		m_TAAPass->setAccumulationLimit(accumulationLimit);
		m_TAAPass->setCurrentFrameNumber(jitterIndex);
		m_TAAPass->setVelocityTexture(m_gBufferPass->tVelocity);
		m_TAAPass->setDepthTexture(m_gBufferPass->tDepth);
		m_TAAPass->setJitterValues(m_gBufferPass->getCurrentJitter(), m_gBufferPass->getPreviousJitter());
		m_TAAPass->draw();
		m_postProcessPass->draw(m_TAAPass->getCurrentOutput(), m_cubemap->envCubemap, m_gBufferPass->tDepth);

		glfwPollEvents();
		ViewportState viewportState = m_uiManager->getViewportState();
		m_uiManager->setFrameCounterPointer(&jitterIndex);
		m_uiManager->setScreenTexture(m_postProcessPass->getPostProcessedTexture());
		m_uiManager->setShadowMapTexture(m_deferedPass->deferedTexture);
		m_uiManager->setPickingTexture(m_pickingPass->pickingTexture);
		m_uiManager->setGBuffer(m_gBufferPass);
		m_uiManager->draw(m_deltaTime);
		m_inputManager->setFrameCounterPointer(&jitterIndex);
		m_inputManager->processInput(m_deltaTime, viewportState, m_pickingPass->pickingTexture);

		glfwSwapBuffers(window);
	}
}
