#include <iostream>
#include <glad/glad.h>
#include "glm/gtc/matrix_transform.hpp"
#include "appConfig.hpp"
#include "gltfImporter.hpp"
#include "sceneManager.hpp"
#include "renderer.hpp"


static const float fullFrameQuadVertices[] = { 
	// positions   // texCoords
	-1.0f,  1.0f,  0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	1.0f, -1.0f,  1.0f, 0.0f,

	-1.0f,  1.0f,  0.0f, 1.0f,
	1.0f, -1.0f,  1.0f, 0.0f,
	1.0f,  1.0f,  1.0f, 1.0f
};

Renderer::Renderer(GLFWwindow* window)
	:m_camera(glm::vec3(-10.0f, 3.0f, 13.0f), glm::vec3(0.0f,1.0f,0.0f), -45.0f, 0.0f)
{
	m_view = glm::mat4(1.0f);
	m_projection = glm::mat4(1.0f);

	m_deltaTime = 0;
	m_lastFrameTime = 0;
	m_shadowMap = new ShadowMap(2048, 2048);
	m_cubemap = new Cubemap(m_camera,std::filesystem::absolute("..\\..\\res\\skybox\\overcast_soil_puresky_4k.hdr").string());
	SceneManager::addShader(&m_cubemap->backgroundShader);
	SceneManager::addShader(&m_cubemap->equirectangularToCubemapShader);
	m_pickingbuffer = new PickingBuffer();
	SceneManager::addShader(&m_pickingbuffer->pickingShader);
	m_inputManager = new InputManager(window, m_camera);
	m_uiManager = new UIManager(window, m_camera, m_inputManager);

	initScreenQuad();
	createOrResizeFrameBufferAndRenderTarget();

	AppConfig::initShaders();
	//import
	GLTFModel model(std::filesystem::absolute("..\\..\\res\\GltfModels\\Face_1.glb").string(), AppConfig::baseShader);

	createLightsSSBO();

	Light directionalLight;
	directionalLight.type = 1;
	directionalLight.intensity = 0.1f;
	directionalLight.position = glm::vec3(0, 300.0f, 0);
	directionalLight.direction = glm::vec3( 0.0f, 0.0f,  0.0f);
	directionalLight.ambient   = glm::vec3(0.05f);
	directionalLight.diffuse   = glm::vec3(0.8f);
	directionalLight.specular  = glm::vec3(1.0f);
	directionalLight.constant  = 1.0f;
	directionalLight.linear    = 0.09f;
	directionalLight.quadratic = 0.032f;

	addLight(directionalLight);
	updateLights();
	checkLightBuffer();
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); 
}

Renderer::~Renderer()
{
	glDeleteTextures(1, &m_screenTexture); 
	glDeleteTextures(1, &m_composedTexture); 
	glDeleteRenderbuffers(1, &m_mainRbo);
	glDeleteFramebuffers(1, &m_mainFbo);
	glDeleteFramebuffers(1, &m_composedFbo);
	glDeleteBuffers(1, &m_fullFrameQuadVBO);
	glDeleteVertexArrays(1, &m_fullFrameQuadVAO);
	glDeleteBuffers(1, &m_lightsSSBO);
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
}

