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

float linear_rgb_to_srgb(float color)
{
if (color < 0.0031308f) {
	return (color < 0.0f) ? 0.0f : color * 12.92f;
}

return 1.055f * pow(color, 1.0f / 2.4f) - 0.055f;
}

void main()
{
	float exposure = .7;
	vec3 hdrColor = texture(screenTexture, TexCoords).rgb;
	// vec3 tonemapped = ACESFilm(hdrColor * exposure);

	vec3 tonemapped = vec3(linear_rgb_to_srgb(hdrColor.r), linear_rgb_to_srgb(hdrColor.g), linear_rgb_to_srgb(hdrColor.b));

	FragColor = vec4(tonemapped, 1.0);
}