#pragma once
#include <cstdint>
#include "shader.hpp"
#include "appConfig.hpp"

class FXAAPass
{
public:
	FXAAPass();
	~FXAAPass() = default;

	uint32_t m_fxaaTextrue;

	void draw(uint32_t deferedTextrue);
	void createOrResize();
private:
	AppConfig& m_appConfig;
	uint32_t m_fbo, m_rbo;
	uint32_t m_fullFrameQuadVAO, m_fullFrameQuadVBO;
	Shader* m_shader;

	void initScreenQuad();

	inline static const float m_fullFrameQuadVertices[] = {
		// positions   // texCoords
		-1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,

		-1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f };
};