#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec3 normal;
in vec2 texCoords;
in vec3 fragPos;

uniform vec3 lightPos;
uniform vec3 viewPos;

struct Material {
	vec3 ambient;
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	float shininess;
};
uniform Material material;

struct PointLight {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};
#define NR_POINT_LIGHTS 2
uniform PointLight pointLights[NR_POINT_LIGHTS];

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	// diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);
	// specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

	// attenuation
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance +
	light.quadratic * (distance * distance));

	// combine results
	vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, texCoords));

	vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, texCoords));
	vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, texCoords));

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;
	return (ambient + diffuse + specular);
}

void main()
{
	vec3 norm = normalize(normal);
	vec3 viewDir = normalize(viewPos - fragPos);
	
	vec3 result = vec3(0.0f);
	for(int i = 0; i < NR_POINT_LIGHTS; i++){
		result += CalcPointLight(pointLights[i], norm, fragPos, viewDir);
	}
	vec4 texColor = vec4(result, texture(material.texture_diffuse1, texCoords).a);
	if(texColor.a < 0.1){
		discard;
	}
	FragColor = texColor;
	//FragColor = texture(material.texture_diffuse1, texCoords);
}