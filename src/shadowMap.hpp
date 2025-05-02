#pragma once
#include <cstdint>
#include "camera.hpp"
#include "glm/glm.hpp"

class ShadowMap
{
public:
	const int width;
	const int height;
	uint32_t depthMapFBO;
	uint32_t depthMap;
	ShadowMap(const int width, const int height);
	~ShadowMap();	

	void bind();
	void unbind();
	void draw(Camera& camera);
};