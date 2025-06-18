#version 460 core
#include "BRDF.glsl"

out vec2 FragColor;
in vec2 TexCoords;

// ----------------------------------------------------------------------------
void main() 
{
	vec2 integratedBRDF = IntegrateBRDF(TexCoords.x, TexCoords.y);
	FragColor = integratedBRDF;
}