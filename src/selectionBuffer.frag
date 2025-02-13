#version 460 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 pickColor;



void main() 
{
	FragColor = vec4(pickColor, 1);
}