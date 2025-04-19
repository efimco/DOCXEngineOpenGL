struct Light 
{
	int type;
	float intensity;
	float position[3];
	float direction[3];
	float ambient[3];
	float diffuse[3];
	float specular[3];

	// Attenuation
	float constant;
	float linear;
	float quadratic;

	float cutOff;       // inner cone angle
	float outerCutOff;  // outer cone angle
};