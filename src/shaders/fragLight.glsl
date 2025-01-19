#version 460 core
out vec4 FragColor;


uniform float lightIntensity;
uniform vec3 lightColor;

void main()
{   
    FragColor = vec4(lightColor,1.0f) * lightIntensity;

} 