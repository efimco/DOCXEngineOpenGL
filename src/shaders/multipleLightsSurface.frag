#version 460 core
#extension GL_GOOGLE_include_directive : enable
#include "light.glsl"
out vec4 FragColor;
layout (location = 3) in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 FragPosLightSpace; 
	mat3 TBN;
} fs_in;


uniform vec3 viewPos;
uniform float gamma;

layout (location = 1) uniform sampler2D tDiffuse;
layout (location = 2) uniform sampler2D tSpecular;
layout (location = 3) uniform sampler2D tNormal;
uniform float shininess;
uniform samplerCube skybox;
uniform sampler2D shadowMap;

layout (std140, binding = 0) buffer LightBuffer {
	Light lights[];
};

#include "shadow.glsl"

vec3 calcPointLight(inout Light light)
{
	vec3 normal = texture(tNormal, fs_in.TexCoords).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(fs_in.TBN * normal);  
	if (texture(tNormal, fs_in.TexCoords).r == 0)
	{
		normal = fs_in.Normal;
	}
	vec3 lightDir = normalize(light.position - fs_in.FragPos);
	float diff = max(dot(normal, lightDir), 0.0);

	vec3 diffuse =light.diffuse * diff * texture(tDiffuse, fs_in.TexCoords).rgb;

	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
	vec3 specular =light.specular * spec * texture(tSpecular, fs_in.TexCoords).r;

	if (texture(tSpecular, fs_in.TexCoords).r == 0)
	{
		spec = pow(max(dot(normal, halfwayDir), 0.0), 8);
		specular = light.specular * spec * diff * vec3(0.25);
	}

		if (texture(tDiffuse, fs_in.TexCoords).r == 0)
	{
			diffuse = light.diffuse * diff * vec3(1);
	}

	vec3 ambient = light.ambient * texture(tDiffuse, fs_in.TexCoords).rgb;

	float dist = length(light.position - fs_in.FragPos)/1000;
	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);
	return vec3 ((ambient + diffuse + specular ) * light.intensity * attenuation);

}

vec3 calcDirectionalLight(inout Light light)
{
	vec3 normal = texture(tNormal, fs_in.TexCoords).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(fs_in.TBN * normal); 
	if (texture(tNormal, fs_in.TexCoords).r == 0)
	{
		normal = fs_in.Normal;
	}
	vec3 lightDir = normalize(light.direction);
	float diff = max(dot(normal, lightDir), 0.0);

	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
	vec3 specular = light.specular * spec * texture(tSpecular, fs_in.TexCoords).r;

	if (texture(tSpecular, fs_in.TexCoords).r == 0)
	{
		spec = pow(max(dot(normal, halfwayDir), 0.0), 8);
		specular = light.specular * spec * diff * vec3(0.25);
	}

			vec3 diffuse = light.diffuse * diff * texture(tDiffuse, fs_in.TexCoords).rgb;
	if (texture(tDiffuse, fs_in.TexCoords).r == 0)
	{
			diffuse = light.diffuse * diff * vec3(1);
	}
	float shadow = ShadowCalculation(fs_in.FragPosLightSpace, light.position);
	vec3 ambient = light.ambient * texture(tDiffuse, fs_in.TexCoords).rgb;

	return vec3((ambient + (1.0 - shadow) * (diffuse + specular)) * light.intensity);
}

vec3 calcSpotLight(inout Light light)
{

	vec3 normal = texture(tNormal, fs_in.TexCoords).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(fs_in.TBN * normal);  
	if (texture(tNormal, fs_in.TexCoords).r == 0)
	{
		normal = fs_in.Normal;
	}
	vec3 lightDir = normalize(light.position - fs_in.FragPos);
	float diff = max(dot(normal, lightDir), 0.0);

	vec3 diffuse = light.diffuse * diff * texture(tDiffuse, fs_in.TexCoords).rgb;

	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
	vec3 specular =light.specular * spec * texture(tSpecular, fs_in.TexCoords).r;
	
	float theta     = dot(lightDir, normalize(-vec3(light.direction[0], light.direction[1], light.direction[2])));
	float epsilon   = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0); 

	if (texture(tSpecular, fs_in.TexCoords).r == 0)
	{
		spec = pow(max(dot(normal, halfwayDir), 0.0), 8);
		specular =light.specular * spec * diff * vec3(0.25);
	}

		if (texture(tDiffuse, fs_in.TexCoords).r == 0)
	{
			diffuse = light.diffuse * diff * vec3(1);
	}
	diffuse *= intensity;
	specular *= intensity;

	vec3 ambient = light.ambient * texture(tDiffuse, fs_in.TexCoords).rgb;

	float dist = length(light.position - fs_in.FragPos)/1000;
	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);
	float shadow = ShadowCalculation(fs_in.FragPosLightSpace, light.position);
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

void main() 
{
	vec3 result = vec3(0.0);
	for(int i = 0; i < lights.length(); i++) 
	{
		switch(lights[i].type) {
			case 0: result += calcPointLight(lights[i]); break;
			case 1: result +=  calcDirectionalLight(lights[i]); break;
			case 2: result += calcSpotLight(lights[i]); break;
		}
	}

	// Apply gamma correction
	result = pow(result, vec3(1.0 / gamma));
	// FragColor = texture(tDiffuse, fs_in.TexCoords);
	FragColor = vec4(result, 1.0);
	// FragColor = vec4(vec3(lights[0].position * lights[0].intensity),1.0);
	// FragColor = vec4(vec3(lights.length()),1.0);
	vec3 projCoords = fs_in.FragPosLightSpace.xyz / fs_in.FragPosLightSpace.w;
	// transform to [0,1] range
	projCoords = projCoords * 0.5 + 0.5;
	// get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	float closestDepth = texture(shadowMap, projCoords.xy).r; 
	// FragColor = vec4(vec3(closestDepth),1);
}