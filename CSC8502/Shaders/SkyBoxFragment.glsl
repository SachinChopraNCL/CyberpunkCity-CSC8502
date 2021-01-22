#version 330 core

uniform samplerCube cubeTex;


in Vertex{
	
	vec3 viewDir;

} IN;

out vec4 fragColour[2];

void main(void){
	fragColour[0] = texture(cubeTex, normalize(IN.viewDir));
	gl_FragDepth = 1;
}