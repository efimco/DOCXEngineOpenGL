#pragma once
#include "shader.hpp"
#include <cstdint>
#include <glm/glm.hpp>
#include "appConfig.hpp"

class PickingPass
{
public:
	PickingPass();
	void draw(glm::mat4 projection, glm::mat4 view);
	void createOrResize();
	uint32_t pickingTexture;
	Shader* pickingShader;
	uint32_t m_pickingFBO;

private:
	AppConfig& m_appConfig;
	uint32_t m_pickingRBO;

	void initTextures();
};