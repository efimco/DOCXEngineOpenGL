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
	m_historyValid = false;
	m_pingPong = false;
	m_currentJitter = glm::vec2(0.0f);
	m_prevJitter = glm::vec2(0.0f);
}

void TAAPass::createOrResize()
{
	if (history0 != 0)
	{
		glDeleteTextures(1, &history0);
		glDeleteTextures(1, &history1);
		glDeleteTextures(1, &m_prevDepth);
	}

	// ping-pong history textures
	glCreateTextures(GL_TEXTURE_2D, 1, &history0);
	glTextureStorage2D(history0, 1, GL_RGBA16F, m_appConfig.renderWidth, m_appConfig.renderHeight);
	glTextureParameteri(history0, GL_TEXTURE_MIN_FILTER, GL_POINT);
	glTextureParameteri(history0, GL_TEXTURE_MAG_FILTER, GL_POINT);
	glTextureParameteri(history0, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(history0, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glCreateTextures(GL_TEXTURE_2D, 1, &history1);
	glTextureStorage2D(history1, 1, GL_RGBA16F, m_appConfig.renderWidth, m_appConfig.renderHeight);
	glTextureParameteri(history1, GL_TEXTURE_MIN_FILTER, GL_POINT);
	glTextureParameteri(history1, GL_TEXTURE_MAG_FILTER, GL_POINT);
	glTextureParameteri(history1, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(history1, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// previous depth texture for depth rejection
	glCreateTextures(GL_TEXTURE_2D, 1, &m_prevDepth);
	glTextureStorage2D(m_prevDepth, 1, GL_R32F, m_appConfig.renderWidth, m_appConfig.renderHeight);
	glTextureParameteri(m_prevDepth, GL_TEXTURE_MIN_FILTER, GL_POINT);
	glTextureParameteri(m_prevDepth, GL_TEXTURE_MAG_FILTER, GL_POINT);
	glTextureParameteri(m_prevDepth, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_prevDepth, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glCreateTextures(GL_TEXTURE_2D, 1, &m_prevVelocity);
	glTextureStorage2D(m_prevVelocity, 1, GL_RG16F, m_appConfig.renderWidth, m_appConfig.renderHeight);
	glTextureParameteri(m_prevVelocity, GL_TEXTURE_MIN_FILTER, GL_POINT);
	glTextureParameteri(m_prevVelocity, GL_TEXTURE_MAG_FILTER, GL_POINT);
	glTextureParameteri(m_prevVelocity, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_prevVelocity, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	m_frameNumber = 0;
	m_historyValid = false;
	m_pingPong = false;
}

void TAAPass::draw()
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "TAA Pass");
	m_TAAShader->use();

	// read and write textures for ping-pong
	uint32_t readTexture = m_pingPong ? history1 : history0;
	uint32_t writeTexture = m_pingPong ? history0 : history1;
	glBindTextureUnit(0, m_current);
	glBindTextureUnit(1, readTexture);
	glBindImageTexture(2, writeTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glBindTextureUnit(3, m_velocity);
	glBindTextureUnit(4, m_prevVelocity);
	glBindTextureUnit(5, m_depth);
	glBindTextureUnit(6, m_prevDepth);

	glm::vec4 resolution(
		static_cast<float>(m_appConfig.renderWidth),
		static_cast<float>(m_appConfig.renderHeight),
		1.0f / static_cast<float>(m_appConfig.renderWidth),
		1.0f / static_cast<float>(m_appConfig.renderHeight)
	);


	m_TAAShader->setInt("isTAA", m_appConfig.isTAA ? 1 : 0);
	m_TAAShader->setInt("frameNumber", m_frameNumber);
	m_TAAShader->setInt("accumulationLimit", m_accumulationLimit);
	m_TAAShader->setVec2("cameraJitter", m_currentJitter);
	m_TAAShader->setVec2("prevCameraJitter", m_prevJitter);
	m_TAAShader->setVec4("resolution", resolution);
	m_TAAShader->setFloat("nearPlane", m_appConfig.nearPlane);
	m_TAAShader->setFloat("farPlane", m_appConfig.farPlane);

	glDispatchCompute((m_appConfig.renderWidth + 15) / 16, (m_appConfig.renderHeight + 7) / 8, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

	// сopy current depth to previous depth 
	glCopyImageSubData(m_depth, GL_TEXTURE_2D, 0, 0, 0, 0,
		m_prevDepth, GL_TEXTURE_2D, 0, 0, 0, 0,
		m_appConfig.renderWidth, m_appConfig.renderHeight, 1);

	glCopyImageSubData(m_velocity, GL_TEXTURE_2D, 0, 0, 0, 0,
		m_prevVelocity, GL_TEXTURE_2D, 0, 0, 0, 0,
		m_appConfig.renderWidth, m_appConfig.renderHeight, 1);

	// Flip ping-pong state
	m_pingPong = !m_pingPong;
	m_frameNumber++;
	m_historyValid = true;

	glPopDebugGroup();
}

uint32_t TAAPass::getCurrentOutput()
{
	return m_pingPong ? history0 : history1;
}

void TAAPass::setCurrrentTexture(uint32_t curentTexture)
{
	m_current = curentTexture;
}

void TAAPass::setVelocityTexture(uint32_t velocityTexture)
{
	m_velocity = velocityTexture;
}

void TAAPass::setDepthTexture(uint32_t depthTexture)
{
	m_depth = depthTexture;
}

void TAAPass::setAccumulationLimit(uint32_t accumulationLimit)
{
	m_accumulationLimit = accumulationLimit;
}

void TAAPass::setCurrentFrameNumber(uint32_t frameNumber)
{
	m_frameNumber = frameNumber;
}

void TAAPass::setJitterValues(glm::vec2 currentJitter, glm::vec2 prevJitter)
{
	m_currentJitter = currentJitter;
	m_prevJitter = prevJitter;
}
