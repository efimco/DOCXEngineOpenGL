#version 460 core
out vec4 FragColor;
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture1;
uniform vec3 lightColor;
uniform vec3 lightPos;
void main()
{   
    vec3 normal = normalize(Normal);

    vec3 lightDir = normalize(lightPos - FragPos);

    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    float diff = max(dot(lightDir,normal),0);
    vec3 diffuse = diff * lightColor;

    vec3 result = (ambient + diffuse);

    vec4 texColor = texture(texture1, TexCoord) * vec4(result,1);

    FragColor = texColor;
} 