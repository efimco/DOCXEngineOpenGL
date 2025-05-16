#include <iostream>
#include "stb_image.h"
#include "glad/glad.h"
#include "cubemap.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "appConfig.hpp"


Cubemap::Cubemap(Camera& camera, std::string pathToCubemap):
	camera(camera),
	m_cubeVAO(0), m_cubeVBO(0),
	m_quadVAO(0), m_quadVBO(0),
	m_hdri(0), envCubemap(0),
	m_captureFBO(0), m_captureRBO(0)
{
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); 

	loadHDR(pathToCubemap);

	initShaders();

	createCubemap();
	createIrradianceMap();
	createSpecularMap();
	createBRDFLUTTexture();
	initCaptureFBO();

	renderEquirectToCubemap();
	convoluteIrradianceCubemap();
	convoluteSpecularCubemap();
	renderBRDFLUTTExture();
}

Cubemap::~Cubemap()
{
	glDeleteVertexArrays(1, &m_cubeVAO);
	glDeleteBuffers(1, &m_cubeVBO);
	glDeleteTextures(1, &m_hdri);
	glDeleteTextures(1, &envCubemap);
	glDeleteFramebuffers(1, &m_captureFBO);
	glDeleteRenderbuffers(1, &m_captureRBO);
}

void Cubemap::initShaders()
{
	const std::string vBackgroundShader = std::filesystem::absolute("..\\..\\src\\shaders\\cubemap\\background.vert").string();
	const std::string fBackgroundShader = std::filesystem::absolute("..\\..\\src\\shaders\\cubemap\\background.frag").string();

	const std::string vEqrToCubemapShader = std::filesystem::absolute("..\\..\\src\\shaders\\cubemap\\equirectangularToCubemap.vert").string();
	const std::string fEqrToCubemapShader = std::filesystem::absolute("..\\..\\src\\shaders\\cubemap\\equirectangularToCubemap.frag").string();

	const std::string vIrradianceConvolutionShader = std::filesystem::absolute("..\\..\\src\\shaders\\cubemap\\equirectangularToCubemap.vert").string();
	const std::string fIrradianceConvolutionShader = std::filesystem::absolute("..\\..\\src\\shaders\\cubemap\\irradianceConvolution.frag").string();

	const std::string vSpecularConvolutionShader = std::filesystem::absolute("..\\..\\src\\shaders\\cubemap\\equirectangularToCubemap.vert").string();
	const std::string fSpecularConvolutionShader = std::filesystem::absolute("..\\..\\src\\shaders\\cubemap\\specularConvolution.frag").string();

	const std::string vBrdfLutShader = std::filesystem::absolute("..\\..\\src\\shaders\\cubemap\\brdfLut.vert").string();
	const std::string fBrdfLutShader = std::filesystem::absolute("..\\..\\src\\shaders\\cubemap\\brdfLut.frag").string();

	backgroundShader = Shader(vBackgroundShader, fBackgroundShader);
	equirectangularToCubemapShader = Shader(vEqrToCubemapShader, fEqrToCubemapShader);
	irradianceShader = Shader(vIrradianceConvolutionShader, fIrradianceConvolutionShader);
	specularShader = Shader(vSpecularConvolutionShader, fSpecularConvolutionShader);
	brdfLutShader = Shader(vBrdfLutShader, fBrdfLutShader);
	
}

