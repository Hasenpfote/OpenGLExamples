#version 430

out vec4 o_color;

uniform sampler2D u_tex0;
uniform vec2 u_pixel_size;

void main(void)
{
    vec2 tex_coord = gl_FragCoord.xy * u_pixel_size;

    vec3 color0 = textureOffset(u_tex0, tex_coord, ivec2(-2,  0)).xyz;
    vec3 color1 = textureOffset(u_tex0, tex_coord, ivec2( 2,  0)).xyz;
    vec3 color2 = textureOffset(u_tex0, tex_coord, ivec2( 0, -2)).xyz;
    vec3 color3 = textureOffset(u_tex0, tex_coord, ivec2( 0,  2)).xyz;

    o_color.rgb = (color0 + color1 + color2 + color3) * 0.25;
    o_color.a = 1.0;
}