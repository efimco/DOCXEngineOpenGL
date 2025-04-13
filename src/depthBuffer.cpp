#include <glad/glad.h>
#include <cstdint>
#include "depthBuffer.hpp"


DepthBuffer::DepthBuffer(const int width, const int height):width(width), height(height)
		{
			glCreateFramebuffers(1, &depthMapFBO);

			glCreateTextures(GL_TEXTURE_2D, 1, &depthMap);
			glTextureStorage2D(depthMap, 1, GL_DEPTH_COMPONENT24, width, height);
			glTextureParameteri(depthMap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(depthMap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(depthMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTextureParameteri(depthMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			glTextureParameterfv(depthMap, GL_TEXTURE_BORDER_COLOR, borderColor);

			glNamedFramebufferTexture(depthMapFBO, GL_DEPTH_ATTACHMENT, depthMap, 0);
			glNamedFramebufferDrawBuffer(depthMapFBO, GL_NONE);
			glNamedFramebufferReadBuffer(depthMapFBO, GL_NONE);
		};

DepthBuffer::~DepthBuffer() = default;	
		
void DepthBuffer::bind() 
	{
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO); 
	};

void DepthBuffer::unbind() 
	{ 
		glBindFramebuffer(GL_FRAMEBUFFER, 0); 
	};
