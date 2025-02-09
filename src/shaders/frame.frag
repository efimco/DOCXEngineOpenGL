#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

const float offset = 1.0 / 300.0;

vec2 offsets[9] = vec2[](
		vec2(-offset,  offset), // top-left
		vec2( 0.0f,    offset), // top-center
		vec2( offset,  offset), // top-right
		vec2(-offset,  0.0f),   // center-left
		vec2( 0.0f,    0.0f),   // center-center
		vec2( offset,  0.0f),   // center-right
		vec2(-offset, -offset), // bottom-left
		vec2( 0.0f,   -offset), // bottom-center
		vec2( offset, -offset)  // bottom-right    
	);


	float kernel[9] = float[](
	1.0,	1.0,	1.0,

	1.0,	-8.0,	1.0,
	
	1.0,	1.0,	1.0
	);

	float kernel2[9] = float[](
		1.0,	2.0,	1.0,

		2.0,	4.0,	2.0,
		
		1.0,	2.0,	1.0
	);
	float kernel1[25] = float[](
	1, 4, 7, 4, 1,
	4, 16, 26, 16, 4,
	7, 26, 41, 26, 7,
	4, 16, 26, 16, 4,
	1, 4, 7, 4, 1
	);
	
void main()
{ 
	FragColor = vec4(texture(screenTexture, TexCoords).rgb,1);
	float average = (FragColor.r + FragColor.g + FragColor.b) / 3.0;
	average = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b;
	// FragColor = vec4(average, average, average, 1.0);

	vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * (kernel[i] / ( (kernel.length() / 2) * (kernel.length() / 2)) ) ;
    
    // FragColor = vec4(col, 1.0);
}