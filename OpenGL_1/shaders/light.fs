#version 330 core
out vec4 FragColor;

in vec2 texCoords;

struct Material {
	sampler2D diffuse;
};
uniform Material material;

void main()
{
    FragColor = texture(material.diffuse, texCoords);
}   