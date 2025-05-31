#version 460 core
layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

layout (binding = 0) uniform sampler2D screenTexture;
layout (binding = 1) uniform isampler2D pickingTexture;
uniform vec2 cursorPos;

layout(std430, binding = 1) buffer selectedPrimitivesBuffer {
    int selectedPrimitives[];
};

// Add this function to help visualize array contents
vec3 debugArrayValue(int index, vec2 uv) {
	float cellSize = 1.0 / 16.0; // Show 16x16 grid
	int row = index / 16;
	int col = index % 16;
	
	vec2 cellPos = vec2(col, row) * cellSize;
	vec2 innerCell = (uv - cellPos) / cellSize;
	
	if (uv.x >= cellPos.x && uv.x < cellPos.x + cellSize &&
		uv.y >= cellPos.y && uv.y < cellPos.y + cellSize) {
		return selectedPrimitives[index] > 0 ? vec3(1.0, 0.0, 0.0) : vec3(0.0);
	}
	return vec3(0.0);
}

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
	vec3 hdrColor = texture(screenTexture, TexCoords).rgb;
	int pickingColor = texture(pickingTexture, TexCoords).r;

	vec3 tonemapped = vec3(linear_rgb_to_srgb(hdrColor.r), linear_rgb_to_srgb(hdrColor.g), linear_rgb_to_srgb(hdrColor.b));

	vec2 aspectCorrectedCoords = TexCoords;
	aspectCorrectedCoords.x *= textureSize(screenTexture, 0).x / float(textureSize(screenTexture, 0).y);
	vec2 aspectCorrectedCursor = cursorPos;
	aspectCorrectedCursor.x *= textureSize(screenTexture, 0).x / float(textureSize(screenTexture, 0).y);
	float circleSize = 0.01;
	float dist = distance(aspectCorrectedCoords, aspectCorrectedCursor);


	int outlineWidth = 2;

	vec2 pixelSize = 1.0f / textureSize(pickingTexture, 0);

	if (selectedPrimitives[pickingColor] == 1)
	{
		for (int i = -outlineWidth; i <= +outlineWidth; i++)
		{
			for (int j = -outlineWidth; j <= +outlineWidth; j++)
			{
				vec2 offset = vec2(i, j) * pixelSize;
				if (texture(pickingTexture, TexCoords + offset).r != pickingColor)
				{
					FragColor = vec4(1.0, 0.5, 0.0, 1.0);
					return;
				}
			}
		}
	}

		// Add before the final FragColor assignment
	vec3 debugOverlay = debugArrayValue(0, TexCoords);
	for(int i = 1; i < 256; i++) {
		debugOverlay += debugArrayValue(i, TexCoords);
	}
	FragColor = vec4(tonemapped, 1.0);


	// FragColor = vec4(float(int(pickingColor.r * 255.0)) / 255.0, 0.0, 0.0, 1.0);

}