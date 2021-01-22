#version 330 core
uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;
uniform sampler2D snowTex;
uniform float scaleFactor;

uniform vec4 lightColour;
uniform vec3 lightPos; 
uniform float lightRadius;

in Vertex{
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
	vec4 shadowProj;
} IN;

out vec4 fragColour;
out vec4 fragNormal;



void main(void){
	mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));

	vec3 normal = texture2D(bumpTex, IN.texCoord).rgb * 2.0 - 1.0;
	normal = normalize(TBN * normalize(normal));

	vec3 snow = texture(snowTex, IN.texCoord).rgb;
	
	fragColour.xyz = texture2D(diffuseTex, IN.texCoord).xyz;

	fragColour.xyz += snow * scaleFactor * 0.01 ; 

	fragColour.w = 1;
	
	fragNormal = vec4(normal.xyz * 0.5 + 0.5, 1);

}