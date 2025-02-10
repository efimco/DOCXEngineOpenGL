#pragma once
#include "shader.h"
#include <vector>
#include "glm/glm.hpp"
#include "camera.h"
#include "material.h"

struct Primitive 
{
	uint32_t vao;
	uint32_t vbo;
	uint32_t ebo;
	Shader shader;
	Shader outlineShader;
	size_t indexCount;
	glm::mat4 transform;
	bool selected;
	Mat material;

	Primitive(uint32_t vao, uint32_t vbo, uint32_t ebo, Shader shader, size_t indexCount, glm::mat4 transform)
		: vao(vao), vbo(vbo), ebo(ebo), shader(shader), indexCount(indexCount), transform(transform),
		outlineShader(std::filesystem::absolute("..\\..\\src\\shaders\\outlineVert.glsl").string(),
						std::filesystem::absolute("..\\..\\src\\shaders\\outlineFrag.glsl").string()),
		selected(false), material(){};

	Primitive(const Primitive&) = delete;
	Primitive& operator=(const Primitive&) = delete;

	Primitive(Primitive&& other) noexcept
		:vao(other.vao),
		vbo(other.vbo),
		ebo(other.ebo),
		shader(std::move(other.shader)),
		indexCount(other.indexCount),
		transform(other.transform),
		outlineShader(std::filesystem::absolute("..\\..\\src\\shaders\\outlineVert.glsl").string(),
						std::filesystem::absolute("..\\..\\src\\shaders\\outlineFrag.glsl").string()),
						selected(false) 
		{
		other.vao = 0;
		other.vbo = 0;
		other.ebo = 0;
		}
	
	~Primitive()
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &ebo);
	}
	
	void draw(Camera& camera, int32_t width, int32_t height)
	{
		const auto view = camera.getViewMatrix();
		const auto projection = glm::perspective(glm::radians(camera.zoom), float(width == 0 ? 1 : width)/float(height == 0 ? 1 : height),0.1f, 100.0f);	
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
		shader.use();
		if (material.diffuse.path != "")
		{
			shader.setInt(material.diffuse.type, 1);
			glBindTextureUnit(1, material.diffuse.id);
		}
		if (material.specular.path != "")
		{
			shader.setInt(material.specular.type, 2);
			shader.setFloat("shininess", 32);
			glBindTextureUnit(2, material.specular.id);

		}
		shader.setMat4("projection",projection);
		shader.setMat4("view",view);
		shader.setMat4("model",transform);
		
		glBindVertexArray(vao);
		int eboSize = 0;
		glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &eboSize);
		int indexSize = eboSize / sizeof(int);
		glDrawElements(GL_TRIANGLES,indexSize,GL_UNSIGNED_INT,(void*)0);
		glActiveTexture(GL_TEXTURE0);
		if (selected == true)
		{
			glEnable(GL_STENCIL_TEST);
			glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
			glStencilMask(0x00); 
			float camDistancece = (float)glm::length(camera.position - glm::vec3(transform[3]));
			float outlineScale = 1.025f;
			scaledTransform = glm::scale(transform, glm::vec3(outlineScale));
			outlineShader.use();
			outlineShader.setMat4("projection",projection);
			outlineShader.setMat4("view",view);
			outlineShader.setMat4("model",scaledTransform);
			glDrawElements(GL_TRIANGLES,indexSize,GL_UNSIGNED_INT,(void*)0);
			glStencilMask(0xFF);
			glStencilFunc(GL_ALWAYS, 1, 0xFF); 
			glActiveTexture(GL_TEXTURE0);
			glDisable(GL_STENCIL_TEST);
		}
		
		glBindVertexArray(0);
	}
	private:
			glm::mat4 scaledTransform;
};