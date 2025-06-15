#pragma once
#include <cstdint>
#include "shader.hpp"
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

	void draw(glm::mat4 projection, glm::mat4 view);
	void createOrResize();

private:
	Shader* m_gBufferShader;
	uint32_t m_gBufferFBO;

	void initTextures();
};