void Cubemap::loadHDR(std::string pathToCubemap)
{
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); 
	float *imageData = stbi_loadf(pathToCubemap.c_str(), &width, &height, &nrChannels, 0);
	if (imageData) 
	{
		std::cout << "Loading HDR cubemap" << std::endl;
		glCreateTextures(GL_TEXTURE_2D, 1, &m_hdri);

		glTextureStorage2D(m_hdri, 1, GL_RGB16F, width, height);
		glTextureSubImage2D(m_hdri, 0, 0, 0, width, height, GL_RGB, GL_FLOAT, imageData);

		glTextureParameteri(m_hdri, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_hdri, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_hdri, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
		glTextureParameteri(m_hdri, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		stbi_image_free(imageData);
	} else 
	{
		std::cerr << "Failed to load HDR cubemap texture: " << pathToCubemap << std::endl;
		std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
	}
	stbi_set_flip_vertically_on_load(false);
}

const int cubemapSize = 512;  // or 1024, etc.

void Cubemap::createCubemap()
{
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &envCubemap);
	
	// Allocate storage for all mip levels at once
	glTextureStorage2D(envCubemap, 1, GL_RGB16F, cubemapSize, cubemapSize);
	
	// Set texture parameters
	glTextureParameteri(envCubemap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(envCubemap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(envCubemap, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(envCubemap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(envCubemap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

const int convolutedMapSize = 64;

void Cubemap::createIrradianceMap()
{
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &irradianceMap);
	
	// Allocate storage for all mip levels at once
	glTextureStorage2D(irradianceMap, 1, GL_RGB16F, convolutedMapSize, convolutedMapSize);
	
	// Set texture parameters
	glTextureParameteri(irradianceMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(irradianceMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(irradianceMap, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(irradianceMap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(irradianceMap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

const uint32_t maxMipLevels = 5;

void Cubemap::createSpecularMap()
{
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &specularMap);
	
	// Allocate storage for all mip levels at once
	glTextureStorage2D(specularMap, maxMipLevels, GL_RGB16F, convolutedMapSize, convolutedMapSize);
	
	// Set texture parameters
	glTextureParameteri(specularMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(specularMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(specularMap, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(specularMap, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
	glTextureParameteri(specularMap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateTextureMipmap(specularMap);

}

const uint32_t brdfLutSize = 512;

void Cubemap::createBRDFLUTTexture()
{
	glCreateTextures(GL_TEXTURE_2D, 1, &brdfLUTTexture);

	glTextureStorage2D(brdfLUTTexture, 1, GL_RGB16F, brdfLutSize, brdfLutSize);
	
	// Set texture parameters
	glTextureParameteri(brdfLUTTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(brdfLUTTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(brdfLUTTexture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(brdfLUTTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(brdfLUTTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


void Cubemap::initCaptureFBO()
{
	glCreateFramebuffers(1, &m_captureFBO);
	glCreateRenderbuffers(1, &m_captureRBO);

	glNamedRenderbufferStorage(m_captureRBO, GL_DEPTH_COMPONENT24, cubemapSize, cubemapSize);
	glNamedFramebufferRenderbuffer(m_captureFBO, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_captureRBO);
}


void Cubemap::renderEquirectToCubemap()
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Cubemap Capture");

	equirectangularToCubemapShader.use();
	equirectangularToCubemapShader.setInt("equirectangularMap", 0);
	glBindTextureUnit(0, m_hdri); 

	glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
	glViewport(0, 0, cubemapSize, cubemapSize);

	for (unsigned int i = 0; i < 6; ++i) {
		// attach cubemap face i to FBO color using DSA
		glNamedFramebufferTextureLayer(m_captureFBO, GL_COLOR_ATTACHMENT0, envCubemap, 0, i);
		equirectangularToCubemapShader.setMat4("view", m_captureViews[i]);
		equirectangularToCubemapShader.setMat4("projection", m_captureProjection);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderCube();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glPopDebugGroup();
}

void Cubemap::convoluteIrradianceCubemap()
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Irradiance Cubemap Convolution");

	irradianceShader.use();
	glBindTextureUnit(0, envCubemap); 

	glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
	glViewport(0, 0, convolutedMapSize, convolutedMapSize);

	for (unsigned int i = 0; i < 6; ++i) 
	{
		// attach cubemap face i to FBO color using DSA
		glNamedRenderbufferStorage(m_captureRBO, GL_DEPTH_COMPONENT24, convolutedMapSize, convolutedMapSize);
		glNamedFramebufferTextureLayer(m_captureFBO, GL_COLOR_ATTACHMENT0, irradianceMap, 0, i);
		irradianceShader.setMat4("view", m_captureViews[i]);
		irradianceShader.setMat4("projection", m_captureProjection);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderCube();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glPopDebugGroup();
}

void Cubemap::convoluteSpecularCubemap()
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Specaulr Cubemap Convolution");

	specularShader.use();
	glBindTextureUnit(0, envCubemap); 

	glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
	for (uint32_t mip = 0; mip < maxMipLevels; ++mip)
	{
		
		const uint32_t mipWidth  = static_cast<uint32_t>(convolutedMapSize * std::pow(0.5, mip));
		const uint32_t mipHeight = static_cast<uint32_t>(convolutedMapSize * std::pow(0.5, mip));
		glNamedRenderbufferStorage(m_captureRBO, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		specularShader.setFloat("roughness", roughness);
		for (uint32_t i = 0; i < 6; ++i) 
		{
			// attach cubemap face i to FBO color using DSA
			glNamedFramebufferTextureLayer(m_captureFBO, GL_COLOR_ATTACHMENT0, specularMap, mip, i);
			specularShader.setMat4("view", m_captureViews[i]);
			specularShader.setMat4("projection", m_captureProjection);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)	;
			renderCube();
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	glPopDebugGroup();

}

void Cubemap::renderBRDFLUTTExture()
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "BRDF LUT");

	glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
	glNamedRenderbufferStorage(m_captureRBO, GL_DEPTH_COMPONENT24, brdfLutSize, brdfLutSize);
	glNamedFramebufferTexture(m_captureFBO, GL_COLOR_ATTACHMENT0, brdfLUTTexture, 0 );

	glViewport(0, 0, brdfLutSize, brdfLutSize);
	brdfLutShader.use();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderQuad();

	glBindFramebuffer(GL_FRAMEBUFFER, 0); 

	glPopDebugGroup();
}

void Cubemap::renderCube()
{
	// initialize (if necessary)
	if (m_cubeVAO == 0)
	{		
		glCreateVertexArrays(1, &m_cubeVAO);
		glCreateBuffers(1, &m_cubeVBO);

		glNamedBufferData(m_cubeVBO, sizeof(m_cubemapVertices), m_cubemapVertices, GL_STATIC_DRAW);

		glVertexArrayVertexBuffer(m_cubeVAO, 0, m_cubeVBO, 0, 3 * sizeof(float));

		glEnableVertexArrayAttrib(m_cubeVAO, 0);

		glVertexArrayAttribFormat(m_cubeVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
	}
	// render Cube
	glBindVertexArray(m_cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void Cubemap::renderQuad()
{
	// initialize (if necessary)
	if (m_quadVAO == 0)
	{		
		glCreateVertexArrays(1, &m_quadVAO);
		glCreateBuffers(1, &m_quadVBO);

		glNamedBufferData(m_quadVBO, sizeof(m_quadPositions) + sizeof(m_quadCoords), NULL, GL_STATIC_DRAW);
		glNamedBufferSubData(m_quadVBO, 0, sizeof(m_quadPositions), m_quadPositions);
		glNamedBufferSubData(m_quadVBO, sizeof(m_quadPositions), sizeof(m_quadCoords), m_quadCoords);

		glVertexArrayVertexBuffer(m_quadVAO, 0, m_quadVBO, 0, 3 * sizeof(float));
		glVertexArrayVertexBuffer(m_quadVAO, 1, m_quadVBO, sizeof(m_quadPositions), 2 * sizeof(float));

		glEnableVertexArrayAttrib(m_quadVAO, 0);
		glEnableVertexArrayAttrib(m_quadVAO, 1);

		glVertexArrayAttribFormat(m_quadVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribFormat(m_quadVAO, 1, 2, GL_FLOAT, GL_FALSE, 0);

		glVertexArrayAttribBinding(m_quadVAO, 0, 0);
		glVertexArrayAttribBinding(m_quadVAO, 1, 1);
	}
	// render quad
	glBindVertexArray(m_quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void Cubemap::draw(glm::mat4 projection)
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Cubemap Pass");
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glm::mat4 skyView = camera.getViewMatrix();
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_FALSE);
		backgroundShader.use();
		backgroundShader.setMat4("projection", projection);
		backgroundShader.setMat4("view", skyView);
		glBindTextureUnit(0, envCubemap);
		backgroundShader.setInt("environmentMap", 0);
		backgroundShader.setFloat("rotationY", AppConfig::irradianceMapRotationY);
		backgroundShader.setFloat("intensity", AppConfig::irradianceMapIntensity);
		renderCube();
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);

		glPopDebugGroup();
	};