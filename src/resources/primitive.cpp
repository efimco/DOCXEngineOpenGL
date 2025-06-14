#pragma once

#include "primitive.hpp"
#include "camera.hpp"
#include "material.hpp"
#include "shader.hpp"
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

Primitive::Primitive(uint32_t vao, uint32_t vbo, uint32_t ebo, size_t indexCount, Transform transformation,
					 std::pair<glm::vec3, glm::vec3> boundingBox, std::shared_ptr<Mat> material, std::string name)
	: vao(vao), vbo(vbo), ebo(ebo), indexCount(indexCount), boundingBox(boundingBox), material(material)
{
	this->transform = transformation;
	this->name = name;
}

Primitive::Primitive(const Primitive &other)
{
	this->vao = other.vao;
	this->vbo = other.vbo;
	this->ebo = other.ebo;
	this->indexCount = other.indexCount;
	this->transform = other.SceneNode::transform;
	this->boundingBox = other.boundingBox;
	this->material = std::move(other.material);
	this->name = other.name;
}

Primitive::Primitive(Primitive &&other) noexcept
	: vao(other.vao), vbo(other.vbo), ebo(other.ebo), indexCount(other.indexCount), boundingBox(other.boundingBox),
	  material(std::move(other.material))
{
	this->transform = other.transform;
	this->name = other.name;
	other.vao = 0;
	other.vbo = 0;
	other.ebo = 0;
	other.indexCount = 0;
	other.boundingBox = {glm::vec3(0.0f), glm::vec3(0.0f)};
	other.transform = Transform();
}

Primitive &Primitive::operator=(Primitive &&other) noexcept
{
	if (this == &other)
		return *this;

	this->vao = other.vao;
	this->vbo = other.vbo;
	this->ebo = other.ebo;
	this->indexCount = other.indexCount;
	this->transform = other.SceneNode::transform;
	this->boundingBox = other.boundingBox;
	this->material = std::move(other.material);
	this->name = other.name;

	other.vao = 0;
	other.vbo = 0;
	other.ebo = 0;
	other.indexCount = 0;
	other.SceneNode::transform = Transform();

	return *this;
}

Primitive::~Primitive()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	this->material.reset();
}

void Primitive::draw() const
{
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
}


void *Primitive::operator new(size_t size) { return primitiveAllocator.allocate(size, alignof(Primitive)); }

void Primitive::operator delete(void *ptr) noexcept
{
	primitiveAllocator.deallocate(ptr, sizeof(Primitive), alignof(Primitive));
}
