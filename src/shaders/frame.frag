#version 460 core
layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

layout (location = 0) uniform sampler2D screenTexture;
uniform float exposure;


void main()
{
	vec3 hdrColor = texture(screenTexture, TexCoords).rgb;
	vec3 mapped   = vec3(1.0) - exp(-hdrColor * exposure);
	mapped         = pow(mapped, vec3(1.0/2.2));
	FragColor      = vec4(mapped, 1.0);

	FragColor = vec4(mapped, 1.0);
}