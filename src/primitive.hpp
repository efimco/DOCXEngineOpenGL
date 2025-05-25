#pragma once
#include "shader.hpp"
#include "glm/glm.hpp"
#include "camera.hpp"
#include "material.hpp"
class Primitive 
{
public:
	uint32_t vao;
	uint32_t vbo;
	uint32_t ebo;
	glm::vec2 boundingBox; // minimum and maximum value
	size_t indexCount;
	glm::mat4 transform;
	std::shared_ptr<Mat> material;
	Primitive(uint32_t vao, uint32_t vbo, uint32_t ebo, size_t indexCount, glm::mat4 transform, std::shared_ptr<Mat> material);
	Primitive(Primitive&) = delete;
	Primitive& operator=(Primitive&) = delete;
	Primitive(Primitive&& other) noexcept;
	~Primitive();
	void draw() const;
};