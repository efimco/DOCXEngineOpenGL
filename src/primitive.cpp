#pragma once

#include "camera.hpp"
#include "shader.hpp"
#include "material.hpp"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include "primitive.hpp"

Primitive::Primitive(uint32_t vao, uint32_t vbo, uint32_t ebo, Shader shader, size_t indexCount, glm::mat4 transform, std::shared_ptr<Mat> material)
: vao(vao),
	vbo(vbo),
	ebo(ebo),
	shader(shader),
	indexCount(indexCount),
	transform(transform),
	selected(false), 
	material(material){};

Primitive::Primitive(Primitive&& other) noexcept
	:vao(other.vao),
	vbo(other.vbo),
	ebo(other.ebo),
	shader(std::move(other.shader)),
	indexCount(other.indexCount),
	transform(other.transform),
	material(std::move(other.material)),
					selected(false) 
	{
		other.vao = 0;
		other.vbo = 0;
		other.ebo = 0;
	}

Primitive::~Primitive()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
}

void Primitive::draw(Camera& camera, int32_t width, int32_t height, uint32_t depthMap, float gamma, uint32_t cubemapID)
{
	glm::mat4 projection = glm::mat4(1.0f);
	if( width != 0 && height != 0) 
	{
		projection = glm::perspective(glm::radians(camera.zoom), float(width)/float(height),0.1f, 100.0f);	
	}
	shader.use();
	shader.setVec3("viewPos", camera.position);
	shader.setFloat("gamma", gamma);
	glBindTextureUnit(5, depthMap);
	glBindTextureUnit(4, cubemapID);
	if (material && material->diffuse && !material->diffuse->path.empty())
	{
		shader.setInt("tDiffuse", 1);
		glBindTextureUnit(1, material->diffuse->id);
	}
	else
		glBindTextureUnit(1, 0);
	
	if (material && material->specular && !material->specular->path.empty())
	{
		shader.setInt("tSpecular", 2);
		shader.setFloat("shininess", 32);
		glBindTextureUnit(2, material->specular->id);
	}else glBindTextureUnit(2, 0);
	
	if (material && material->normal && !material->normal->path.empty())
	{
		shader.setInt("tNormal", 3);
		glBindTextureUnit(3, material->normal->id);
	}else glBindTextureUnit(3, 0);

	shader.setMat4("projection",projection);
	shader.setMat4("view",camera.getViewMatrix());
	shader.setMat4("model",transform);
	
	glBindVertexArray(vao);
	int eboSize = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &eboSize);
	int indexSize = eboSize / sizeof(int);

	
	glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, (void*)0);
	glActiveTexture(GL_TEXTURE0);
	if (selected == true)
	{
		std::cerr << "drawing outline of: "  << vao << std::endl;
	}
	
	glBindVertexArray(0);
}