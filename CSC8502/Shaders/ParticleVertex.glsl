#version 330 core 

in vec3 position;
in vec2 texCoord;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix; 
uniform mat4 projMatrix; 
uniform vec3 offset;
uniform vec4 colour;

out vec2 texCoords;
out vec4 particleColour;

void main(){
	float scale = 10.0f;
	texCoords = texCoord;
	particleColour = colour; 
	gl_Position = projMatrix * vec4((position * scale) + offset, 1.0);
}