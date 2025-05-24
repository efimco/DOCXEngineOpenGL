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
} fs_in;

layout (std140, binding = 0) buffer LightBuffer {
	Light lights[];
};

layout (binding = 1) uniform sampler2D tDiffuse;
layout (binding = 2) uniform sampler2D tSpecular;
layout (binding = 3) uniform sampler2D tNormal;
layout (binding = 4) uniform samplerCube irradianceMap;
layout (binding = 5) uniform sampler2D shadowMap;
layout (binding = 6) uniform sampler2D brdfLUT;
layout (binding = 7) uniform samplerCube specularMap; 

uniform float ufRoughness;
uniform float ufMetallic;

uniform vec3 viewPos;
uniform float irradianceMapRotationY;
uniform float irradianceMapIntensity;


const float yaw = 3.1415f / 180.0f;
mat3 irradianceMapYawRotation = mat3(
	cos(irradianceMapRotationY * yaw), 0.0, -sin(irradianceMapRotationY * yaw),
	0.0, 1.0, 0.0,
	sin(irradianceMapRotationY * yaw), 0.0, cos(irradianceMapRotationY * yaw)
);

#include "shadow.glsl"

struct Material {
	vec3 albedo;
	float metallic;
	float roughness;
	vec3 normal;
};

vec3 getNormalFromMap()
{
	vec3 tangentNormal = texture(tNormal, fs_in.TexCoords).xyz * 2.0 - 1.0;
	vec3 Q1  = dFdx(fs_in.FragPos);
	vec3 Q2  = dFdy(fs_in.FragPos);
	vec2 st1 = dFdx(fs_in.TexCoords);
	vec2 st2 = dFdy(fs_in.TexCoords);

	vec3 N   = normalize(fs_in.Normal);
	vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
	T.y = -T.y;
	vec3 B  = -normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);
	return normalize(TBN * tangentNormal);
}

// Function to get material properties
Material getMaterial(vec2 texCoords, vec3 defaultNormal) {
	Material material;

	// Normal mapping

	material.normal = getNormalFromMap();
	if (material.normal.r == 0.0 && material.normal.g == 0.0 && material.normal.b == 0.0) {
		material.normal = defaultNormal;
	}
	// material.normal.y = -material.normal.y;
	material.albedo = texture(tDiffuse, texCoords).rgb;
	material.albedo = pow(material.albedo, vec3(2.2));

	material.metallic = texture(tSpecular, texCoords).b  ;
	material.roughness = texture(tSpecular, texCoords).g ;

	ivec2 specTexSize = textureSize(tSpecular, 0);
	if (length(specTexSize) <= length(ivec2(1,1)))
	{
		material.metallic = ufMetallic;
		material.roughness = ufRoughness;
	}
	
	ivec2 diffuseTexSize = textureSize(tDiffuse, 0);
		if (length(specTexSize) <= length(ivec2(1,1)))
	{
		material.albedo = vec3(1.0, 0.0, 1.0);
	}
	
	return material;
}

const vec2 invAtan = vec2(0.1591, 0.3183); // 1 / (2 * PI), 1 / PI
vec2 SampleSphericalCoords(vec3 v)
{
	vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
	uv *= invAtan;
	uv += 0.5;
	return uv;
}

vec3 calcDirectionalLight(inout Light light, Material material, vec3 F0) 
{
	return vec3(0);
}

// Main function remains mostly the same but simplified
void main() {
	if (texture(tDiffuse, fs_in.TexCoords).a < 0.2) discard;

	Material material = getMaterial(fs_in.TexCoords, fs_in.Normal);
	

	float minReflectance = 0.04;
	vec3 F0 = mix(vec3(minReflectance), material.albedo, material.metallic);
	vec3 result = vec3(0.0);
	// for(int i = 0; i < lights.length(); i++) {
	// 	if(lights[i].type == 1) {
	// 		result += calcDirectionalLight(lights[i], material, F0);
	// 	}
	// }

	// Calculate IBL ambient lighting
	vec3 irradiance = texture(irradianceMap, material.normal * irradianceMapYawRotation).rgb;
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 F = fresnelSchlickRoughness(max(dot(material.normal, viewDir), 0.0), F0, material.roughness);
	vec3 kS = F;
	vec3 kD = (1.0 - kS);
	kD *= 1.0 -  material.metallic;
	
	// Add image-based ambient lighting
	vec3 diffuse = material.albedo * irradiance * irradianceMapIntensity;
	
	const float MAX_REFLECTION_LOD = 5.0;
	vec3 R = reflect(-viewDir, material.normal);
	vec3 prefilteredColor = textureLod(specularMap, R * irradianceMapYawRotation,  material.roughness * MAX_REFLECTION_LOD).rgb;    
	vec2 brdf  = texture(brdfLUT, vec2(max(dot(material.normal, viewDir), 0.0), material.roughness)).rg;
	vec3 specular = prefilteredColor * (F * brdf.x + brdf.y) * irradianceMapIntensity * 1;
	
	vec3 ambient = kD * diffuse + specular;
	result += ambient;
	result = result / (result + vec3(1.0));
	FragColor = vec4(result, 1.0);
}