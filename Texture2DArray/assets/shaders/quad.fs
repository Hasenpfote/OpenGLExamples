#version 430
#extension GL_EXT_texture_array : enable

in vec3 fsTCoord;
out vec4 OutColor;

uniform sampler2DArray texture;

void main(void)
{
	//vec2 lod = textureQueryLod(texture, fsTCoord);
	//OutColor.rgb = textureLod(texture, fsTCoord, lod.x).rgb;
	//OutColor.a = 1.0;
	OutColor = texture2DArray(texture, fsTCoord.xyz);
}