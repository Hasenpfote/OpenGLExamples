#version 430

layout(location = 0) in vec2 vsTCoord;

out gl_PerVertex
{
    vec4 gl_Position;
};

out vec2 tcsTCoord;

uniform float horizontal_scale;
uniform float vertical_scale;
uniform sampler2D height_map;

void main(void)
{
	gl_Position.xz = vsTCoord * horizontal_scale;
	gl_Position.y = texture2D(height_map, vsTCoord).r * vertical_scale;
	gl_Position.w = 1.0;

	tcsTCoord = vsTCoord;
}