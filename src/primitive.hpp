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
		void draw(Camera camera,glm::mat4 lightSpaceMatrix, int32_t width, int32_t height);
		void draw(Camera camera, glm::mat4 lightSpaceMatrix, int32_t width, int32_t height, uint32_t depthMap, float gamma);
		void draw(Camera& camera, glm::mat4 lightSpaceMatrix, int32_t width, int32_t height, Shader& shader, uint32_t depthMap, float gamma);

	private:
		glm::vec3 setPickColor(unsigned int id);
		glm::vec3 hsv2rgb(float h, float s, float v);
		glm::mat4 scaledTransform;
};