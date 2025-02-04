#include <cstdint>
#include "glm/glm.hpp"

struct PointLight
{
	uint32_t vao;
	uint32_t vbo;
	uint32_t ebo;
	glm::mat4 transform;
	float intensity;
	glm::vec3 color;
};

struct DirectionalLight
{
	uint32_t vao;
	uint32_t vbo;
	uint32_t ebo;
	glm::mat4 transform;
	glm::vec3 direction;
	float intensity;
	glm::vec3 color;
};

struct SpotLight
{
	uint32_t vao;
	uint32_t vbo;
	uint32_t ebo;
	glm::mat4 transform;
	glm::vec3 direction;
	float cutOff; //Phi 
	float intensity;
	glm::vec3 color;
};