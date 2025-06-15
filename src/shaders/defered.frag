#version 460 core
#include "BRDF.glsl"

layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

layout (binding = 0) uniform sampler2D gAlbedo;
layout (binding = 1) uniform sampler2D gMetallic;
layout (binding = 2) uniform sampler2D gRoughness;
layout (binding = 3) uniform sampler2D gNormal;
layout (binding = 4) uniform sampler2D gPosition;
layout (binding = 5) uniform sampler2D gDepth;

layout (binding = 6) uniform samplerCube irradianceMap;
layout (binding = 7) uniform sampler2D shadowMap;
layout (binding = 8) uniform sampler2D brdfLUT;
layout (binding = 9) uniform samplerCube specularMap;
layout (binding = 10) uniform sampler2D envCubemap;

uniform vec3 viewPos;
uniform float irradianceMapRotationY;
uniform float irradianceMapIntensity;

const float yaw = PI / 180.0f;
const vec2 invAtan = vec2(0.1591, 0.3183); // 1 / (2 * PI), 1 / PI
const float MAX_REFLECTION_LOD = 5.0;
const float MIN_REFLECTANCE = 0.04;

struct GBuffer
{
	vec3 albedo;
	float metallic;
	float roughness;
	vec3 normal;
	vec3 position;
	float depth;
	float albedoAlpha;
};

GBuffer getGBuffer()
{
	GBuffer gBuffer;

	gBuffer.albedo = texture(gAlbedo, TexCoords).rgb;
	gBuffer.metallic = texture(gMetallic, TexCoords).r;
	gBuffer.roughness = texture(gRoughness, TexCoords).r;
	gBuffer.normal = texture(gNormal, TexCoords).rgb;
	gBuffer.position = texture(gPosition, TexCoords).rgb;
	gBuffer.depth = texture(gDepth, TexCoords).r;
	gBuffer.albedoAlpha = texture(gAlbedo, TexCoords).a;
	return gBuffer;
}


mat3 getIrradianceMapRotation() 
{
	float angle = irradianceMapRotationY * yaw;
	float cosA = cos(angle);
	float sinA = sin(angle);
	return mat3(
		cosA, 0.0, -sinA,
		0.0,  1.0,   0.0,
		sinA, 0.0,  cosA
	);
}

vec3 calculateIBL(GBuffer gbuffer, vec3 F0)
{
	mat3 rotationMatrix = getIrradianceMapRotation();
	vec3 viewDir = normalize(viewPos - gbuffer.position);
	vec3 R = reflect(-viewDir, gbuffer.normal);
	
	// Calculate Fresnel
	vec3 F = fresnelSchlickRoughness(
		max(dot(gbuffer.normal, viewDir), 0.0), 
		F0, 
		gbuffer.roughness
	);
	
	// Calculate diffuse and specular factors
	vec3 kS = F;
	vec3 kD = (1.0 - kS) * (1.0 - gbuffer.metallic);
	
	// Diffuse IBL
	vec3 irradiance = texture(irradianceMap, gbuffer.normal * rotationMatrix).rgb;
	vec3 diffuse = gbuffer.albedo * irradiance;
	
	// Specular IBL
	vec3 prefilteredColor = textureLod(
		specularMap, 
		R * rotationMatrix, 
		gbuffer.roughness * MAX_REFLECTION_LOD
	).rgb;
	
	vec2 brdf = texture(brdfLUT, 
		vec2(max(dot(gbuffer.normal, viewDir), 0.0), gbuffer.roughness)
	).rg;
	
	vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
	
	return (kD * diffuse + specular) * irradianceMapIntensity;
}

float linear_rgb_to_srgb(float color)
{
	if (color < 0.0031308f) 
	{
		return (color < 0.0f) ? 0.0f : color * 12.92f;
	}

	return 1.055f * pow(color, 1.0f / 2.4f) - 0.055f;
}

void main()
{

	// Get gBuffer properties
	GBuffer gBuffer = getGBuffer();
	
	// Calculate base reflectivity
	vec3 F0 = mix(vec3(MIN_REFLECTANCE), gBuffer.albedo, gBuffer.metallic);
	
	// Calculate lighting
	vec3 result = calculateIBL(gBuffer, F0);
	
	// Tone mapping

	vec3 tonemapped = vec3(linear_rgb_to_srgb(result.r), linear_rgb_to_srgb(result.g), linear_rgb_to_srgb(result.b));
	vec3 background = texture(envCubemap, TexCoords).rgb;
	bool isBackground = gBuffer.depth >= 1.0 - 1e-5;

	// Use IBL result for geometry, background otherwise
	vec3 finalColor = isBackground ? background : tonemapped;

	// Tone map if needed
	// finalColor = finalColor / (finalColor + vec3(1.0));

	// Output
	FragColor = vec4(finalColor, 1);
	
}