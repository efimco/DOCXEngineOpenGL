#version 460 core
layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

layout (location = 0) uniform sampler2D screenTexture;
uniform float exposure;


void main()
{ 
	// FragColor = vec4(texture(screenTexture, TexCoords).rgb,1);

	vec3 hdrColor = texture(screenTexture, TexCoords).rgb;
	vec3 mapped   = vec3(1.0) - exp(-hdrColor * exposure);
	mapped         = pow(mapped, vec3(1.0/2.2));
	FragColor      = vec4(mapped, 1.0);

	FragColor = vec4(mapped, 1.0);
	float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
	vec4 BrightColor = vec4(.3);
	if(brightness > 1)
		BrightColor = vec4(FragColor.rgb, 1.0);
	else
		BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
	// FragColor = BrightColor;
}