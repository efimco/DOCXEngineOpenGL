#version 460 core
layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

layout (binding = 0) uniform sampler2D screenTexture;
layout (binding = 1) uniform sampler2D silhouetteTexture;
uniform vec2 cursorPos;
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
	
	vec2 aspectCorrectedCoords = TexCoords;
	aspectCorrectedCoords.x *= textureSize(screenTexture, 0).x / float(textureSize(screenTexture, 0).y);
	vec2 aspectCorrectedCursor = cursorPos;
	aspectCorrectedCursor.x *= textureSize(screenTexture, 0).x / float(textureSize(screenTexture, 0).y);
	float circleSize = 0.01;
	float dist = distance(aspectCorrectedCoords, aspectCorrectedCursor);

	vec3 silhouetteColor = texture(silhouetteTexture, TexCoords).rgb;
	int w = 3;

	// if the pixel is black (we are on the silhouette)
	if (silhouetteColor == vec3(0.0f))
	{
		vec2 size = 1.0f / textureSize(silhouetteTexture, 0);

		for (int i = -w; i <= +w; i++)
		{
			for (int j = -w; j <= +w; j++)
			{
				if (i == 0 && j == 0)
				{
					continue;
				}

				vec2 offset = vec2(i, j) * size;

				// and if one of the pixel-neighbor is white (we are on the border)
				if (texture(silhouetteTexture, TexCoords + offset).rgb == vec3(1.0f))
				{
					FragColor = vec4(vec3(1.0f), 1.0f);
					return;
				}
			}
		}
	}
	FragColor = vec4(tonemapped, 1.0);
	

}