#include "rand.glsl"

const float offset = 1.0 / 300.0;

vec2 offsets[9] = vec2[](vec2(-offset, offset), // top-left
vec2(0.0, offset), // top-center
vec2(offset, offset), // top-right
vec2(-offset, 0.0),   // center-left
vec2(0.0, 0.0),   // center-center
vec2(offset, 0.0),   // center-right
vec2(-offset, -offset), // bottom-left
vec2(0.0, -offset), // bottom-center
vec2(offset, -offset)  // bottom-right    
);

vec2 randomOffset(int i, vec2 texelSize)
{
	// Generate more varied offsets using the fragment position
	vec2 noise = vec2(rand(vec2(i * 0.764331, fs_in.FragPos.x)), rand(vec2(fs_in.FragPos.z, i * 0.358318)));

	// Convert to polar coordinates for better distribution
	float angle = noise.x * 2.0 * 3.14159;
	float radius = noise.y;

	// Apply smoothstep for better radius distribution
	radius = smoothstep(0.0, 1.0, radius);

	return vec2(cos(angle), sin(angle)) * radius * texelSize * 4.0; // Increased radius for softer shadows
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 lightPos)
{
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = texture(shadowMap, projCoords.xy).r;

	float currentDepth = projCoords.z;

	vec3 normal = normalize(fs_in.Normal);
	vec3 lightDir = normalize(lightPos - fs_in.FragPos);
	float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.001);

	float shadow = 0.0;

	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	int samples = 16;

	for(int i = 0; i < samples; ++i)
	{
		vec2 offset = randomOffset(i, texelSize);
		float pcfDepth = texture(shadowMap, projCoords.xy + offset).r;
		shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
	}

	shadow /= float(samples) * 2;

	// shadow = (currentDepth - bias) >  closestDepth ? 1.0 : 0.0;
	if(projCoords.z > 1.0)
		shadow = 0.0;

	return shadow;
}
