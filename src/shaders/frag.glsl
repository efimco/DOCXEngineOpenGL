#version 460 core
out vec4 FragColor;
in vec2 TexCoord;
in vec3 Normal;

uniform sampler2D texture1;
void main()
{   
    FragColor = vec4(Normal, 1);// texture(texture1, TexCoord);
} 