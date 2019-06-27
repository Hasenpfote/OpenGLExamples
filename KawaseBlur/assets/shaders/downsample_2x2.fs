#version 430

out vec4 OutColor;

uniform sampler2D texture0;
uniform vec2 pixel_size;

void main(void)
{
    vec2 tex_coord = gl_FragCoord.xy * pixel_size;
    OutColor.rgb = texture(texture0, tex_coord).xyz;
    OutColor.a = 1.0;
}