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
	vec4 clipPos = proj * rotView  * vec4(WorldPos, 1.0);
	gl_Position = clipPos.xyww;
}