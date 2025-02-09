#version 460 core
out vec4 FragColor;
in vec2 TexCoords;  // Expected to be in the range [0, 1]

void main() 
{
	vec2 center = vec2(0.5, 0.5);
	float dist = distance(TexCoords, center);
	float brightness = 1.0 - mix(0.0, 1.0, dist);
	brightness /= 5.0;
	FragColor = vec4(vec3(brightness), 1.0);
}
