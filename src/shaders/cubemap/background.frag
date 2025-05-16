#version 460 core
out vec4 FragColor;
in vec3 WorldPos;

layout (binding = 0) uniform samplerCube environmentMap;
uniform float rotationY;
uniform float intensity;

const float yaw = 3.1415f / 180.0f;
mat3 rotationMat = mat3(
	cos(rotationY * yaw), 0.0, -sin(rotationY * yaw),
	0.0, 1.0, 0.0,
	sin(rotationY * yaw), 0.0, cos(rotationY * yaw)
);
void main()
{		
	vec3 envColor = texture(environmentMap, WorldPos, 1.2).rgb; 
	// vec3 envColor = textureLod(environmentMap, WorldPos, 5).rgb;
	FragColor = vec4(envColor.rgb * intensity, 1.0);
	// FragColor = vec4(vec3(1),1);
}