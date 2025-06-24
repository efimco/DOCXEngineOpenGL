#pragma once
#include "glm/glm.hpp"
#include "scene/sceneNode.hpp"

class Light : public SceneNode
{

public:

	enum LightType
	{
		POINT = 0,
		DIRECTIONAL = 1,
		SPOT = 2
	};

	struct alignas(16) GPUData
	{
		glm::vec4 position; // w - type
		glm::vec4 direction; // w - innerCutOff
		glm::vec4 ambient; // W - padding
		glm::vec4 diffuse; // W - padding
		glm::vec4 specular; // W - padding
		glm::vec4 attenuation; // (constant, linear, quadratic, outerCutOff)
		glm::vec4 extra; // (intensity, 0,0,0) – can be repurposed
	};

	Light(LightType type);


	GPUData getGPUData();

	LightType& getType();
	void setType(LightType type);
	glm::vec3& getPosition();
	void setPosition(glm::vec3 position);
	glm::vec3& getDirection();
	void setDirection(glm::vec3 direction);
	glm::vec3& getAmbient();
	void setAmbient(glm::vec3 ambient);
	glm::vec3& getDiffuse();
	void setDiffuse(glm::vec3 diffuse);
	glm::vec3& getSpecular();
	void setSpecular(glm::vec3 specular);

	void draw(glm::mat4 view, glm::mat4 projection);

private:
	LightType m_type;
	glm::vec3 m_position;
	glm::vec3 m_direction;
	glm::vec3 m_ambient;
	glm::vec3 m_diffuse;
	glm::vec3 m_specular;
	float m_constantAttenuation;
	float m_linearAttenuation;
	float m_quadraticAttenuation;
	float m_innerCutOff;
	float m_outerCutOff;
	float m_intensity;
};

