#version 460 core
out vec4 FragColor;

in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 FragPosLightSpace;
} fs_in;

uniform vec3 viewPos;
uniform float gamma;

uniform sampler2D tDiffuse;
uniform sampler2D tSpecular;
uniform float shininess;
uniform samplerCube skybox;
uniform sampler2D shadowMap;

struct Light 
{
	int type;
	float intensity;
	float position[3];
	float direction[3];
	float ambient[3];
	float diffuse[3];
	float specular[3];

	// Attenuation
	float constant;
	float linear;
	float quadratic;

	float cutOff;       // inner cone angle
	float outerCutOff;  // outer cone angle
};

layout (std430, binding = 0) buffer LightBuffer {
	Light lights[];
};


float kernel[9] = float[](
	1.0,	2.0,	1.0,

	2.0,	4.0,	2.0,
	
	1.0,	2.0,	1.0
);

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

float ShadowCalculation(vec4 fragPosLightSpace, vec3 lightPos)
{
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = texture(shadowMap, projCoords.xy).r;

	float currentDepth = projCoords.z;

	vec3 normal = normalize(fs_in.Normal);
	vec3 lightDir = normalize(lightPos - fs_in.FragPos);
	float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.001);

	float shadow = 0.0;

	vec2 texelSize = 1.0 / textureSize(shadowMap,0);
	int sampleSize = 2;
	for (int x = 1-sampleSize; x <=sampleSize; ++x)
	{
		for (int y = 1-sampleSize; y <= sampleSize; ++y)
		{
				float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
				shadow += (currentDepth - bias) >  pcfDepth ? 1.0 : 0.0;
		}
	}
	
	shadow /= pow((sampleSize*2 +1), 2);

	// shadow = (currentDepth - bias) >  closestDepth ? 1.0 : 0.0;
	if(projCoords.z > 1.0)
		shadow = 0.0;
	
		
	return shadow;
}


vec3 calcPointLight(Light light)
	{
	vec3 lightDir = normalize(vec3(light.position[0], light.position[1], light.position[2]) - fs_in.FragPos);
	float diff = max(dot(fs_in.Normal, lightDir), 0.0);

	vec3 diffuse = vec3(light.diffuse[0],light.diffuse[1],light.diffuse[2]) * diff * texture(tDiffuse, fs_in.TexCoords).rgb;

	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(fs_in.Normal, halfwayDir), 0.0), shininess);
	vec3 specular = vec3(light.specular[0], light.specular[1], light.specular[2]) * spec * texture(tSpecular, fs_in.TexCoords).r;

	if (texture(tSpecular, fs_in.TexCoords).r == 0)
	{
		spec = pow(max(dot(fs_in.Normal, halfwayDir), 0.0), 8);
		specular = vec3(light.specular[0], light.specular[1], light.specular[2]) * spec * diff * vec3(0.25);
	}

		if (texture(tDiffuse, fs_in.TexCoords).r == 0)
	{
			diffuse = vec3(light.diffuse[0],light.diffuse[1],light.diffuse[2]) * diff * vec3(1);
	}

	vec3 ambient = vec3(light.ambient[0], light.ambient[1], light.ambient[2]) * texture(tDiffuse, fs_in.TexCoords).rgb;

	float dist = length(vec3(light.position[0], light.position[1], light.position[2]) - fs_in.FragPos)/1000;
	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);
	return vec3 ((ambient + diffuse + specular ) * light.intensity * attenuation);

}

