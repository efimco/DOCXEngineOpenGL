#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;
layout(location = 2) in vec3 aNormal;

layout(location = 0) out VS_OUT
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 currClipPos;
	vec4 prevClipPos;
}
vs_out;

uniform mat4 projection;
uniform mat4 view;

uniform mat4 prevProjection;
uniform mat4 prevView;

uniform mat4 model;
uniform mat4 prevModel;

void main()
{
	// Current world position
	vec4 worldPos = model * vec4(aPos, 1.0);
	vec4 prevWorldPos = prevModel * vec4(aPos, 1.0);
	vs_out.FragPos = worldPos.xyz;
	vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;
	vs_out.TexCoords = aTexCoords;

	vs_out.currClipPos = projection * view * worldPos;
	vs_out.prevClipPos = prevProjection * prevView * prevWorldPos;

	gl_Position = vs_out.currClipPos;
}