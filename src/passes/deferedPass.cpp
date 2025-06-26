#include "deferedPass.hpp"
#include "sceneManager.hpp"
#include <glad/gl.h>
#include <filesystem>

DeferedPass::DeferedPass(Camera& camera) : m_appConfig(AppConfig::get()), m_camera(camera)
{
	deferedTexture = 0;
	m_deferedFBO = 0;
	m_deferedRBO = 0;
	createOrResize(1);

	const std::string fPickingShader = std::filesystem::absolute("..\\..\\src\\shaders\\picking.frag").string();
	const std::string vPickingShader = std::filesystem::absolute("..\\..\\src\\shaders\\picking.vert").string();
	pickingShader = new Shader(vPickingShader, fPickingShader);
	SceneManager::addShader(pickingShader);

}

void DeferedPass::createOrResize(uint32_t m_nMipLevels)
{
	if (m_deferedFBO <= 0)
	{
		glDeleteFramebuffers(1, &m_deferedFBO);
		glDeleteTextures(1, &deferedTexture);
	}

	glCreateFramebuffers(1, &m_deferedFBO);
	glCreateRenderbuffers(1, &m_deferedRBO);

	glNamedRenderbufferStorage(m_deferedRBO, GL_DEPTH24_STENCIL8, m_appConfig.renderWidth, m_appConfig.renderHeight);
	glNamedFramebufferRenderbuffer(m_deferedFBO, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_deferedRBO);

	glCreateTextures(GL_TEXTURE_2D, 1, &deferedTexture);
	glTextureStorage2D(deferedTexture, m_nMipLevels, GL_RGBA16F, m_appConfig.renderWidth, m_appConfig.renderHeight);
	glTextureParameteri(deferedTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(deferedTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(deferedTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(deferedTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glNamedFramebufferTexture(m_deferedFBO, GL_COLOR_ATTACHMENT0, deferedTexture, 0);
}



void DeferedPass::draw(uint32_t fullFrameQuadVAO, GBuffer* gBuffer, Cubemap* cubemap, ShadowMap* shadowMap, PickingPass* pickingPass)
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Defered Pass");
	glClearColor(0, 0, 0, 1);
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, m_appConfig.renderWidth, m_appConfig.renderHeight);
	glPolygonMode(GL_FRONT_AND_BACK, m_appConfig.polygonMode);
	glActiveTexture(GL_TEXTURE0);

	glBindFramebuffer(GL_FRAMEBUFFER, m_deferedFBO);
	m_appConfig.deferedShader->use();
	glBindVertexArray(fullFrameQuadVAO);
	glBindTextureUnit(0, gBuffer->tAlbedo);
	glBindTextureUnit(1, gBuffer->tMetallic);
	glBindTextureUnit(2, gBuffer->tRoughness);
	glBindTextureUnit(3, gBuffer->tNormal);
	glBindTextureUnit(4, gBuffer->tPosition);
	glBindTextureUnit(5, gBuffer->tDepth);

	glBindTextureUnit(6, cubemap->irradianceMap);
	glBindTextureUnit(7, shadowMap->depthMap);
	glBindTextureUnit(8, cubemap->brdfLUTTexture);
	glBindTextureUnit(9, cubemap->specularMap);
	glBindTextureUnit(10, cubemap->envCubemap);

	glBindTextureUnit(11, pickingPass->pickingTexture);

	GLuint pickingSSBO;
	glGenBuffers(1, &pickingSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, pickingSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, SceneManager::getSelectedPrimitives().size() * sizeof(uint32_t),
		SceneManager::getSelectedPrimitives().data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, pickingSSBO); // 12 matches binding in shader

	if (SceneManager::getSelectedPrimitives().size() > 0)
	{
		m_appConfig.deferedShader->setIntArray(
			"selectedPrimitives", static_cast<uint32_t>(SceneManager::getSelectedPrimitives().size()),
			reinterpret_cast<const int32_t*>(SceneManager::getSelectedPrimitives().data()));
	}

	m_appConfig.deferedShader->setVec3("viewPos", m_camera.position);
	m_appConfig.deferedShader->setFloat("irradianceMapRotationY", m_appConfig.irradianceMapRotationY);
	m_appConfig.deferedShader->setFloat("irradianceMapIntensity", m_appConfig.irradianceMapIntensity);

	m_appConfig.deferedShader->setFloat("nearPlane", m_appConfig.nearPlane);
	m_appConfig.deferedShader->setFloat("farPlane", m_appConfig.farPlane);


	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTextureUnit(0, 0);
	glEnable(GL_DEPTH_TEST);
	glPopDebugGroup();
}