vec3 calcDirectionalLight(Light light)
	{
		vec3 lightDir = normalize(vec3(light.direction[0], light.direction[1], light.direction[2]));
		float diff = max(dot(fs_in.Normal, lightDir), 0.0);

		vec3 viewDir = normalize(viewPos - fs_in.FragPos);
		vec3 halfwayDir = normalize(lightDir + viewDir);
		float spec = pow(max(dot(fs_in.Normal, halfwayDir), 0.0), shininess);
		vec3 specular = vec3(light.specular[0], light.specular[1], light.specular[2]) * spec * texture(tSpecular, fs_in.TexCoords).r;

		if (texture(tSpecular, fs_in.TexCoords).r == 0)
		{
			spec = pow(max(dot(fs_in.Normal, halfwayDir), 0.0), 8);
			specular = vec3(light.specular[0], light.specular[1], light.specular[2]) * spec * diff * vec3(0.25);
		}

				vec3 diffuse = vec3(light.diffuse[0],light.diffuse[1],light.diffuse[2]) * diff * texture(tDiffuse, fs_in.TexCoords).rgb;
		if (texture(tDiffuse, fs_in.TexCoords).r == 0)
		{
				diffuse = vec3(light.diffuse[0],light.diffuse[1],light.diffuse[2]) * diff * vec3(1);
		}
		float shadow = ShadowCalculation(fs_in.FragPosLightSpace, vec3(light.position[0], light.position[1], light.position[2]));
		vec3 ambient = vec3(light.ambient[0], light.ambient[1], light.ambient[2]) * texture(tDiffuse, fs_in.TexCoords).rgb;

		return vec3((ambient + (1.0 - shadow) * (diffuse + specular)) * light.intensity);
	}

vec3 calcSpotLight(Light light)
	{
	vec3 lightDir = normalize(vec3(light.position[0], light.position[1], light.position[2]) - fs_in.FragPos);
	float diff = max(dot(fs_in.Normal, lightDir), 0.0);

	vec3 diffuse = vec3(light.diffuse[0],light.diffuse[1],light.diffuse[2]) * diff * texture(tDiffuse, fs_in.TexCoords).rgb;

	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(fs_in.Normal, halfwayDir), 0.0), shininess);
	vec3 specular = vec3(light.specular[0], light.specular[1], light.specular[2]) * spec * texture(tSpecular, fs_in.TexCoords).r;
	
	float theta     = dot(lightDir, normalize(-vec3(light.direction[0], light.direction[1], light.direction[2])));
	float epsilon   = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0); 

	if (texture(tSpecular, fs_in.TexCoords).r == 0)
	{
		spec = pow(max(dot(fs_in.Normal, halfwayDir), 0.0), 8);
		specular = vec3(light.specular[0], light.specular[1], light.specular[2]) * spec * diff * vec3(0.25);
	}

		if (texture(tDiffuse, fs_in.TexCoords).r == 0)
	{
			diffuse = vec3(light.diffuse[0],light.diffuse[1],light.diffuse[2]) * diff * vec3(1);
	}
	diffuse *= intensity;
	specular *= intensity;

	vec3 ambient = vec3(light.ambient[0], light.ambient[1], light.ambient[2]) * texture(tDiffuse, fs_in.TexCoords).rgb;

	float dist = length(vec3(light.position[0], light.position[1], light.position[2]) - fs_in.FragPos)/1000;
	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);
	float shadow = ShadowCalculation(fs_in.FragPosLightSpace, vec3(light.position[0], light.position[1], light.position[2]));
	vec3 color = texture(tDiffuse, fs_in.TexCoords).rgb;
	if (theta > light.outerCutOff)
	{
		return (ambient + (1.0 - shadow) * (diffuse + specular)) * light.intensity * attenuation;
	}
	else
	{
		return ambient;
	}
	
		
	}

void main() {
	vec3 result = vec3(0.0);
	for (int i = 0; i < lights.length(); i++) 
	{
		Light currentLight = lights[i];
		switch (currentLight.type)
		{
			case 0: // Point light
			{ 
				result += calcPointLight(currentLight);
				break;
			}
			case 1: // Directional light
			{
				result += calcDirectionalLight(currentLight);
				break;
			}
			case 2: // Spot light
			{
				result += calcSpotLight(currentLight);
				break;
			}
		}
	}

	// Apply gamma correction
	result = pow(result, vec3(1.0 / gamma));
	// FragColor = texture(tDiffuse, fs_in.TexCoords);
	// FragColor = vec4(result,1);
	FragColor = vec4(result, 1);
	vec3 projCoords = fs_in.FragPosLightSpace.xyz / fs_in.FragPosLightSpace.w;
	// transform to [0,1] range
	projCoords = projCoords * 0.5 + 0.5;
	// get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	float closestDepth = texture(shadowMap, projCoords.xy).r; 
	// FragColor = vec4(vec3(closestDepth),1);
}