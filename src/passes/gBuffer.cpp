#include "gBuffer.hpp"
#include <string>
#include <filesystem>
#include "glad/gl.h"
#include "appConfig.hpp"
#include "sceneManager.hpp"
#include <iostream>

GBuffer::GBuffer()
{
	m_gBufferFBO = 0;
	tAlbedo = 0;
	tMetallic = 0;
	tRoughness = 0;
	tNormal = 0;
	tPosition = 0;
	tDepth = 0;

	createOrResize();

	const std::string fShaderPath = std::filesystem::absolute("..\\..\\src\\shaders\\gBuffer.frag").string();
	const std::string vShaderPath = std::filesystem::absolute("..\\..\\src\\shaders\\gBuffer.vert").string();

	m_gBufferShader = new Shader(vShaderPath, fShaderPath);
	SceneManager::addShader(m_gBufferShader);
}



void GBuffer::initTextures()
{
	//Albedo
	glCreateTextures(GL_TEXTURE_2D, 1, &tAlbedo);
	glTextureStorage2D(tAlbedo, 1, GL_RGBA16F, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT);
	glTextureParameteri(tAlbedo, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tAlbedo, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tAlbedo, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tAlbedo, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(tAlbedo, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//Metallic
	glCreateTextures(GL_TEXTURE_2D, 1, &tMetallic);
	glTextureStorage2D(tMetallic, 1, GL_R16F, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT);
	glTextureParameteri(tMetallic, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tMetallic, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tMetallic, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tMetallic, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(tMetallic, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//Roughness
	glCreateTextures(GL_TEXTURE_2D, 1, &tRoughness);
	glTextureStorage2D(tRoughness, 1, GL_R16F, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT);
	glTextureParameteri(tRoughness, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tRoughness, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tRoughness, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tRoughness, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(tRoughness, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//Normal
	glCreateTextures(GL_TEXTURE_2D, 1, &tNormal);
	glTextureStorage2D(tNormal, 1, GL_RGB16F, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT);
	glTextureParameteri(tNormal, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tNormal, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tNormal, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tNormal, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(tNormal, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//tPosition
	glCreateTextures(GL_TEXTURE_2D, 1, &tPosition);
	glTextureStorage2D(tPosition, 1, GL_RGBA16F, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT);
	glTextureParameteri(tPosition, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tPosition, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tPosition, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tPosition, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(tPosition, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//Depth
	glCreateTextures(GL_TEXTURE_2D, 1, &tDepth);
	glTextureStorage2D(tDepth, 1, GL_DEPTH_COMPONENT32F, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT);
	glTextureParameteri(tDepth, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tDepth, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tDepth, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tDepth, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(tDepth, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

}



void GBuffer::createOrResize()
{
	if (m_gBufferFBO != 0)
	{
		glDeleteFramebuffers(1, &m_gBufferFBO);
		glDeleteTextures(1, &tAlbedo);
		glDeleteTextures(1, &tMetallic);
		glDeleteTextures(1, &tRoughness);
		glDeleteTextures(1, &tNormal);
		glDeleteTextures(1, &tPosition);
		glDeleteTextures(1, &tDepth);
	}
	initTextures();
	glCreateFramebuffers(1, &m_gBufferFBO);
	// Attach all color textures
	glNamedFramebufferTexture(m_gBufferFBO, GL_COLOR_ATTACHMENT0, tAlbedo, 0);
	glNamedFramebufferTexture(m_gBufferFBO, GL_COLOR_ATTACHMENT1, tMetallic, 0);
	glNamedFramebufferTexture(m_gBufferFBO, GL_COLOR_ATTACHMENT2, tRoughness, 0);
	glNamedFramebufferTexture(m_gBufferFBO, GL_COLOR_ATTACHMENT3, tNormal, 0);
	glNamedFramebufferTexture(m_gBufferFBO, GL_COLOR_ATTACHMENT4, tPosition, 0);
	glNamedFramebufferTexture(m_gBufferFBO, GL_DEPTH_ATTACHMENT, tDepth, 0);

	// Specify draw buffers
	GLenum drawBuffers[] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4
	};
	glNamedFramebufferDrawBuffers(m_gBufferFBO, 5, drawBuffers);

	// Check framebuffer completeness
	if (glCheckNamedFramebufferStatus(m_gBufferFBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "gBuffer Framebuffer is not complete!" << std::endl;
	}
}

void GBuffer::draw(glm::mat4 projection, glm::mat4 view)
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "gBuffer Pass");

	glBindFramebuffer(GL_FRAMEBUFFER, m_gBufferFBO);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	for (auto& primitive : SceneManager::getPrimitives())
	{
		const bool hasDiffuse =
			primitive->material && primitive->material->diffuse && !primitive->material->diffuse->path.empty();
		const bool hasSpecular =
			primitive->material && primitive->material->specular && !primitive->material->specular->path.empty();
		const bool hasNormal =
			primitive->material && primitive->material->normal && !primitive->material->normal->path.empty();

		m_gBufferShader->use();

		glBindTextureUnit(1, hasDiffuse ? primitive->material->diffuse->id : 0);
		glBindTextureUnit(2, hasSpecular ? primitive->material->specular->id : 0);
		glBindTextureUnit(3, hasNormal ? primitive->material->normal->id : 0);

		m_gBufferShader->setMat4("projection", projection);
		m_gBufferShader->setMat4("view", view);
		m_gBufferShader->setMat4("model", primitive->transform.matrix);

		m_gBufferShader->setFloat("ufRoughness", primitive->material->roughness);
		m_gBufferShader->setFloat("ufMetallic", primitive->material->metallic);
		primitive->draw();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);
	glPopDebugGroup();

}