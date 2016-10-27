#version 430

uniform sampler2D texture;

in vec2 fsTc0;
in vec4 fsColor;
out vec4 OutColor;

void main(void)
{
	vec2 lod = textureQueryLod(texture, fsTc0);
	OutColor = textureLod(texture, fsTc0, lod.x);
}	
