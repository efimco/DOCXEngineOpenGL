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

void setPickColor(unsigned int id)
{
	unsigned char r = (id >> 16) & 0xFF;
	unsigned char g = (id >> 8) & 0xFF;
	unsigned char b = id & 0xFF;
	glm::vec3 color = glm::vec3(r,g,b);
	std::cout << color.x << " " << color.y << " " << color.z << std::endl;
}
