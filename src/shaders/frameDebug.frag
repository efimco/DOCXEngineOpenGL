#version 460 core
layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

layout (location = 0) uniform sampler2D screenTexture;


void main()
{ 

	FragColor      = texture(screenTexture, TexCoords);

}