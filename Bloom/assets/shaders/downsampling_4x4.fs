#version 430

out vec4 OutColor;

uniform sampler2D texture0;
uniform vec2 pixel_size;

void main(void)
{
    vec2 tex_coord = gl_FragCoord.xy * pixel_size;

    vec3 color0 = textureOffset(texture0, tex_coord, ivec2(-2,  0)).xyz;
    vec3 color1 = textureOffset(texture0, tex_coord, ivec2( 2,  0)).xyz;
    vec3 color2 = textureOffset(texture0, tex_coord, ivec2( 0, -2)).xyz;
    vec3 color3 = textureOffset(texture0, tex_coord, ivec2( 0,  2)).xyz;

    OutColor.rgb = (color0 + color1 + color2 + color3) * 0.25;
    OutColor.a = 1.0;
}