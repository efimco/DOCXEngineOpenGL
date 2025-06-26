#include "postProcessPass.hpp"
#include "sceneManager.hpp"

#include <filesystem>
#include <glad/gl.h>


PostProcessPass::PostProcessPass() : m_appConfig(AppConfig::get()), m_postProcessedTexture(0)
{
	const std::string cShaderPath = std::filesystem::absolute("..\\..\\src\\shaders\\postProcess.comp").string();

	m_postProcessShader = new Shader(cShaderPath);
	SceneManager::addShader(m_postProcessShader);
	createOrResize();
}


void PostProcessPass::createOrResize()
{
	if (m_postProcessedTexture != 0)
	{
		glDeleteTextures(1, &m_postProcessedTexture);
	}

	glCreateTextures(GL_TEXTURE_2D, 1, &m_postProcessedTexture);
	glTextureStorage2D(m_postProcessedTexture, 1, GL_RGBA16F, m_appConfig.renderWidth, m_appConfig.renderHeight);
	glTextureParameteri(m_postProcessedTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_postProcessedTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_postProcessedTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(m_postProcessedTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

uint32_t PostProcessPass::getPostProcessedTexture() const
{
	return m_postProcessedTexture;
}

void PostProcessPass::draw(uint32_t prePostProcessTexture, uint32_t cubemap, uint32_t depthTexture)
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Post Process Pass");
	m_postProcessShader->use();

	glBindImageTexture(0, prePostProcessTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
	glBindImageTexture(1, m_postProcessedTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

	glBindImageTexture(2, cubemap, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
	glBindImageTexture(3, depthTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);

	m_postProcessShader->setFloat("nearPlane", m_appConfig.nearPlane);
	m_postProcessShader->setFloat("farPlane", m_appConfig.farPlane);

	glDispatchCompute((m_appConfig.renderWidth + 15) / 16, (m_appConfig.renderHeight + 7) / 8, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
	glPopDebugGroup();
}