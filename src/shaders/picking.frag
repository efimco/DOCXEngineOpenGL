#version 460 core
uniform int objectID;
layout(location = 0) out int FragColor;

void main()
{
	FragColor = objectID;
}
