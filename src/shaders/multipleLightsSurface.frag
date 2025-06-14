#version 460 core
#extension GL_ARB_shading_language_include : enable
#include "light.glsl"
#include "BRDF.glsl"

// Outputs
out vec4 FragColor;

// Inputs from vertex shader
layout (location = 3) in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 FragPosLightSpace; 
} fs_in;

// Lights buffer
layout (std140, binding = 0) buffer LightBuffer {
	Light lights[];
};

// Texture bindings
layout (binding = 1) uniform sampler2D tDiffuse;
layout (binding = 2) uniform sampler2D tSpecular;
layout (binding = 3) uniform sampler2D tNormal;
layout (binding = 4) uniform samplerCube irradianceMap;
layout (binding = 5) uniform sampler2D shadowMap;
layout (binding = 6) uniform sampler2D brdfLUT;
layout (binding = 7) uniform samplerCube specularMap;

// Material uniforms
uniform float ufRoughness;
uniform float ufMetallic;

// Camera and environment uniforms
uniform vec3 viewPos;
uniform float irradianceMapRotationY;
uniform float irradianceMapIntensity;

// Constants
const float yaw = PI / 180.0f;
const vec2 invAtan = vec2(0.1591, 0.3183); // 1 / (2 * PI), 1 / PI
const float MAX_REFLECTION_LOD = 5.0;
const float MIN_REFLECTANCE = 0.04;

// Material structure
struct Material {
	vec3 albedo;
	float metallic;
	float roughness;
	vec3 normal;
};

// Calculate rotation matrix for environment mapping
mat3 getIrradianceMapRotation() {
	float angle = irradianceMapRotationY * yaw;
	float cosA = cos(angle);
	float sinA = sin(angle);
	return mat3(
		cosA, 0.0, -sinA,
		0.0,  1.0,   0.0,
		sinA, 0.0,  cosA
	);
}

// Normal mapping calculation
vec3 getNormalFromMap() {
	vec3 tangentNormal = texture(tNormal, fs_in.TexCoords).xyz * 2.0 - 1.0;
	vec3 Q1 = dFdx(fs_in.FragPos);
	vec3 Q2 = dFdy(fs_in.FragPos);
	vec2 st1 = dFdx(fs_in.TexCoords);
	vec2 st2 = dFdy(fs_in.TexCoords);

	vec3 N = normalize(fs_in.Normal);
	vec3 T = normalize(Q1*st2.t - Q2*st1.t);
	T.y = -T.y;
	vec3 B = -normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);
	
	return normalize(TBN * tangentNormal);
}

// Get material properties from textures
Material getMaterial() {
	Material material;
	
	// Normal
	material.normal = getNormalFromMap();
	if (all(equal(material.normal, vec3(0.0)))) {
		material.normal = fs_in.Normal;
	}
	
	// Albedo with gamma correction
	material.albedo = pow(texture(tDiffuse, fs_in.TexCoords).rgb, vec3(2.2));
	
	// Metallic and roughness
	ivec2 specTexSize = textureSize(tSpecular, 0);
	if (all(lessThanEqual(specTexSize, ivec2(1)))) {
		material.metallic = ufMetallic;
		material.roughness = ufRoughness;
	} else {
		material.metallic = texture(tSpecular, fs_in.TexCoords).b;
		material.roughness = texture(tSpecular, fs_in.TexCoords).g;
	}
	
	// Default albedo for missing texture
	if (all(lessThanEqual(textureSize(tDiffuse, 0), ivec2(1)))) {
		material.albedo = vec3(1.0, 1.0, 1.0);
	}
	
	return material;
}

// Calculate IBL (Image Based Lighting)
vec3 calculateIBL(Material material, vec3 F0) {
	mat3 rotationMatrix = getIrradianceMapRotation();
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 R = reflect(-viewDir, material.normal);
	
	// Calculate Fresnel
	vec3 F = fresnelSchlickRoughness(
		max(dot(material.normal, viewDir), 0.0), 
		F0, 
		material.roughness
	);
	
	// Calculate diffuse and specular factors
	vec3 kS = F;
	vec3 kD = (1.0 - kS) * (1.0 - material.metallic);
	
	// Diffuse IBL
	vec3 irradiance = texture(irradianceMap, material.normal * rotationMatrix).rgb;
	vec3 diffuse = material.albedo * irradiance;
	
	// Specular IBL
	vec3 prefilteredColor = textureLod(
		specularMap, 
		R * rotationMatrix, 
		material.roughness * MAX_REFLECTION_LOD
	).rgb;
	
	vec2 brdf = texture(brdfLUT, 
		vec2(max(dot(material.normal, viewDir), 0.0), material.roughness)
	).rg;
	
	vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
	
	return (kD * diffuse + specular) * irradianceMapIntensity;
}

void main() {
	// Early discard for transparent pixels
	if (texture(tDiffuse, fs_in.TexCoords).a < 0.2) discard;

	// Get material properties
	Material material = getMaterial();
	
	// Calculate base reflectivity
	vec3 F0 = mix(vec3(MIN_REFLECTANCE), material.albedo, material.metallic);
	
	// Calculate lighting
	vec3 result = calculateIBL(material, F0);
	
	// Tone mapping
	result = result / (result + vec3(1.0));
	
	FragColor = vec4(result, 1.0);
}