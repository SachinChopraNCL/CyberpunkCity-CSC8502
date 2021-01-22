#version 330 core 

uniform sampler2D diffuseTex;
uniform sampler2D bloomTex;
uniform int drawBloom;


in Vertex {
	vec2 texCoord;
} IN;


out vec4 fragColour;

void main(void){
		if(drawBloom == 1){
			const float gamma = 1;
			float exposure = 1.0f;
			vec3 sceneColour = texture(diffuseTex, IN.texCoord).xyz;
			vec3 bloomColour = texture(bloomTex, IN.texCoord).xyz;
			sceneColour += bloomColour * 2;
	
			vec3 result = vec3(1.0) - exp(-sceneColour * exposure);
			result = pow(result, vec3(1.0 / gamma));
			fragColour = vec4(result, 1.0);
		} else{

			fragColour = vec4(texture(diffuseTex, IN.texCoord).xyz, 1.0);
		}
}