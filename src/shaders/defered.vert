#version 460 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
	gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
	// If aTexCoords is not filled, set TexCoords to zero
	TexCoords = (aTexCoords == vec2(0.0)) ? vec2(0.0) : aTexCoords;
}