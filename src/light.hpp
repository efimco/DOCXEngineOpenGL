#include "glm/glm.hpp"

struct Light 
{
	int   type;
	float intensity;
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 ambient; 
	glm::vec3 diffuse; 
	glm::vec3 specular;

	// Attenuation 
	float constant;
	float linear;
	float quadratic;

	float cutOff;       // inner cone angle
	float outerCutOff;  // outer cone angle
};

