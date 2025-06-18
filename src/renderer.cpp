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

Renderer::Renderer(GLFWwindow* window) : m_camera(glm::vec3(-10.0f, 3.0f, 13.0f), glm::vec3(0.0f, 1.0f, 0.0f), -45.0f, 0.0f)
{
	m_view = glm::mat4(1.0f);
	m_projection = glm::mat4(1.0f);
	// import
	scene = new Scene("Main Scene");
	GLTFModel model(std::filesystem::absolute("..\\..\\res\\GltfModels\\mr_elephant_RENDERTEST.glb").string());
	scene->addChild(std::move(model.getModel()));

	m_deltaTime = 0;
	m_lastFrameTime = 0;
	m_shadowMap = new ShadowMap(2048, 2048);
	m_cubemap = new Cubemap(m_camera, std::filesystem::absolute("..\\..\\res\\skybox\\river_walk_1_2k.hdr").string());
	m_inputManager = new InputManager(window, m_camera);
	m_uiManager = new UIManager(window, m_camera, scene);
	m_pickingPass = new PickingPass();
	m_gBufferPass = new GBuffer();
	m_FXAAPass = new FXAAPass();
	initScreenQuad();
	createOrResizeFrameBufferAndRenderTarget();

	AppConfig::initShaders();

	createLightsSSBO();

	Light directionalLight;
	directionalLight.type = 1;
	directionalLight.intensity = 0.1f;
	directionalLight.position = glm::vec3(0, 300.0f, 0);
	directionalLight.direction = glm::vec3(0.0f, 0.0f, 0.0f);
	directionalLight.ambient = glm::vec3(0.05f);
	directionalLight.diffuse = glm::vec3(0.8f);
	directionalLight.specular = glm::vec3(1.0f);
	directionalLight.constant = 1.0f;
	directionalLight.linear = 0.09f;
	directionalLight.quadratic = 0.032f;

	addLight(directionalLight);
}

Renderer::~Renderer()
{
	glDeleteFramebuffers(1, &m_deferedFBO);
	glDeleteFramebuffers(1, &m_deferedRBO);
	glDeleteBuffers(1, &m_fullFrameQuadVBO);
	glDeleteVertexArrays(1, &m_fullFrameQuadVAO);
	glDeleteBuffers(1, &m_lightsSSBO);
	delete m_shadowMap;
	delete m_cubemap;
	delete m_gBufferPass;
	delete m_FXAAPass;
	delete m_inputManager;
	delete m_uiManager;
	delete m_pickingPass;
	glfwTerminate();
}

void Renderer::createLightsSSBO()
{
	glCreateBuffers(1, &m_lightsSSBO);
	glNamedBufferData(m_lightsSSBO, SceneManager::getLights().size() * sizeof(Light), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_lightsSSBO);
}

void Renderer::checkLightBuffer()
{
	GLint bufferSize = 0;
	glGetNamedBufferParameteriv(m_lightsSSBO, GL_BUFFER_SIZE, &bufferSize);
	std::cout << "Light buffer size: " << bufferSize << " bytes" << std::endl;
	std::cout << "Lights count: " << SceneManager::getLights().size() << std::endl;
	std::cout << "Light size: " << sizeof(Light) << std::endl;
}

void Renderer::addLight(Light& light)
{
	SceneManager::addLight(light);
	glNamedBufferData(m_lightsSSBO, SceneManager::getLights().size() * sizeof(Light), nullptr, GL_DYNAMIC_DRAW);
	updateLights();
	checkLightBuffer();
}

