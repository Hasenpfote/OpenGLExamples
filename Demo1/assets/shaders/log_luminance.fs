#version 430

out float o_color;

uniform sampler2D u_tex0;
uniform vec2 u_pixel_size;

float get_luminance(vec3 color)
{
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

void main(void)
{
    vec2 tex_coord = gl_FragCoord.xy * u_pixel_size;

    vec3 color = texture(u_tex0, tex_coord).rgb;
    float luminance = get_luminance(color);
    o_color = log(max(luminance, 0.001));
}