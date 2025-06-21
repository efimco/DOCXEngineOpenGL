struct Light
{
	int type;
	float intensity;
	vec3 position;
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	// Attenuation
	float constant;
	float linear;
	float quadratic;

	float cutOff;       // inner cone angle
	float outerCutOff;  // outer cone angle
};