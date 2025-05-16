#version 460 core
layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

layout (location = 0) uniform sampler2D screenTexture;
// uniform float exposure;

vec3 ACESFilm(vec3 x) {
	// an approximation to the RRT+ODT fit from ACES
	const float a = 2.51;
	const float b = 0.03;
	const float c = 2.43;
	const float d = 0.59;
	const float e = 0.14;
	return clamp((x*(a*x + b)) / (x*(c*x + d) + e), 0.0, 1.0);
}


void main()
{
	float exposure = .8;
	vec3 hdrColor = texture(screenTexture, TexCoords).rgb;
	vec3 tonemapped = ACESFilm(hdrColor * exposure);
	vec3 srgb = pow(tonemapped, vec3(1.0/2.2));

	FragColor = vec4(srgb, 1.0);
}