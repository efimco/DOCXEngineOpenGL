#pragma once
#include "glm/glm.hpp"
#include <cstdint>
#include "primitive.hpp"

class PickingBuffer
{
public:
	PickingBuffer();
	~PickingBuffer();

	void bind();
	void draw(Camera& camera);
	void resize();
	glm::vec3 pickColorAt(double mouseX, double mouseY, int32_t windowHeight);
	Primitive* getIdFromPickColor(const glm::vec3 &color);

private:
	uint32_t m_pickingFBO;
	uint32_t m_pickingRBO;
	uint32_t m_pickingTexture;
	glm::vec3 setPickColor(unsigned int id);
};


glm::vec3 rgb2hsv(const glm::vec3 &rgb);

glm::vec3 hsv2rgb(float h, float s, float v);

