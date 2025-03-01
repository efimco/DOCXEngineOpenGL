#version 460 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
in vec3 normalMatrix;

uniform vec3 viewPos;
uniform float gamma;

const float PI = 3.14159265359;

uniform sampler2D  tDiffuse;
uniform sampler2D  tSpecular;
uniform sampler2D  tRoughness;
uniform sampler2D  tMetallic;
uniform sampler2D  tNormal;
uniform sampler2D  tAO;
uniform float shininess;

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

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a      = roughness*roughness;
	float a2     = a*a;
	float NdotH  = max(dot(N, H), 0.0);
	float NdotH2 = NdotH*NdotH;
	
	float num   = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = 3.141592 * denom * denom;
	
	return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;

	float num   = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	
	return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2  = GeometrySchlickGGX(NdotV, roughness);
	float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
	return ggx1 * ggx2;
}

vec3 getNormalFromMap()
{
	vec3 tangentNormal = texture(tNormal, TexCoords).xyz * 2.0 - 1.0;

	vec3 Q1  = dFdx(FragPos);
	vec3 Q2  = dFdy(FragPos);
	vec2 st1 = dFdx(TexCoords);
	vec2 st2 = dFdy(TexCoords);

	vec3 N   = normalize(Normal);
	vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
	vec3 B  = -normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);

	return normalize(TBN * tangentNormal);
}

void main() 
{

	vec3 albedo     = pow(texture(tDiffuse, TexCoords).rgb, vec3(2.2));
	float metallic  = texture(tSpecular, TexCoords).r;
	float roughness = texture(tSpecular, TexCoords).g;
	float ao        = texture(tAO, TexCoords).r;
	
	vec3 N = getNormalFromMap();
	vec3 V = normalize(viewPos - FragPos);
	vec3 R = reflect(-V, N); 

	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo, metallic);

	// reflectance equation
	vec3 Lo = vec3(0.0);
	for (int i = 0; i < lights.length(); i++) 
	{
		Light currentLight = lights[i];
		if (currentLight.type == 0) 
		{ // Point light
			vec3 L = normalize(vec3(currentLight.position[0], currentLight.position[1], currentLight.position[2]) - FragPos);
			vec3 H = normalize(V + L);
			float distance    = length(vec3(currentLight.position[0], currentLight.position[1], currentLight.position[2]) - FragPos);
			float attenuation = 1.0 / (distance * distance);
			vec3 radiance     = vec3(currentLight.diffuse[0], currentLight.diffuse[1], currentLight.diffuse[2]) * attenuation * currentLight.intensity;        
			
			// cook-torrance brdf
			float NDF = DistributionGGX(N, H, roughness);
			float G   = GeometrySmith(N, V, L, roughness);
			vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
			

			
			vec3 numerator    = NDF * G * F;
			float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
			vec3 specular     = numerator / denominator;  
				
			vec3 kS = F;
			vec3 kD = vec3(1.0) - kS;
			kD *= 1.0 - metallic;
			// add to outgoing radiance Lo
			float NdotL = max(dot(N, L), 0.0);                
			Lo += (kD * albedo / PI + specular) * radiance * NdotL; 
		}
		else if (currentLight.type == 1)
		{
			vec3 L = normalize(vec3(currentLight.direction[0], currentLight.direction[1], currentLight.direction[2]) - FragPos);
			vec3 H = normalize(V + L);
			vec3 radiance     = vec3(currentLight.diffuse[0],currentLight.diffuse[1],currentLight.diffuse[2]) * currentLight.intensity;        
			
			// cook-torrance brdf
			float NDF = DistributionGGX(N, H, roughness);        
			float G   = GeometrySmith(N, V, L, roughness);      
			vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
			
			vec3 kS = F;
			vec3 kD = vec3(1.0) - kS;
			kD *= 1.0 - metallic;
			
			vec3 numerator    = NDF * G * F;
			float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
			vec3 specular     = numerator / denominator;  
				
			// add to outgoing radiance Lo
			float NdotL = max(dot(N, L), 0.0);                
			Lo += (kD * albedo / PI + specular) * radiance * NdotL; 
		}
				else if (currentLight.type == 2)
		{ // Spotlight
			vec3 lightPos = vec3(currentLight.position[0], currentLight.position[1], currentLight.position[2]);
			vec3 L = normalize(lightPos - FragPos);
			// Calculate the cosine of the angle between the light direction and the direction from the light to the fragment.
			float theta = dot(L, normalize(-vec3(currentLight.direction[0], currentLight.direction[1], currentLight.direction[2])));
			// Use the inner and outer cone angles to create a smooth edge.
			float epsilon = currentLight.cutOff *20 - currentLight.outerCutOff;
			float spotIntensity = clamp((theta - currentLight.outerCutOff) / epsilon, 0.0, 1.0);

			float distance    = length(lightPos - FragPos);
			float attenuation = 1.0 / (currentLight.constant + currentLight.linear * distance +
									   currentLight.quadratic * (distance * distance));
			vec3 radiance     = vec3(currentLight.diffuse[0], currentLight.diffuse[1], currentLight.diffuse[2])
								* attenuation * currentLight.intensity;        
			
			vec3 H = normalize(V + L);
			float NDF = DistributionGGX(N, H, roughness);        
			float G   = GeometrySmith(N, V, L, roughness);      
			vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
			
			vec3 kS = F;
			vec3 kD = vec3(1.0) - kS;
			kD *= 1.0 - metallic;
			
			vec3 numerator    = NDF * G * F;
			float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
			vec3 specular     = numerator / denominator;  
				
			float NdotL = max(dot(N, L), 0.0);                
			// Modulate the radiance with the spotlight intensity factor.
			radiance *= spotIntensity;
			Lo += (kD * albedo / PI + specular) * radiance * NdotL;
		}

	}
	
    vec3 ambient = vec3(0.03) * albedo * ao;
    
    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 	

    FragColor = vec4(color, 1.0);
}
