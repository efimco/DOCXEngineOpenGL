#include <glad/glad.h>
#include <cstdint>
#include "depthBuffer.hpp"
#include "primitive.hpp"
#include "sceneManager.hpp"
#include <glm/gtc/matrix_transform.hpp>

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

DepthBuffer::~DepthBuffer()
{
	glDeleteTextures(1, &depthMap);
	glDeleteFramebuffers(1, &depthMapFBO);
};

void DepthBuffer::bind() 
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
};

void DepthBuffer::unbind() 
{ 
	glBindFramebuffer(GL_FRAMEBUFFER, 0); 
}
void DepthBuffer::draw(Camera& camera, glm::mat4 lightSpaceMatrix, int32_t width, int32_t height) 
{
	for (Primitive& primitive: SceneManager::getPrimitives())
	{
		glm::mat4 projection = glm::mat4(1.0f);
		if( width != 0 && height != 0) 
		{
			projection = glm::perspective(glm::radians(camera.zoom), float(width)/float(height),0.1f, 100.0f);	
		}
		primitive.shader.use();
		primitive.shader.setVec3("viewPos", camera.position);
		primitive.shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

	
		primitive.shader.setMat4("projection",projection);
		primitive.shader.setMat4("view",camera.getViewMatrix());
		primitive.shader.setMat4("model",primitive.transform);
		
		glBindVertexArray(primitive.vao);
		int eboSize = 0;
		glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &eboSize);
		int indexSize = eboSize / sizeof(int);
		glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, (void*)0);
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(0);
	}

};
