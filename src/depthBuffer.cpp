#include <glad/glad.h>
#include <cstdint>
#include "depthBuffer.hpp"


DepthBuffer::DepthBuffer(const int width, const int height):width(width), height(height)
		{
			glGenFramebuffers(1, &depthMapFBO);  

			glGenTextures(1, &depthMap);
			glBindTexture(GL_TEXTURE_2D, depthMap);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);  
			float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);  
			
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
			glBindFramebuffer(GL_FRAMEBUFFER, 0); 
		};

DepthBuffer::~DepthBuffer() = default;	
		
void DepthBuffer::bindDepthMap() 
	{
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO); 
	};

void DepthBuffer::unbindDepthMap() 
	{ 
		glBindFramebuffer(GL_FRAMEBUFFER, 0); 
	};
	
uint32_t DepthBuffer::getDepthMap() 
	{
	return depthMap; 
	};
