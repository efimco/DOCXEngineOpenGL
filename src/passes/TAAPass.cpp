#include "TAAPass.hpp"
#include <glad/gl.h>
#include <filesystem>
#include "sceneManager.hpp"
#include <algorithm>

TAAPass::TAAPass() : m_appConfig(AppConfig::get())
{
	const std::string computeShaderPath = std::filesystem::absolute("..\\..\\src\\shaders\\TAA.comp").string();
	m_TAAShader = new Shader(computeShaderPath);
	SceneManager::addShader(m_TAAShader);
	history = 0;
	current = 0;
	output = 0;
	m_FBOCurrent = 0;
	m_FBOHistory = 0;
	m_frameNumber = 0;
}

void TAAPass::createOrResize()
{
	if (history != 0)
	{
		glDeleteTextures(1, &history);
	}

	// Create history/accumulation texture
	glCreateTextures(GL_TEXTURE_2D, 1, &history);
	glTextureStorage2D(history, 1, GL_RGBA16F, m_appConfig.renderWidth, m_appConfig.renderHeight);
	glTextureParameteri(history, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(history, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(history, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(history, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	m_frameNumber = 0;
}


void TAAPass::draw()
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "TAA Pass");
	m_TAAShader->use();

	// Bind current frame and history as input textures
	glBindTextureUnit(0, current);

	// Bind output as writeonly image
	glBindImageTexture(0, current, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
	glBindImageTexture(1, history, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
	glBindImageTexture(2, m_velocity, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG16F);
	glBindImageTexture(3, m_depth, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);

	// Set uniforms
	m_TAAShader->setInt("isTAA", m_appConfig.isTAA ? 1 : 0);

	glDispatchCompute((m_appConfig.renderWidth + 15) / 16, (m_appConfig.renderHeight + 7) / 8, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
	m_frameNumber++;

	glPopDebugGroup();
}

void TAAPass::setCurrrentTexture(uint32_t curentTexture)
{
	current = curentTexture;
}

void TAAPass::setVelocityTexture(uint32_t velocityTexture)
{
	m_velocity = velocityTexture;
}

void TAAPass::setDepthTexture(uint32_t depthTexture)
{
	m_depth = depthTexture;
}
