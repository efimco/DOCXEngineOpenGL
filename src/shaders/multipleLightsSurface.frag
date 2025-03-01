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
uniform samplerCube skybox;

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

vec3 calcPointLight(Light light)
	{
	vec3 lightDir = normalize(vec3(light.position[0], light.position[1], light.position[2]) - FragPos);
	float diff = max(dot(Normal, lightDir), 0.0);

	vec3 diffuse = vec3(light.diffuse[0],light.diffuse[1],light.diffuse[2]) * diff * texture(tDiffuse, TexCoords).rgb;

	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(Normal, halfwayDir), 0.0), shininess);
	vec3 specular = vec3(light.specular[0], light.specular[1], light.specular[2]) * spec * texture(tSpecular, TexCoords).r;

	if (texture(tSpecular, TexCoords).r == 0)
	{
		spec = pow(max(dot(Normal, halfwayDir), 0.0), 8);
		specular = vec3(light.specular[0], light.specular[1], light.specular[2]) * spec * diff * vec3(0.25);
	}

		if (texture(tDiffuse, TexCoords).r == 0)
	{
			diffuse = vec3(light.diffuse[0],light.diffuse[1],light.diffuse[2]) * diff * vec3(1);
	}

	vec3 ambient = vec3(light.ambient[0], light.ambient[1], light.ambient[2]) * texture(tDiffuse, TexCoords).rgb;

	float dist = length(vec3(light.position[0], light.position[1], light.position[2]) - FragPos)/1000;
	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);
	return vec3 ((ambient + diffuse + specular ) * light.intensity * attenuation);

}

vec3 calcDirectionalLight(Light light)
	{
		vec3 lightDir = normalize(vec3(light.direction[0], light.direction[1], light.direction[2]));
		float diff = max(dot(Normal, lightDir), 0.0);

		vec3 viewDir = normalize(viewPos - FragPos);
		vec3 halfwayDir = normalize(lightDir + viewDir);
		float spec = pow(max(dot(Normal, halfwayDir), 0.0), shininess);
		vec3 specular = vec3(light.specular[0], light.specular[1], light.specular[2]) * spec * texture(tSpecular, TexCoords).r;

		if (texture(tSpecular, TexCoords).r == 0)
		{
			spec = pow(max(dot(Normal, halfwayDir), 0.0), 8);
			specular = vec3(light.specular[0], light.specular[1], light.specular[2]) * spec * diff * vec3(0.25);
		}

				vec3 diffuse = vec3(light.diffuse[0],light.diffuse[1],light.diffuse[2]) * diff * texture(tDiffuse, TexCoords).rgb;
		if (texture(tDiffuse, TexCoords).r == 0)
		{
				diffuse = vec3(light.diffuse[0],light.diffuse[1],light.diffuse[2]) * diff * vec3(1);
		}

		vec3 ambient = vec3(light.ambient[0], light.ambient[1], light.ambient[2]) * texture(tDiffuse, TexCoords).rgb;

		return vec3((ambient + diffuse + specular) * light.intensity);
	}

vec3 calcSpotLight(Light light)
	{
	vec3 lightDir = normalize(vec3(light.position[0], light.position[1], light.position[2]) - FragPos);
	float diff = max(dot(Normal, lightDir), 0.0);

	vec3 diffuse = vec3(light.diffuse[0],light.diffuse[1],light.diffuse[2]) * diff * texture(tDiffuse, TexCoords).rgb;

	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(Normal, halfwayDir), 0.0), shininess);
	vec3 specular = vec3(light.specular[0], light.specular[1], light.specular[2]) * spec * texture(tSpecular, TexCoords).r;
	
	float theta     = dot(lightDir, normalize(-vec3(light.direction[0], light.direction[1], light.direction[2])));
	float epsilon   = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0); 

	if (texture(tSpecular, TexCoords).r == 0)
	{
		spec = pow(max(dot(Normal, halfwayDir), 0.0), 8);
		specular = vec3(light.specular[0], light.specular[1], light.specular[2]) * spec * diff * vec3(0.25);
	}

		if (texture(tDiffuse, TexCoords).r == 0)
	{
			diffuse = vec3(light.diffuse[0],light.diffuse[1],light.diffuse[2]) * diff * vec3(1);
	}
	diffuse *= intensity;
	specular *= intensity;

	vec3 ambient = vec3(light.ambient[0], light.ambient[1], light.ambient[2]) * texture(tDiffuse, TexCoords).rgb;

	float dist = length(vec3(light.position[0], light.position[1], light.position[2]) - FragPos)/1000;
	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

	if (theta > light.outerCutOff)
	{
		return vec3 ((ambient + diffuse + specular ) * light.intensity * attenuation);
	}
	else return vec3(ambient);
	
		
	}

void main() {
	vec3 result = vec3(0.0);
	for (int i = 0; i < lights.length(); i++) 
	{
		Light currentLight = lights[i];
		switch (currentLight.type)
		{
			case 0: // Point light
			{ 
				result += calcPointLight(currentLight);
			}
			case 1: // Directional light
			{
				result += calcDirectionalLight(currentLight);
			}
			case 2: // Spot light
			{
				result += calcSpotLight(currentLight);
			}
		}
	}

	// Apply gamma correction
	// result = pow(result, vec3(1.0 / gamma));
	// FragColor = texture(tDiffuse, TexCoords);
	// FragColor = vec4(result,1);
	FragColor = vec4(result, 1);
}