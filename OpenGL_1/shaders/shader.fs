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
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};
uniform Material material;

struct Light {
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};
uniform Light light;

void main()
{
	//FragColor = texture(texture1, texCoord) ;
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, texCoords));

	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(lightPos - fragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, texCoords));

	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * vec3(texture(material.specular, texCoords));

	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;
	vec4 texColor = vec4(ambient + diffuse + specular, texture(material.diffuse, texCoords).a);
	if(texColor.a < 0.1)
		discard;
	FragColor = texColor;
	//FragColor = texture(material.diffuse, texCoords);
}