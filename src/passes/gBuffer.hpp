#pragma once
#include <cstdint>
#include "shader.hpp"
#include "appConfig.hpp"
#include <glm/glm.hpp>

class GBuffer
{
public:
	GBuffer();

	uint32_t tAlbedo;
	uint32_t tMetallic;
	uint32_t tRoughness;
	uint32_t tNormal;
	uint32_t tPosition;
	uint32_t tDepth;
	uint32_t tVelocity;

	void draw(glm::mat4 projection, glm::mat4 view, float cameraDistance);
	void createOrResize();

private:
	glm::mat4 m_prevView;
	glm::mat4 m_prevProjection;
	AppConfig& m_appConfig;
	Shader* m_gBufferShader;
	uint32_t m_gBufferFBO;

	void initTextures();
};