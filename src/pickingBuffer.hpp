#pragma once
#include "glm/glm.hpp"
#include <cstdint>
#include "primitive.hpp"

class PickingBuffer
{
public:
	uint32_t pickingFBO;
	uint32_t pickingRBO;	
	uint32_t pickingTexture;

	PickingBuffer();
	~PickingBuffer();

	void bind();
	void resize();
	glm::vec3 pickColorAt(double mouseX, double mouseY, int32_t windowHeight);
	Primitive* getIdFromPickColor(const glm::vec3 &color) ;
};

glm::vec3 rgb2hsv(const glm::vec3 &rgb);


