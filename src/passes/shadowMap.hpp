#pragma once
#include "camera.hpp"
#include "shader.hpp"
#include "light.hpp"
#include "appConfig.hpp"

#include <glm/glm.hpp>
#include <cstdint>


class ShadowMap
{
public:
	uint32_t depthMap;

	ShadowMap(const int width, const int height);
	~ShadowMap();

	void draw(Camera& camera, Light light);

private:
	AppConfig& m_appConfig;
	Shader* depthShader;
	const int width;
	const int height;
	uint32_t depthMapFBO;

};