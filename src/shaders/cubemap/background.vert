#version 460 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 WorldPos;

void main()
{
	WorldPos = aPos;
	mat4 rotView = mat4(mat3(view));
	mat4 proj = projection;

	float zNear = 1.1f; // Near plane distance
	float zFar = 100.0f; // Far plane distance

	proj[2][2] = - (zFar + zNear) / (zFar - zNear);
	proj[3][2] = - (1 * zFar * zNear) / (zFar - zNear);
	vec4 clipPos = proj * rotView  * vec4(WorldPos, 1.0);
	gl_Position = clipPos.xyww;
}