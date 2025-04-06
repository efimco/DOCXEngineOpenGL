#pragma once

#include "camera.hpp"
#include "shader.hpp"
#include "material.hpp"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include "primitive.hpp"

Primitive::Primitive(uint32_t vao, uint32_t vbo, uint32_t ebo, Shader shader, size_t indexCount, glm::mat4 transform, Mat material)
: vao(vao), vbo(vbo), ebo(ebo), shader(shader), indexCount(indexCount), transform(transform),
selected(false), material(material){};

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

void Primitive::draw(Camera camera,glm::mat4 lightSpaceMatrix, int32_t width, int32_t height)
{
	draw(camera, lightSpaceMatrix, width, height, shader, 0,0);
}
void Primitive::draw(Camera camera, glm::mat4 lightSpaceMatrix, int32_t width, int32_t height, uint32_t depthMap, float gamma)
{
	draw(camera, lightSpaceMatrix, width, height, shader, depthMap, gamma);
}

void Primitive::draw(Camera& camera, glm::mat4 lightSpaceMatrix, int32_t width, int32_t height, Shader& shader, uint32_t depthMap, float gamma)
{
	glm::mat4 projection = glm::mat4(1.0f);
	if( width != 0 && height != 0) 
	{
		projection = glm::perspective(glm::radians(camera.zoom), float(width)/float(height),0.1f, 100.0f);	
	}
	shader.use();
	shader.setVec3("viewPos", camera.position);
	shader.setVec3("objectIDColor", setPickColor(vao));
	shader.setInt("shadowMap", 5);
	shader.setFloat("gamma", gamma);
	glBindTextureUnit(5, depthMap);
	shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
	if (material.diffuse -> path != "")
	{
		shader.setInt(material.diffuse -> type, 1);
		glBindTextureUnit(1, material.diffuse -> id);
	}else glBindTextureUnit(1, 0);
	
	if (material.specular -> path != "")
	{
		shader.setInt(material.specular -> type, 2);
		shader.setFloat("shininess", 32);
		glBindTextureUnit(2, material.specular -> id);
	}else glBindTextureUnit(2, 0);
	
	if (material.normal -> path != "")
	{
		shader.setInt(material.normal -> type, 3);
		glBindTextureUnit(3, material.normal -> id);
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

glm::vec3 Primitive::setPickColor(unsigned int id)
{
	float golden_ratio_conjugate = 0.618033988749895f;
	float h = glm::fract(id * golden_ratio_conjugate);
	float s = 0.7f; // moderately saturated
	float v = 1.0f; // bright

	glm::vec3 idColor = hsv2rgb(h,s,v);
	return idColor;
}

glm::vec3 Primitive::hsv2rgb(float h, float s, float v) {
	float c = v * s;
	float h_prime = h * 6.0f;
	float x = c * (1.0f - fabs(fmod(h_prime, 2.0f) - 1.0f));
	glm::vec3 rgb;
	
	if (h_prime < 1.0f)
		rgb = glm::vec3(c, x, 0.0f);
	else if (h_prime < 2.0f)
		rgb = glm::vec3(x, c, 0.0f);
	else if (h_prime < 3.0f)
		rgb = glm::vec3(0.0f, c, x);
	else if (h_prime < 4.0f)
		rgb = glm::vec3(0.0f, x, c);
	else if (h_prime < 5.0f)
		rgb = glm::vec3(x, 0.0f, c);
	else
		rgb = glm::vec3(c, 0.0f, x);
	
	float m = v - c;
	glm::vec3 res = rgb + glm::vec3(m);
	return res;
}
