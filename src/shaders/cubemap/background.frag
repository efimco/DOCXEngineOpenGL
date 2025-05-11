#version 460 core
out vec4 FragColor;
in vec3 WorldPos;

layout (location = 0) uniform samplerCube environmentMap;
layout (location = 1) uniform float rotationY;
layout (location = 2) uniform float intensity;

const float yaw = 3.1415f / 180.0f;
mat3 rotationMat = mat3(
	cos(rotationY * yaw), 0.0, -sin(rotationY * yaw),
	0.0, 1.0, 0.0,
	sin(rotationY * yaw), 0.0, cos(rotationY * yaw)
);
void main()
{		
	vec4 envColor = texture(environmentMap, rotationMat *  WorldPos);
	FragColor = vec4(envColor.rgb * intensity, 1.0);
}