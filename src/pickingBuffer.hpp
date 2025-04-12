#pragma once
#include "glm/glm.hpp"
#include <cstdint>
#include "primitive.hpp"

class PickingBuffer
{
public:
	uint32_t pickingFBO;
	uint32_t pickingTexture;

	PickingBuffer(int& windowWidth, int& windowHeight);
	~PickingBuffer();

	void bind();
	void resize(int& windowWidth, int& windowHeight);
};


glm::vec3 pickObjectAt(double mouseX, double mouseY, int32_t windowHeight, uint32_t pickingFBO);

glm::vec3 rgb2hsv(const glm::vec3 &rgb);

Primitive* getIdFromPickColor(const glm::vec3 &color) ;