void Renderer::updateLights()
{
	glNamedBufferSubData(m_lightsSSBO, 0, SceneManager::getLights().size() * sizeof(Light),
		SceneManager::getLights().data());
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

void Renderer::createOrResizeFrameBufferAndRenderTarget()
{
	if (m_deferedFBO <= 0)
	{
		glDeleteFramebuffers(1, &m_deferedFBO);
		glDeleteTextures(1, &m_deferedScreenTexture);
	}

	glCreateFramebuffers(1, &m_deferedFBO);
	glCreateRenderbuffers(1, &m_deferedRBO);

	glNamedRenderbufferStorage(m_deferedRBO, GL_DEPTH24_STENCIL8, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT);
	glNamedFramebufferRenderbuffer(m_deferedFBO, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_deferedRBO);

	m_nMipLevels = (int)floor(log2(std::max(AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT))) + 1;

	glCreateTextures(GL_TEXTURE_2D, 1, &m_deferedScreenTexture);
	glTextureStorage2D(m_deferedScreenTexture, m_nMipLevels, GL_RGBA32F, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT);
	glTextureParameteri(m_deferedScreenTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(m_deferedScreenTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(m_deferedScreenTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_deferedScreenTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glNamedFramebufferTexture(m_deferedFBO, GL_COLOR_ATTACHMENT0, m_deferedScreenTexture, 0);

}

void Renderer::checkFrameBufeerSize()
{
	if (!AppConfig::isFramebufferSizeSetted || !m_uiManager->viewportSizeSetteled)
	{
		AppConfig::RENDER_WIDTH = (int)m_uiManager->getViewportSize().x;
		AppConfig::RENDER_HEIGHT = (int)m_uiManager->getViewportSize().y;
		createOrResizeFrameBufferAndRenderTarget();
		m_gBufferPass->createOrResize();
		m_pickingPass->createOrResize();
		m_cubemap->createOrResize();
		m_FXAAPass->createOrResize();
		glViewport(0, 0, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT);

		AppConfig::isFramebufferSizeSetted = true;
	}
}


void Renderer::deferedPass()
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Defered Pass");
	glClearColor(0, 0, 0, 1);
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT);
	glPolygonMode(GL_FRONT_AND_BACK, AppConfig::polygonMode);
	glActiveTexture(GL_TEXTURE0);

	glBindFramebuffer(GL_FRAMEBUFFER, m_deferedFBO);
	AppConfig::deferedShader->use();
	glBindVertexArray(m_fullFrameQuadVAO);
	glBindTextureUnit(0, m_gBufferPass->tAlbedo);
	glBindTextureUnit(1, m_gBufferPass->tMetallic);
	glBindTextureUnit(2, m_gBufferPass->tRoughness);
	glBindTextureUnit(3, m_gBufferPass->tNormal);
	glBindTextureUnit(4, m_gBufferPass->tPosition);
	glBindTextureUnit(5, m_gBufferPass->tDepth);

	glBindTextureUnit(6, m_cubemap->irradianceMap);
	glBindTextureUnit(7, m_shadowMap->depthMap);
	glBindTextureUnit(8, m_cubemap->brdfLUTTexture);
	glBindTextureUnit(9, m_cubemap->specularMap);
	glBindTextureUnit(10, m_cubemap->envCubemap);

	glBindTextureUnit(11, m_pickingPass->pickingTexture);

	GLuint pickingSSBO;
	glGenBuffers(1, &pickingSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, pickingSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, SceneManager::getSelectedPrimitives().size() * sizeof(uint32_t),
		SceneManager::getSelectedPrimitives().data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, pickingSSBO); // 12 matches binding in shader

	if (SceneManager::getSelectedPrimitives().size() > 0)
	{
		AppConfig::deferedShader->setIntArray(
			"selectedPrimitives", static_cast<uint32_t>(SceneManager::getSelectedPrimitives().size()),
			reinterpret_cast<const int32_t*>(SceneManager::getSelectedPrimitives().data()));
	}

	AppConfig::deferedShader->setVec3("viewPos", m_camera.position);
	AppConfig::deferedShader->setFloat("irradianceMapRotationY", AppConfig::irradianceMapRotationY);
	AppConfig::deferedShader->setFloat("irradianceMapIntensity", AppConfig::irradianceMapIntensity);


	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTextureUnit(0, 0);
	glEnable(GL_DEPTH_TEST);
	glPopDebugGroup();

}

void Renderer::render(GLFWwindow* window)
{
	while (!glfwWindowShouldClose(window))
	{

		const float time = (float)glfwGetTime();
		m_deltaTime = time - m_lastFrameTime;
		m_lastFrameTime = time;

		checkFrameBufeerSize();

		if (AppConfig::RENDER_WIDTH != 0 && AppConfig::RENDER_HEIGHT != 0)
		{
			m_projection = glm::perspective(glm::radians(m_camera.zoom),
				float(AppConfig::RENDER_WIDTH) / float(AppConfig::RENDER_HEIGHT), 0.1f, 100.0f);
			m_view = m_camera.getViewMatrix();
		}

		if (AppConfig::reloadCubeMap)
		{
			m_cubemap = new Cubemap(m_camera, AppConfig::cubeMapPath);
			AppConfig::reloadCubeMap = false;
		}

		m_cubemap->draw(m_projection);
		m_pickingPass->draw(m_projection, m_view);
		m_shadowMap->draw(m_camera);
		m_gBufferPass->draw(m_projection, m_view);
		m_FXAAPass->draw(m_deferedScreenTexture);
		deferedPass();


		glfwPollEvents();
		ViewportState viewportState = m_uiManager->getViewportState();
		m_uiManager->setScreenTexture(m_FXAAPass->m_fxaaTextrue);
		m_uiManager->setShadowMapTexture(m_shadowMap->depthMap);
		m_uiManager->setPickingTexture(m_pickingPass->pickingTexture);
		m_uiManager->setGBuffer(m_gBufferPass);
		m_uiManager->draw(m_deltaTime);

		m_inputManager->processInput(m_deltaTime, viewportState, m_pickingPass->pickingTexture);
		glfwSwapBuffers(window);
	}
}
