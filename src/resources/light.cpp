#include "light.hpp"

Light::Light(LightType type) :m_type(type)
{
	m_position = this->transform.position;
	m_direction = glm::vec3(0, -1, 0);
	m_ambient = glm::vec3(0, 0, 0);
	m_diffuse = glm::vec3(1, 1, 1);
	m_specular = glm::vec3(1, 1, 1);
	m_constantAttenuation = 0;
	m_linearAttenuation = 0;
	m_quadraticAttenuation = 0;
	m_innerCutOff = 0;
	m_outerCutOff = 0;
	m_intensity = 0;
}

Light::GPUData Light::getGPUData()
{
	GPUData d{};
	d.position = glm::vec4(m_position, float(m_type));
	d.direction = glm::vec4(glm::normalize(m_direction), m_innerCutOff);
	d.ambient = glm::vec4(m_ambient, 0.0f);
	d.diffuse = glm::vec4(m_diffuse, 0.0f);
	d.specular = glm::vec4(m_specular, 0.0f);
	d.attenuation = glm::vec4(m_constantAttenuation, m_linearAttenuation, m_quadraticAttenuation, m_outerCutOff);
	d.extra = glm::vec4(m_intensity, 0.0f, 0.0f, 0.0f);
	return d;
}

Light::LightType& Light::getType()
{
	return m_type;
}

void Light::setType(LightType type)
{
	m_type = type;
}

glm::vec3& Light::getDirection()
{
	return m_direction;
}

void Light::setDirection(glm::vec3 direction)
{
	m_direction = direction;
}

glm::vec3& Light::getPosition()
{
	return m_position;
}

void Light::setPosition(glm::vec3 position)
{
	m_position = position;
}

glm::vec3& Light::getAmbient()
{
	return m_ambient;
}

void Light::setAmbient(glm::vec3 ambient)
{
	m_ambient = ambient;
}

glm::vec3& Light::getDiffuse()
{
	return m_diffuse;
}

void Light::setDiffuse(glm::vec3 diffuse)
{
	m_diffuse = diffuse;
}

glm::vec3& Light::getSpecular()
{
	return m_specular;
}

void Light::setSpecular(glm::vec3 specular)
{
	m_specular = specular;
}


void draw(glm::mat4 view, glm::mat4 projection)
{

}