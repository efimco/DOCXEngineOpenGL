#version 460 core
uniform vec3 objectIDColor;  // Unique color for each object
out vec4 FragColor;

void main() 
{
	FragColor = vec4(objectIDColor, 1.0);
}
