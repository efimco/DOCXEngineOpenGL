#version 460 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;


uniform vec3 viewPos;

struct Material 
{
    sampler2D tDiffuse1;
    sampler2D tSpecular1;
    float shininess;

};

struct Light
{
    float intensity;
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Light light;
uniform Material material;

void main()
{   
    // ambient
    vec3 ambient = light.ambient * texture(material.tDiffuse1, TexCoords).rgb;

    // diffuse 
    //vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(Normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.tDiffuse1, TexCoords).rgb;  
    
    // specular
    vec3 viewDir = normalize( viewPos - FragPos);
    vec3 halfwayDir  = normalize(lightDir + viewDir);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec  * texture(material.tSpecular1, TexCoords).rgb;  
    float distance = length(light.position - FragPos);
    float attenuation = 1/ (1 + 0.9 * distance +  0.032 * (distance*distance) );
    vec3 result = (ambient + diffuse + specular ) * attenuation * light.intensity;
    FragColor = vec4(result, 1.0);
    
} 

