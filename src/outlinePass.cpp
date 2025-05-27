#include <filesystem>
#include <glad/gl.h>
#include "appConfig.hpp"
#include "sceneManager.hpp"
#include "outlinePass.hpp"

OutlinePass::OutlinePass()
{
	m_fbo = 0;
	silhouetteTexture = 0;
	createOrResize();
	const std::string fsilhouetteShader = std::filesystem::absolute("..\\..\\src\\shaders\\silhouette.frag").string();
	const std::string vsilhouetteShader = std::filesystem::absolute("..\\..\\src\\shaders\\picking.vert").string();
	silhouetteShader = new Shader(vsilhouetteShader, fsilhouetteShader);
}

void OutlinePass::createOrResize()
{
	if (m_fbo)
	{
		glDeleteFramebuffers(1, &m_fbo);
		glDeleteRenderbuffers(1, &m_rbo);
		glDeleteTextures(1, &silhouetteTexture);
	}
	glCreateTextures(GL_TEXTURE_2D, 1, &silhouetteTexture);
	glTextureStorage2D(silhouetteTexture, 1, GL_RGB16, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT);
	glTextureParameteri(silhouetteTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(silhouetteTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(silhouetteTexture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(silhouetteTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(silhouetteTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glCreateFramebuffers(1, &m_fbo);
	
	glCreateRenderbuffers(1, &m_rbo);
	glNamedRenderbufferStorage(m_rbo, GL_DEPTH_COMPONENT24, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT);
	glNamedFramebufferRenderbuffer(m_fbo, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

	glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, silhouetteTexture, 0);
}

void OutlinePass::draw(glm::mat4 projection, glm::mat4 view, uint32_t depthFBO)
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "silhouette Pass");

	glBindFramebuffer(GL_READ_FRAMEBUFFER, depthFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glBlitFramebuffer(0, 0, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT,
			  0, 0, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT,
			  GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	GLint depthFunc;
	glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);


	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL); // Draw on top of equal depths

		glClear(GL_COLOR_BUFFER_BIT);
		glViewport(0, 0, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT);
		if (SceneManager::getSelectedPrimitive() != nullptr)
		{
			const auto primitive = SceneManager::getSelectedPrimitive();
			silhouetteShader->use();
			silhouetteShader->setMat4("projection", projection);
			silhouetteShader->setMat4("view", view);
			silhouetteShader->setMat4("model", primitive->transform);
			silhouetteShader->setInt("objectID", primitive->vao);
			primitive->draw();
		}		// Restore previous state
		glDepthFunc(depthFunc);

		// Reset framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	

	glPopDebugGroup();
}