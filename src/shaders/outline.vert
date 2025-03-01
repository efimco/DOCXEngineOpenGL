#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	vec3 position = aPos;
	position += (aNormal * 0.5);
	gl_Position = projection * view * model * vec4(position, 1.0);  
}