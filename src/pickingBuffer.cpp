#include <vector>
#include <glad/glad.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "primitive.hpp"
#include "sceneManager.hpp"
#include "pickingBuffer.hpp"




PickingBuffer::PickingBuffer(int& windowWidth, int& windowHeight)
	{
		glCreateFramebuffers(1, &pickingFBO);
		glCreateTextures(GL_TEXTURE_2D, 1, &pickingTexture);
		
		glTextureStorage2D(pickingTexture, 1, GL_RGB8, windowWidth, windowHeight);
		glTextureParameteri(pickingTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(pickingTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		
		glNamedFramebufferTexture(pickingFBO, GL_COLOR_ATTACHMENT0, pickingTexture, 0);
		
		if (glCheckNamedFramebufferStatus(pickingFBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cerr << "Error: Picking FBO is not complete!" << std::endl;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

PickingBuffer::~PickingBuffer()
{
	glDeleteTextures(1, &pickingTexture);
	glDeleteFramebuffers(1, &pickingFBO);
}

void PickingBuffer::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void PickingBuffer::resize(int& windowWidth, int& windowHeight)
{
	glDeleteTextures(1, &pickingTexture);

	glCreateTextures(GL_TEXTURE_2D, 1, &pickingTexture);
	
	glTextureStorage2D(pickingTexture, 1, GL_RGB8, windowWidth, windowHeight);
	glTextureParameteri(pickingTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(pickingTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	glNamedFramebufferTexture(pickingFBO, GL_COLOR_ATTACHMENT0, pickingTexture, 0);
	
	if (glCheckNamedFramebufferStatus(pickingFBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: Picking FBO is not complete!" << std::endl;
	}

}

glm::vec3 PickingBuffer::pickColorAt(double mouseX, double mouseY, int32_t windowHeight)
{
	glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO);

	int readX = (int) mouseX;
	int readY = (int) (windowHeight - mouseY);
	
	GLfloat  pixel[3];
	glPixelStoref(GL_PACK_ALIGNMENT, 1);
	glReadPixels(readX, readY, 1, 1, GL_RGB, GL_FLOAT, pixel);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	std::cout << "Pixel: \n" << pixel[0] << " " << pixel[1] << " " << pixel[2] << std::endl;
	return glm::vec3(pixel[0], pixel[1], pixel[2]);

}

Primitive* PickingBuffer::getIdFromPickColor(const glm::vec3 &color) 
{
	const float golden_ratio_conjugate = 0.618033988749895f;
	glm::vec3 hsv = rgb2hsv(color);
	float h = hsv.x;
	Primitive* closestObject  = nullptr;
	const unsigned int MAX_PICKABLE_OBJECTS = 1000;  // adjust as needed
	for (Primitive& primitive: SceneManager::getPrimitives()) 
		{
			float computedH = glm::fract(primitive.vao * golden_ratio_conjugate);
			std::cout << computedH << " " << h << std::endl;
			// Allow a small tolerance since floating-point imprecision can occur
			if (glm::abs(computedH - h) < 0.01f) 
			{
				closestObject = &primitive;
				break;
			}
		}
	return closestObject;
}

glm::vec3 rgb2hsv(const glm::vec3 &rgb) {
	float r = rgb.r, g = rgb.g, b = rgb.b;
	float cmax = glm::max(r, glm::max(g, b));
	float cmin = glm::min(r, glm::min(g, b));
	float delta = cmax - cmin;
	float h = 0.0f;
	
	if (delta < 1e-6f) {
		h = 0.0f;
	}
	else if (cmax == r)
	{
		h = fmod((g - b) / delta, 6.0f);
	}
	else if (cmax == g)
	{
		h = ((b - r) / delta) + 2.0f;
	}
	else
	{
		h = ((r - g) / delta) + 4.0f;
	}
	h /= 6.0f;
	if (h < 0.0f)
		h += 1.0f;
	
	float s = (cmax <= 0.0f ? 0.0f : delta / cmax);
	float v = cmax;
	return glm::vec3(h, s, v);
}



