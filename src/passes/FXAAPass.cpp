#include "FXAAPass.hpp"
#include <glad/gl.h>
#include "appConfig.hpp"
#include <string>
#include <filesystem>
#include "sceneManager.hpp"

FXAAPass::FXAAPass() : m_fxaaTextrue(0), m_fbo(0)
{
	createOrResize();
	initScreenQuad();
	const std::string fShaderPath = std::filesystem::absolute("..\\..\\src\\shaders\\fxaa.frag").string();
	const std::string vShaderPath = std::filesystem::absolute("..\\..\\src\\shaders\\fxaa.vert").string();
	m_shader = new Shader(vShaderPath, fShaderPath);
	SceneManager::addShader(m_shader);
}


void FXAAPass::draw(uint32_t deferedTextrue)
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "FXAA Pass");
	glClearColor(0, 0, 0, 0);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glActiveTexture(GL_TEXTURE0);

	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	m_shader->use();
	if (AppConfig::isFXAA == true)
		m_shader->setInt("isFXAA", 1);
	else
		m_shader->setInt("isFXAA", 0);
	glBindVertexArray(m_fullFrameQuadVAO);
	glBindTextureUnit(0, deferedTextrue);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindTextureUnit(0, 0);
	glDisable(GL_DEPTH_TEST);
	glPopDebugGroup();

}

void FXAAPass::createOrResize()
{

	if (m_fbo <= 0)
	{
		glDeleteFramebuffers(1, &m_fbo);
		glDeleteTextures(1, &m_fxaaTextrue);
	}

	glCreateTextures(GL_TEXTURE_2D, 1, &m_fxaaTextrue);
	glTextureStorage2D(m_fxaaTextrue, 1, GL_RGBA16F, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT);
	glTextureParameteri(m_fxaaTextrue, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_fxaaTextrue, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_fxaaTextrue, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_fxaaTextrue, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_fxaaTextrue, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glCreateFramebuffers(1, &m_fbo);
	glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_fxaaTextrue, 0);
}

void FXAAPass::initScreenQuad()
{

	glCreateVertexArrays(1, &m_fullFrameQuadVAO);
	glCreateBuffers(1, &m_fullFrameQuadVBO);
	glNamedBufferData(m_fullFrameQuadVBO, sizeof(m_fullFrameQuadVertices), &m_fullFrameQuadVertices, GL_STATIC_DRAW);
	glVertexArrayVertexBuffer(m_fullFrameQuadVAO, 0, m_fullFrameQuadVBO, 0, sizeof(float) * 4);

	glEnableVertexArrayAttrib(m_fullFrameQuadVAO, 0);
	glVertexArrayAttribFormat(m_fullFrameQuadVAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(m_fullFrameQuadVAO, 0, 0);

	glEnableVertexArrayAttrib(m_fullFrameQuadVAO, 1);
	glVertexArrayAttribFormat(m_fullFrameQuadVAO, 1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2);
	glVertexArrayAttribBinding(m_fullFrameQuadVAO, 1, 0);
}