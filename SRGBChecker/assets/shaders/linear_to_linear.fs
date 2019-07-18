#version 430

out vec4 o_color;

uniform sampler2D u_tex0;
uniform vec2 u_pixel_size;

void main(void)
{
    vec2 tex_coord = gl_FragCoord.xy * u_pixel_size;
    o_color.rgb = texture(u_tex0, tex_coord).rgb;
    o_color.a = 1.0;
}