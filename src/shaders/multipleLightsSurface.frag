#version 460 core
#include "light.glsl"
out vec4 FragColor;
layout (location = 3) in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 FragPosLightSpace;
} fs_in;

uniform vec3 viewPos;
uniform float gamma;

layout (location = 1) uniform sampler2D tDiffuse;
layout (location = 2) uniform sampler2D tSpecular;
uniform float shininess;
uniform samplerCube skybox;
uniform sampler2D shadowMap;

layout (std430, binding = 0) buffer LightBuffer {
	Light lights[];
};

#include "shadow.glsl"

vec3 calcPointLight(Light light)
{
	vec3 lightDir = normalize(vec3(light.position[0], light.position[1], light.position[2]) - fs_in.FragPos);
	float diff = max(dot(fs_in.Normal, lightDir), 0.0);

	vec3 diffuse = vec3(light.diffuse[0],light.diffuse[1],light.diffuse[2]) * diff * texture(tDiffuse, fs_in.TexCoords).rgb;

	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(fs_in.Normal, halfwayDir), 0.0), shininess);
	vec3 specular = vec3(light.specular[0], light.specular[1], light.specular[2]) * spec * texture(tSpecular, fs_in.TexCoords).r;

	if (texture(tSpecular, fs_in.TexCoords).r == 0)
	{
		spec = pow(max(dot(fs_in.Normal, halfwayDir), 0.0), 8);
		specular = vec3(light.specular[0], light.specular[1], light.specular[2]) * spec * diff * vec3(0.25);
	}

		if (texture(tDiffuse, fs_in.TexCoords).r == 0)
	{
			diffuse = vec3(light.diffuse[0],light.diffuse[1],light.diffuse[2]) * diff * vec3(1);
	}

	vec3 ambient = vec3(light.ambient[0], light.ambient[1], light.ambient[2]) * texture(tDiffuse, fs_in.TexCoords).rgb;

	float dist = length(vec3(light.position[0], light.position[1], light.position[2]) - fs_in.FragPos)/1000;
	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);
	return vec3 ((ambient + diffuse + specular ) * light.intensity * attenuation);

}

vec3 calcDirectionalLight(Light light)
{
	vec3 lightDir = normalize(vec3(light.direction[0], light.direction[1], light.direction[2]));
	float diff = max(dot(fs_in.Normal, lightDir), 0.0);

	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(fs_in.Normal, halfwayDir), 0.0), shininess);
	vec3 specular = vec3(light.specular[0], light.specular[1], light.specular[2]) * spec * texture(tSpecular, fs_in.TexCoords).r;

	if (texture(tSpecular, fs_in.TexCoords).r == 0)
	{
		spec = pow(max(dot(fs_in.Normal, halfwayDir), 0.0), 8);
		specular = vec3(light.specular[0], light.specular[1], light.specular[2]) * spec * diff * vec3(0.25);
	}

			vec3 diffuse = vec3(light.diffuse[0],light.diffuse[1],light.diffuse[2]) * diff * texture(tDiffuse, fs_in.TexCoords).rgb;
	if (texture(tDiffuse, fs_in.TexCoords).r == 0)
	{
			diffuse = vec3(light.diffuse[0],light.diffuse[1],light.diffuse[2]) * diff * vec3(1);
	}
	float shadow = ShadowCalculation(fs_in.FragPosLightSpace, vec3(light.position[0], light.position[1], light.position[2]));
	vec3 ambient = vec3(light.ambient[0], light.ambient[1], light.ambient[2]) * texture(tDiffuse, fs_in.TexCoords).rgb;

	return vec3((ambient + (1.0 - shadow) * (diffuse + specular)) * light.intensity);
}

vec3 calcSpotLight(Light light)
{
	vec3 lightDir = normalize(vec3(light.position[0], light.position[1], light.position[2]) - fs_in.FragPos);
	float diff = max(dot(fs_in.Normal, lightDir), 0.0);

	vec3 diffuse = vec3(light.diffuse[0],light.diffuse[1],light.diffuse[2]) * diff * texture(tDiffuse, fs_in.TexCoords).rgb;

	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(fs_in.Normal, halfwayDir), 0.0), shininess);
	vec3 specular = vec3(light.specular[0], light.specular[1], light.specular[2]) * spec * texture(tSpecular, fs_in.TexCoords).r;
	
	float theta     = dot(lightDir, normalize(-vec3(light.direction[0], light.direction[1], light.direction[2])));
	float epsilon   = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0); 

	if (texture(tSpecular, fs_in.TexCoords).r == 0)
	{
		spec = pow(max(dot(fs_in.Normal, halfwayDir), 0.0), 8);
		specular = vec3(light.specular[0], light.specular[1], light.specular[2]) * spec * diff * vec3(0.25);
	}

		if (texture(tDiffuse, fs_in.TexCoords).r == 0)
	{
			diffuse = vec3(light.diffuse[0],light.diffuse[1],light.diffuse[2]) * diff * vec3(1);
	}
	diffuse *= intensity;
	specular *= intensity;

	vec3 ambient = vec3(light.ambient[0], light.ambient[1], light.ambient[2]) * texture(tDiffuse, fs_in.TexCoords).rgb;

	float dist = length(vec3(light.position[0], light.position[1], light.position[2]) - fs_in.FragPos)/1000;
	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);
	float shadow = ShadowCalculation(fs_in.FragPosLightSpace, vec3(light.position[0], light.position[1], light.position[2]));
	vec3 color = texture(tDiffuse, fs_in.TexCoords).rgb;
	if (theta > light.outerCutOff)
	{
		return (ambient + (1.0 - shadow) * (diffuse + specular)) * light.intensity * attenuation;
	}
	else
	{
		return ambient;
	}
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
				break;
			}
			case 1: // Directional light
			{
				result += calcDirectionalLight(currentLight);
				break;
			}
			case 2: // Spot light
			{
				result += calcSpotLight(currentLight);
				break;
			}
		}
	}

	// Apply gamma correction
	result = pow(result, vec3(1.0 / gamma));
	// FragColor = texture(tDiffuse, fs_in.TexCoords);
	// FragColor = vec4(result,1);
	FragColor = vec4(result, 1);
	vec3 projCoords = fs_in.FragPosLightSpace.xyz / fs_in.FragPosLightSpace.w;
	// transform to [0,1] range
	projCoords = projCoords * 0.5 + 0.5;
	// get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	float closestDepth = texture(shadowMap, projCoords.xy).r; 
	// FragColor = vec4(vec3(closestDepth),1);
}