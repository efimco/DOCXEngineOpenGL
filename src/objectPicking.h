#pragma once
#include "glm/glm.hpp"
#include <vector>
#include "primitive.h"
#include <glad/glad.h>
glm::vec3 ScreenPosToWorldRay(
	double mouseX, double mouseY, int screenWidth, int screenHeight,
	glm::mat4 projection, glm::mat4 view)
{
	// mouse to clip space
	double x = (2.0f * mouseX) / screenWidth - 1.0f;
	double y = 1.0f - (2.0f * mouseY) / screenHeight;
	glm::vec4 ray_clip(x, y, -1.0f, 1.0f);

	// eye coords
	glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
	ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);

	// world coords
	glm::vec3 ray_world = glm::vec3(glm::inverse(view) * ray_eye);
	return glm::normalize(ray_world);
}

// Проверка пересечения луча со сферой (грубая проверка)
bool RayIntersectsSphere(glm::vec3 rayOrigin, glm::vec3 rayDir, const Primitive& obj) {
	glm::vec3 oc = rayOrigin - glm::vec3(obj.transform[3]);
	double b = 2.0f * glm::dot(oc, rayDir);
	double c = glm::dot(oc, oc) - .1 * .1;
	double discriminant = b * b - 4 * c;
	return (discriminant > 0);
}

Primitive* PickObject(
	double mouseX, double mouseY, int screenWidth, int screenHeight,
	glm::mat4 projection, glm::mat4 view, glm::vec3 cameraPos, std::vector<Primitive>& objects)
{
	glm::vec3 rayDir = ScreenPosToWorldRay(mouseX, mouseY, screenWidth, screenHeight, projection, view);

	Primitive* closestObject = nullptr;
	float closestDist = FLT_MAX;

	for (auto& obj : objects) {
		if (RayIntersectsSphere(cameraPos, rayDir, obj)) {
			float dist = glm::length(glm::vec4(cameraPos,1) - obj.transform[3]);
			if (dist < closestDist) {
				closestDist = dist;
				closestObject = &obj;
			}
		}
	}

	return closestObject;
}


uint32_t pickingFBO = 0;
uint32_t pickingTexture = 0;

void initPickingFBO(int& windowWidth, int& windowHeight)
{
	glGenFramebuffers(1, &pickingFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO);

	glGenTextures(1, &pickingTexture);
	glBindTexture(GL_TEXTURE_2D, pickingTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pickingTexture, 0);

	GLuint depthRenderBuffer;
	glGenRenderbuffers(1, &depthRenderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowWidth, windowHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);


	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: Picking FBO is not complete!" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::vec3 pickObjectAt(double mouseX, double mouseY, int32_t windowHeight)
{
	glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO);

	int readX = mouseX;
	int readY = windowHeight - mouseY;
	
	GLfloat  pixel[3];
	glPixelStoref(GL_PACK_ALIGNMENT, 1);
	glReadPixels(readX, readY, 1, 1, GL_RGB, GL_FLOAT, pixel);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	std::cout << "Pixel: \n" << pixel[0] << " " << pixel[1] << " " << pixel[2] << std::endl;
	return glm::vec3(pixel[0], pixel[1], pixel[2]);

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

Primitive* getIdFromPickColor(const glm::vec3 &color) 
	{
		const float golden_ratio_conjugate = 0.618033988749895f;
		glm::vec3 hsv = rgb2hsv(color);
		float h = hsv.x;
		Primitive* closestObject  = nullptr;
		const unsigned int MAX_PICKABLE_OBJECTS = 1000;  // adjust as needed
		for (Primitive& primitive: SceneManager::primitives) 
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

