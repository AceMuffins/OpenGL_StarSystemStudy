#version 330 core
out vec4 FragColor;

in vec3 normal;
in vec2 texCoords;
in vec3 fragPos;

struct Material {
	vec3 ambient;
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};
uniform Material material;

void main()
{
    FragColor = texture(material.diffuse, texCoords);
}   