#version 460 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 viewPos;
uniform float gamma;

uniform sampler2D tDiffuse;
uniform sampler2D tSpecular;
uniform float shininess;

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

layout (std430, binding = 0) buffer LightBuffer {
	Light lights[];
};

void main() {
	vec3 result = vec3(0.0);
	lights[0].intensity = 2;
	for (int i = 0; i < lights.length(); i++) 
	{
		Light currentLight = lights[i];
		if (currentLight.type == 0) 
		{ // Point light
			vec3 lightDir = normalize(vec3(currentLight.position[0], currentLight.position[1], currentLight.position[2]) - FragPos);
			float diff = max(dot(Normal, lightDir), 0.0);

			vec3 diffuse = vec3(currentLight.diffuse[0],currentLight.diffuse[1],currentLight.diffuse[2]) * diff * texture(tDiffuse, TexCoords).rgb;
			if (texture(tDiffuse, TexCoords).r == 0)
			{
				 diffuse = vec3(currentLight.diffuse[0],currentLight.diffuse[1],currentLight.diffuse[2]) * diff * vec3(1);
			}

			vec3 viewDir = normalize(viewPos - FragPos);
			vec3 halfwayDir = normalize(lightDir + viewDir);
			float spec = pow(max(dot(Normal, halfwayDir), 0.0), shininess);
			vec3 specular = vec3(currentLight.specular[0],currentLight.specular[1],currentLight.specular[2]) * spec * texture(tSpecular, TexCoords).rgb;
			if (texture(tSpecular, TexCoords).r == 0)
			{
				specular = vec3(currentLight.specular[0],currentLight.specular[1],currentLight.specular[2]) * spec * diff * vec3(0.25);
			}

			vec3 ambient = vec3(currentLight.ambient[0], currentLight.ambient[1], currentLight.ambient[2]) * texture(tDiffuse, TexCoords).rgb;

			float dist = length(vec3(currentLight.position[0], currentLight.position[1], currentLight.position[2]) - FragPos)/1000;
			float attenuation = 1.0 / (currentLight.constant + currentLight.linear * dist + currentLight.quadratic * dist * dist);
			result += (ambient + diffuse + specular ) * currentLight.intensity * attenuation;
		}
	}

	// Apply gamma correction
	// result = pow(result, vec3(1.0 / gamma));
	// FragColor = texture(tDiffuse, TexCoords);
	// FragColor = vec4(result,1);
	FragColor = vec4(result, 1);
}