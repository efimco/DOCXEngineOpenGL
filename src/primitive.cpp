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
