#version 460 core
layout(location = 0) out vec4 gAlbedo;
layout(location = 1) out float gMetallic;
layout(location = 2) out float gRoughness;
layout(location = 3) out vec3 gNormal;
layout(location = 4) out vec3 gPosition;
layout(location = 5) out vec2 gVelocity;

layout(binding = 1) uniform sampler2D tDiffuse;
layout(binding = 2) uniform sampler2D tSpecular;
layout(binding = 3) uniform sampler2D tNormal;

uniform vec3 uvAlbedo;
uniform float ufRoughness;
uniform float ufMetallic;

uniform vec2 jitter;
uniform vec2 prevJitter;

layout(location = 0) in VS_OUT
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 currClipPos;
	vec4 prevClipPos;
}
fs_in;

struct Material
{
	vec4 albedo;
	float metallic;
	float roughness;
	vec3 normal;
};

vec3 getNormalFromMap()
{
	if(textureSize(tNormal, 0).x <= 1 || textureSize(tNormal, 0).y <= 1)
	{
		return fs_in.Normal; // No normal map, return the original normal
	}
	vec3 tangentNormal = texture(tNormal, fs_in.TexCoords).xyz * 2.0 - 1.0;
	vec3 Q1 = dFdx(fs_in.FragPos);
	vec3 Q2 = dFdy(fs_in.FragPos);
	vec2 st1 = dFdx(fs_in.TexCoords);
	vec2 st2 = dFdy(fs_in.TexCoords);

	vec3 N = normalize(fs_in.Normal);
	vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
	T.y = -T.y;
	vec3 B = -normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);

	return normalize(TBN * tangentNormal);
}

Material getMaterial()
{
	Material material;

	// Normal
	material.normal = getNormalFromMap();
	if(all(equal(material.normal, vec3(0.0))))
	{
		material.normal = fs_in.Normal;
	}

	if(all(lessThanEqual(textureSize(tDiffuse, 0), ivec2(1))))
	{
		material.albedo = pow(vec4(uvAlbedo, 1.0), vec4(2.2));
	}
	else
	{
		material.albedo = pow(texture(tDiffuse, fs_in.TexCoords), vec4(2.2));
	}

	ivec2 specTexSize = textureSize(tSpecular, 0);
	if(all(lessThanEqual(specTexSize, ivec2(1))))
	{
		material.metallic = ufMetallic;
		material.roughness = ufRoughness;
	}
	else
	{
		material.metallic = texture(tSpecular, fs_in.TexCoords).b;
		material.roughness = texture(tSpecular, fs_in.TexCoords).g;
	}

	return material;
}

void main()
{
	Material material = getMaterial();
	gAlbedo = material.albedo;
	gMetallic = material.metallic;
	gRoughness = material.roughness;
	gNormal = material.normal;
	gPosition = fs_in.FragPos;
	vec2 currentPosNDC = fs_in.currClipPos.xy / fs_in.currClipPos.w;
	vec2 previousPosNDC = fs_in.prevClipPos.xy / fs_in.prevClipPos.w;
	vec2 velocity = (currentPosNDC - previousPosNDC);
	velocity = velocity * vec2(0.5, -0.5) + 0.5;
	velocity -= jitter.xy;
	velocity -= prevJitter.xy;
	gVelocity = velocity;

}
