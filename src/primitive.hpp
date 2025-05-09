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
	Shader shader;
	size_t indexCount;
	glm::mat4 transform;
	bool selected;
	std::shared_ptr<Mat> material;
	Primitive(uint32_t vao, uint32_t vbo, uint32_t ebo, Shader shader, size_t indexCount, glm::mat4 transform, std::shared_ptr<Mat> material);
	Primitive(const Primitive&) = delete;
	Primitive& operator=(const Primitive&) = delete;
	Primitive(Primitive&& other) noexcept;
	~Primitive();
};