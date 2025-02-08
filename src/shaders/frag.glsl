#version 460 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;


uniform vec3 viewPos;
uniform float gamma;

uniform sampler2D tDiffuse;
uniform sampler2D tSpecular;
uniform float shininess;

struct Light
{
	float intensity;
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

layout(std430, binding = 0) buffer LightBuffer 
{
	Light lights[]; 
};

uniform Light light;

void main()
{   
		// store the fragment position vector in the first gbuffer texture
	// gPosition = vec4(FragPos.rgb, 0.0);
	// also store the per-fragment normals into the gbuffer
	// gNormal = normalize(Normal);
	// and the diffuse per-fragment color, ignore specular
	// gAlbedoSpec.rgb = vec3(0.95);
	// ambient
	vec3 ambient = light.ambient * texture(tDiffuse, TexCoords).rgb;

	// diffuse 
	vec3 lightDir = normalize(light.position - FragPos);
	float diff = max(dot(Normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(tDiffuse, TexCoords).rgb; 
	if (texture(tSpecular,TexCoords).r == 0)
	{
	vec3 diffuse = light.diffuse * diff * vec3(0.5); 
	} 
	
	// specular
	vec3 viewDir = normalize( viewPos - FragPos);
	vec3 halfwayDir  = normalize(lightDir + viewDir);   

	float spec = pow(max(dot(Normal, halfwayDir), 0.0),8);
	vec3 specular = light.specular * spec * texture(tSpecular, TexCoords).rgb;
	if (texture(tSpecular,TexCoords).r == 0)
	{
		specular =  light.specular * spec * vec3(0);
	} 
	float distance = length(light.position - FragPos);
	float attenuation = 1/  (distance*distance) ;
	vec3 result = (ambient + diffuse + specular )  * light.intensity * attenuation;
	
	//depth buffer
	float normalizedDepth =gl_FragCoord.z*2 -1;
	float linearDepth = 50 / ((1 - normalizedDepth)*100);
	FragColor = vec4(result,1);
	FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));
	FragColor = vec4(vec3(linearDepth),1);
	// FragColor = vec4(1);
	FragColor = texture(tDiffuse,TexCoords);
	// FragColor = vec4(TexCoords.xy,0,1);
	
} 

