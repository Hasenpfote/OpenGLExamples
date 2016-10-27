#version 430

in vec2 fsTCoord;
out vec4 OutColor;

uniform sampler2D texture;

void main(void)
{
	vec2 lod = textureQueryLod(texture, fsTCoord);
	OutColor.rgb = textureLod(texture, fsTCoord, lod.x).rgb;
	OutColor.a = 1.0;
}