void Renderer::updateLights()
{
		glNamedBufferSubData(m_lightsSSBO, 0, SceneManager::getLights().size() * sizeof(Light), SceneManager::getLights().data());
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
	if (m_mainFbo)
	{
		glDeleteFramebuffers(1, &m_mainFbo);
		glDeleteTextures(1, &m_screenTexture); 
		glDeleteRenderbuffers(1, &m_mainRbo);
		glDeleteTextures(1, &m_composedTexture); 
		glDeleteFramebuffers(1, &m_composedFbo);
	}

	glCreateFramebuffers(1, &m_mainFbo);
	glCreateFramebuffers(1, &m_composedFbo);
	glCreateRenderbuffers(1, &m_mainRbo);

	glNamedRenderbufferStorage(m_mainRbo, GL_DEPTH24_STENCIL8, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT);
	glNamedFramebufferRenderbuffer(m_mainFbo, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_mainRbo);

	m_nMipLevels = (int)floor(log2(std::max(AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT))) + 1;
	// create a color attachment texture
	glCreateTextures(GL_TEXTURE_2D, 1, &m_screenTexture);
	glTextureStorage2D(m_screenTexture, m_nMipLevels, GL_RGBA32F, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT);
	glTextureParameteri(m_screenTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(m_screenTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(m_screenTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_screenTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_screenTexture, GL_TEXTURE_MAX_LEVEL, m_nMipLevels - 1);
	glNamedFramebufferTexture(m_mainFbo, GL_COLOR_ATTACHMENT0 , m_screenTexture, 0);


	GLint width, height, mipLevels, maxLevels;
	glGetTextureParameteriv(m_screenTexture, GL_TEXTURE_IMMUTABLE_LEVELS, &mipLevels);
	glGetTextureParameteriv(m_screenTexture, GL_TEXTURE_MAX_LEVEL, &maxLevels);
	glGetTextureLevelParameteriv(m_screenTexture, maxLevels, GL_TEXTURE_WIDTH, &width);
	glGetTextureLevelParameteriv(m_screenTexture, maxLevels, GL_TEXTURE_HEIGHT, &height);

	glCreateTextures(GL_TEXTURE_2D, 1, &m_composedTexture);
	glTextureStorage2D(m_composedTexture, m_nMipLevels, GL_RGBA32F, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT);
	glTextureParameteri(m_composedTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(m_composedTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(m_composedTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_composedTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glNamedFramebufferTexture(m_composedFbo, GL_COLOR_ATTACHMENT0, m_composedTexture, 0);

	GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glNamedFramebufferDrawBuffers(m_mainFbo, 1, drawBuffers);
}

void Renderer::checkFrameBufeerSize()
{
	if (!AppConfig::isFramebufferSizeSetted || !m_uiManager->viewportSizeSetteled)
	{
		AppConfig::RENDER_WIDTH = (int)m_uiManager->getViewportSize().x;
		AppConfig::RENDER_HEIGHT = (int)m_uiManager->getViewportSize().y;
		createOrResizeFrameBufferAndRenderTarget();

		m_pickingbuffer->resize();

		glViewport(0, 0, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT);

		AppConfig::isFramebufferSizeSetted = true;
	}
}

void Renderer::mainPass()
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Main Pass");
		glBindFramebuffer(GL_FRAMEBUFFER, m_mainFbo);
		glClearColor(AppConfig::clearColor[0], AppConfig::clearColor[1], AppConfig::clearColor[2], AppConfig::clearColor[3]); 
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, AppConfig::polygonMode);
		SceneManager::setShader(AppConfig::baseShader);
		for (const auto& primitive: SceneManager::getPrimitives())
		{
			glm::mat4 projection = glm::mat4(1.0f);
			if( AppConfig::RENDER_WIDTH != 0 && AppConfig::RENDER_HEIGHT != 0) 
			{
				projection = glm::perspective(glm::radians(m_camera.zoom), float(AppConfig::RENDER_WIDTH)/float(AppConfig::RENDER_HEIGHT),0.1f, 100.0f);	
			}

			const bool hasDiffuse = primitive.material && primitive.material->diffuse && 
									!primitive.material->diffuse->path.empty();
									
			const bool hasSpecular = primitive.material && primitive.material->specular && 
									!primitive.material->specular->path.empty();

			const bool hasNormal = primitive.material && primitive.material->normal && 
									!primitive.material->normal->path.empty();

			primitive.shader.use();
			primitive.shader.setVec3("viewPos", m_camera.position);
			primitive.shader.setFloat("gamma", AppConfig::gamma);
			glBindTextureUnit(5, m_shadowMap->depthMap);
			if (hasDiffuse)
			{
				// std::cout << "Binding diffuse texture ID: " << primitive.material->diffuse->id << std::endl;
				primitive.shader.setInt("tDiffuse", 1);
				glBindTextureUnit(1, primitive.material->diffuse->id);
			}
			else
				glBindTextureUnit(1, 0);
			
			if (hasSpecular)
			{
				primitive.shader.setInt("tSpecular", 2);
				primitive.shader.setFloat("shininess", 32);
				glBindTextureUnit(2, primitive.material->specular->id);
			}else glBindTextureUnit(2, 0);
			
			if (hasNormal)
			{
				primitive.shader.setInt("tNormal", 3);
				glBindTextureUnit(3, primitive.material->normal->id);
			}else glBindTextureUnit(3, 0);

			primitive.shader.setMat4("projection", projection);
			primitive.shader.setMat4("view", m_camera.getViewMatrix());
			primitive.shader.setMat4("model", primitive.transform);
			
			glBindVertexArray(primitive.vao);
			int eboSize = 0;
			glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &eboSize);
			const int indexSize = eboSize / sizeof(int);

			
			glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, (void*)0);
			glActiveTexture(GL_TEXTURE0);
			if (primitive.selected == true)
			{
				std::cerr << "drawing outline of: "  << primitive.vao << std::endl;
			}
			primitive.shader.setInt("irradianceMap", 4);
			glBindTextureUnit(4, m_cubemap->irradianceMap);

			primitive.shader.setFloat("irradianceMapRotationY", AppConfig::irradianceMapRotationY);
			primitive.shader.setFloat("irradianceMapIntensity", AppConfig::irradianceMapIntensity);

			glBindVertexArray(0);
			}
			//CUBEMAP RENDER PASS
			m_cubemap->draw(m_projection);

			glGenerateTextureMipmap(m_screenTexture);
			float avgLum[4]; 
			glGetTextureImage(m_screenTexture, m_nMipLevels-1, GL_RGBA, GL_FLOAT, sizeof(avgLum), avgLum);

			// compute luminance (e.g. Rec. 709)
			const float sceneLum = 0.2126f*avgLum[0] + 0.7152f*avgLum[1] + 0.0722f*avgLum[2];
			const float key = 0.18f;
			const float eps = 1e-2f;
			float newExposure = key / (std::max(sceneLum, eps) + eps);
			const float tau = 0.4f;  
			float alpha = 1.0f - glm::exp(-m_deltaTime / tau);
			if (!std::isfinite(newExposure)) newExposure = 1.0f;
			if (!std::isfinite(alpha))        alpha       = 1.0f;
			AppConfig::exposure = glm::mix(AppConfig::exposure, newExposure, alpha);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glPopDebugGroup();
}

void Renderer::composedPass()
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Composed Pass");

	//SCREEN QUAD RENDER PASS
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindFramebuffer(GL_FRAMEBUFFER, m_composedFbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glClearColor(AppConfig::clearColor[0], AppConfig::clearColor[1], AppConfig::clearColor[2], AppConfig::clearColor[3]); 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	AppConfig::screenShader.use();  
	AppConfig::screenShader.setFloat("near_plane", AppConfig::near_plane);
	AppConfig::screenShader.setFloat("far_plane", AppConfig::far_plane);
	AppConfig::screenShader.setFloat("exposure", AppConfig::exposure);
	glBindVertexArray(m_fullFrameQuadVAO);
	glBindTextureUnit(0, m_screenTexture);
	glDrawArrays(GL_TRIANGLES, 0, 6);  
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTextureUnit(0, 0);
	glPopDebugGroup();
}

void Renderer::render(GLFWwindow* window)
{
	while(!glfwWindowShouldClose(window))
	{
		
		const float time = (float)glfwGetTime();
		m_deltaTime = time - m_lastFrameTime;
		m_lastFrameTime = time;

		checkFrameBufeerSize();

		if( AppConfig::RENDER_WIDTH != 0 && AppConfig::RENDER_HEIGHT != 0) 
		{
			m_projection = glm::perspective(glm::radians(m_camera.zoom), float(AppConfig::RENDER_WIDTH)/float(AppConfig::RENDER_HEIGHT),0.1f, 100.0f);	
			m_view = m_camera.getViewMatrix();
		}

		//DIRECTIONAL LIGHT SHADOW MAP PASS
		m_shadowMap->bind();
		m_shadowMap->draw(m_camera);

		//OBJECT ID PASS
		m_pickingbuffer->bind();
		m_pickingbuffer->draw(m_camera);

		//MAIN RENDER PASS
		updateLights();
		mainPass();

		//SCREEN QUAD RENDER PASS
		composedPass();

		glfwPollEvents();
		m_inputManager->setPickingBuffer(m_pickingbuffer);
		m_inputManager->setWindowPos(m_uiManager->getWindowPos());
		m_inputManager->processInput(m_deltaTime, m_uiManager);
		m_uiManager->setScreenTexture(m_composedTexture);
		m_uiManager->setPickingTexture(m_pickingbuffer->getPickingTexture());
		m_uiManager->setShadowMapTexture(m_shadowMap->depthMap);
		m_uiManager->draw(m_deltaTime);
		glfwSwapBuffers(window);
	}
	this->~Renderer();
}
