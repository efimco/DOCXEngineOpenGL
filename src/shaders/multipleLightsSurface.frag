#version 460 core
#extension GL_ARB_shading_language_include : enable
#include "light.glsl"
#include "BRDF.glsl"

out vec4 FragColor;
layout (location = 3) in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 FragPosLightSpace; 
	mat3 TBN;
} fs_in;

layout (location = 1) uniform sampler2D tDiffuse;
layout (location = 2) uniform sampler2D tSpecular;
layout (location = 3) uniform sampler2D tNormal;
layout (location = 4) uniform samplerCube skybox;
layout (location = 5) uniform sampler2D shadowMap;

layout (location = 6) uniform vec3 viewPos;
layout (location = 7) uniform float gamma;

layout (std140, binding = 0) buffer LightBuffer {
	Light lights[];
};

#include "shadow.glsl"

struct Material {
	vec3 albedo;
	float metallic;
	float roughness;
	vec3 normal;
};

// Function to get material properties
Material getMaterial(vec2 texCoords, mat3 TBN, vec3 defaultNormal) {
	Material material;
	
	// Normal mapping
	material.normal = texture(tNormal, texCoords).rgb;
	material.normal = normalize(material.normal * 2.0 - 1.0);
	material.normal = normalize(TBN * material.normal);
	if (material.normal.r == 0.0 && material.normal.g == 0.0 && material.normal.b == 0.0) {
		material.normal = defaultNormal;
	}
	
	// Albedo
	material.albedo = texture(tDiffuse, texCoords).rgb;

	// PBR properties
	material.metallic = texture(tSpecular, texCoords).b;
	// material.metallic = 0;
	material.roughness = texture(tSpecular, texCoords).g;
	// material.roughness = 0;
	return material;
}

// Refactored directional light calculation
vec3 calcDirectionalLight(inout Light light, Material material) 
{
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 lightDir = normalize(1.0 - light.direction);
	vec3 halfwayDir = normalize(viewDir + lightDir);
	vec3 radiance = light.diffuse * light.intensity;

	float minReflectance = 0.04;

	vec3 F0 = mix(vec3(minReflectance), material.albedo, material.metallic);

	// BRDF components
	float NDF = DistributionGGX(material.normal, halfwayDir, material.roughness);
	float G = GeometrySmith(material.normal, viewDir, lightDir, material.roughness);
	vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.5), F0);

	// Cook-Torrance BRDF

	float NdotL = max(dot(material.normal, lightDir), 0.0);	

	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(material.normal, viewDir), 0.0) * NdotL + 0.001;
	vec3 specular = numerator / denominator;

	vec3 kS = F; //specular reflection
	vec3 kD = (1.0 - kS) * (1.0 - material.metallic);

	vec3 Lo = (kD * material.albedo / PI + specular) * radiance * NdotL;

	// Shadow and ambient
	float shadow = ShadowCalculation(fs_in.FragPosLightSpace, light.position);	
	return (1.0 - shadow) * Lo;
}

// Main function remains mostly the same but simplified
void main() {
	if (texture(tDiffuse, fs_in.TexCoords).a < 0.2) discard;

	Material material = getMaterial(fs_in.TexCoords, fs_in.TBN, fs_in.Normal);


	vec3 result = vec3(0.0);
	for(int i = 0; i < lights.length(); i++) {
		if(lights[i].type == 1) {
			result += calcDirectionalLight(lights[i], material);
		}
	}
	vec3 ambient = vec3(0.03) * material.albedo;
	result += ambient;
	vec3 tex = texture(tDiffuse, fs_in.TexCoords).rgb;
	FragColor = vec4(result, 1.0);
}