#include "gBuffer.hpp"
#include <string>
#include <filesystem>
#include "glad/gl.h"
#include "appConfig.hpp"
#include "sceneManager.hpp"
#include <iostream>


static uint32_t jitterIndex = 0;
static glm::vec2 jitter;
static glm::vec2 prevJitter;


GBuffer::GBuffer() : m_appConfig(AppConfig::get())
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

	m_prevView = glm::mat4(1);
	m_prevProjection = glm::mat4(1);
}



void GBuffer::initTextures()
{
	//Albedo
	glCreateTextures(GL_TEXTURE_2D, 1, &tAlbedo);
	glTextureStorage2D(tAlbedo, 1, GL_RGBA16F, m_appConfig.renderWidth, m_appConfig.renderHeight);
	glTextureParameteri(tAlbedo, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tAlbedo, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tAlbedo, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tAlbedo, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(tAlbedo, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//Metallic
	glCreateTextures(GL_TEXTURE_2D, 1, &tMetallic);
	glTextureStorage2D(tMetallic, 1, GL_R16F, m_appConfig.renderWidth, m_appConfig.renderHeight);
	glTextureParameteri(tMetallic, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tMetallic, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tMetallic, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tMetallic, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(tMetallic, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//Roughness
	glCreateTextures(GL_TEXTURE_2D, 1, &tRoughness);
	glTextureStorage2D(tRoughness, 1, GL_R16F, m_appConfig.renderWidth, m_appConfig.renderHeight);
	glTextureParameteri(tRoughness, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tRoughness, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tRoughness, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tRoughness, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(tRoughness, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//Normal
	glCreateTextures(GL_TEXTURE_2D, 1, &tNormal);
	glTextureStorage2D(tNormal, 1, GL_RGB16F, m_appConfig.renderWidth, m_appConfig.renderHeight);
	glTextureParameteri(tNormal, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tNormal, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tNormal, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tNormal, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(tNormal, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//tPosition
	glCreateTextures(GL_TEXTURE_2D, 1, &tPosition);
	glTextureStorage2D(tPosition, 1, GL_RG16F, m_appConfig.renderWidth, m_appConfig.renderHeight);
	glTextureParameteri(tPosition, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tPosition, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tPosition, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tPosition, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(tPosition, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//Depth
	glCreateTextures(GL_TEXTURE_2D, 1, &tDepth);
	glTextureStorage2D(tDepth, 1, GL_DEPTH_COMPONENT32F, m_appConfig.renderWidth, m_appConfig.renderHeight);
	glTextureParameteri(tDepth, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tDepth, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tDepth, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tDepth, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(tDepth, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//Velocity
	glCreateTextures(GL_TEXTURE_2D, 1, &tVelocity);
	glTextureStorage2D(tVelocity, 1, GL_RG16F, m_appConfig.renderWidth, m_appConfig.renderHeight);
	glTextureParameteri(tVelocity, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tVelocity, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tVelocity, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tVelocity, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(tVelocity, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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
		glDeleteTextures(1, &tVelocity);
	}
	initTextures();
	glCreateFramebuffers(1, &m_gBufferFBO);
	// Attach all color textures
	glNamedFramebufferTexture(m_gBufferFBO, GL_COLOR_ATTACHMENT0, tAlbedo, 0);
	glNamedFramebufferTexture(m_gBufferFBO, GL_COLOR_ATTACHMENT1, tMetallic, 0);
	glNamedFramebufferTexture(m_gBufferFBO, GL_COLOR_ATTACHMENT2, tRoughness, 0);
	glNamedFramebufferTexture(m_gBufferFBO, GL_COLOR_ATTACHMENT3, tNormal, 0);
	glNamedFramebufferTexture(m_gBufferFBO, GL_COLOR_ATTACHMENT4, tPosition, 0);
	glNamedFramebufferTexture(m_gBufferFBO, GL_COLOR_ATTACHMENT5, tVelocity, 0);
	glNamedFramebufferTexture(m_gBufferFBO, GL_DEPTH_ATTACHMENT, tDepth, 0);

	// Specify draw buffers
	GLenum drawBuffers[] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4,
		GL_COLOR_ATTACHMENT5
	};
	glNamedFramebufferDrawBuffers(m_gBufferFBO, 6, drawBuffers);

	// Check framebuffer completeness
	if (glCheckNamedFramebufferStatus(m_gBufferFBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "gBuffer Framebuffer is not complete!" << std::endl;
	}
}

float Halton(uint32_t i, uint32_t b)
{
	float f = 1.0f;
	float r = 0.0f;

	while (i > 0)
	{
		f /= static_cast<float>(b);
		r = r + f * static_cast<float>(i % b);
		i = static_cast<uint32_t>(floorf(static_cast<float>(i) / static_cast<float>(b)));
	}

	return r;
}

void GBuffer::draw(glm::mat4 projection, glm::mat4 view, float cameraDistance)
{

	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "gBuffer Pass");

	glBindFramebuffer(GL_FRAMEBUFFER, m_gBufferFBO);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, m_appConfig.renderWidth, m_appConfig.renderHeight);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	// in [-0.5, 0.5] range (fraction of a pixel)
	float haltonX = 2 * Halton((jitterIndex % 8) + 1, 2) - 0.5f;
	float haltonY = 2 * Halton((jitterIndex % 8) + 1, 3) - 0.5f;

	jitter.x = haltonX / static_cast<float>(m_appConfig.renderWidth);
	jitter.y = haltonY / static_cast<float>(m_appConfig.renderHeight);
	float jitterScale = glm::clamp(cameraDistance * 0.1f, 0.01f, 1.0f);

	jitter.x *= jitterScale;
	jitter.y *= jitterScale;

	projection = glm::translate(projection, glm::vec3(jitter.x * 2.0f, jitter.y * 2.0f, 0.0f));

	for (auto& primitive : SceneManager::getPrimitives())
	{
		const bool hasDiffuse =
			primitive->material && primitive->material->tDiffuse && !primitive->material->tDiffuse->path.empty();
		const bool hasSpecular =
			primitive->material && primitive->material->tSpecular && !primitive->material->tSpecular->path.empty();
		const bool hasNormal =
			primitive->material && primitive->material->tNormal && !primitive->material->tNormal->path.empty();

		m_gBufferShader->use();

		glBindTextureUnit(1, hasDiffuse ? primitive->material->tDiffuse->id : 0);
		glBindTextureUnit(2, hasSpecular ? primitive->material->tSpecular->id : 0);
		glBindTextureUnit(3, hasNormal ? primitive->material->tNormal->id : 0);

		m_gBufferShader->setMat4("projection", projection);
		m_gBufferShader->setMat4("view", view);

		m_gBufferShader->setMat4("prevProjection", m_prevProjection);
		m_gBufferShader->setMat4("prevView", m_prevView);

		m_gBufferShader->setMat4("model", primitive->transform.matrix);
		m_gBufferShader->setMat4("prevModel", primitive->transform.prevMatrix);

		primitive->transform.prevMatrix = primitive->transform.matrix;

		m_gBufferShader->setFloat("ufRoughness", primitive->material->roughness);
		m_gBufferShader->setFloat("ufMetallic", primitive->material->metallic);
		m_gBufferShader->setVec3("uvAlbedo", primitive->material->albedo);

		m_gBufferShader->setVec2("jitter", jitter);
		m_gBufferShader->setVec2("prevJitter", prevJitter);

		primitive->draw();
	}
	jitterIndex++;
	m_prevProjection = projection;
	m_prevView = view;
	prevJitter = jitter;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);
	glPopDebugGroup();

}