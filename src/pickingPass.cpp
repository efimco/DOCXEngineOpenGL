#include "pickingPass.hpp"
#include "glad/gl.h"
#include "appConfig.hpp"
#include "filesystem"
#include "sceneManager.hpp"
#include <glm/glm.hpp>
#include <iostream>
PickingPass::PickingPass()
{
	pickingTexture = 0;
	m_pickingFBO = 0;
	m_pickingRBO = 0;

	createOrResize();

	const std::string fPickingShader = std::filesystem::absolute("..\\..\\src\\shaders\\picking.frag").string();
	const std::string vPickingShader = std::filesystem::absolute("..\\..\\src\\shaders\\picking.vert").string();
	pickingShader = new Shader(vPickingShader, fPickingShader);
	SceneManager::addShader(pickingShader);
}

void PickingPass::createOrResize()
{

	if (m_pickingFBO != 0)
	{
		glDeleteFramebuffers(1, &m_pickingFBO);
		glDeleteRenderbuffers(1, &m_pickingRBO);
		glDeleteTextures(1, &pickingTexture);
	}

	glCreateTextures(GL_TEXTURE_2D, 1, &pickingTexture);
	glTextureStorage2D(pickingTexture, 1, GL_R32I, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT);
	glTextureParameteri(pickingTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(pickingTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(pickingTexture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(pickingTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(pickingTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glCreateFramebuffers(1, &m_pickingFBO);
	glCreateRenderbuffers(1, &m_pickingRBO);

	glNamedFramebufferRenderbuffer(m_pickingFBO, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_pickingRBO);
	glNamedRenderbufferStorage(m_pickingRBO, GL_DEPTH_COMPONENT24, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT);
	glNamedFramebufferTexture(m_pickingFBO, GL_COLOR_ATTACHMENT0, pickingTexture, 0);
}

void PickingPass::draw(glm::mat4 projection, glm::mat4 view)
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Picking Pass");
	glBindFramebuffer(GL_FRAMEBUFFER, m_pickingFBO);
	const int zero = 0;
	glClearBufferiv(GL_COLOR, 0, &zero);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	for (auto &primitive : SceneManager::getPrimitives())
	{
		pickingShader->use();
		pickingShader->setMat4("projection", projection);
		pickingShader->setMat4("view", view);
		pickingShader->setMat4("model", primitive.transform);
		pickingShader->setInt("objectID", primitive.vao);
		primitive.draw();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glPopDebugGroup();
}
