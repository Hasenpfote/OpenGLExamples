#version 430

in vec2 fsTCoord;
in vec4 fsColor;
out vec4 OutColor;

uniform sampler2D texture;

void main(void)
{
	vec2 lod = textureQueryLod(texture, fsTCoord);
#if 0
	OutColor.rgb = textureLod(texture, fsTCoord, lod.x).rgb * fsColor.rgb;
	OutColor.a = fsColor.a;
#else
	vec4 color = textureLod(texture, fsTCoord, lod.x) * fsColor;
	if(color.a < 0.004)
		discard;
	OutColor = color;
#endif
}