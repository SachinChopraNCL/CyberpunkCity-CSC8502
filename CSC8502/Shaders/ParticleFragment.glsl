#version 330 core
in vec2 texCoords;
in vec4 particleColour;

uniform sampler2D diffuseTex;

out vec4 colour;

void main()
{
	colour = (texture(diffuseTex, texCoords) * particleColour);
}