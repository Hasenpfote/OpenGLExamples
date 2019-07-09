#version 430

out vec4 o_color;

uniform sampler2D u_tex0;
uniform sampler2D u_tex1;
uniform vec2 u_pixel_size;
uniform float u_dimension;
uniform int u_mode;


float get_luminance(vec3 color)
{
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

void main(void)
{
    vec2 tex_coord0 = mod(gl_FragCoord.xy, u_dimension) / u_dimension;
    float threshold = texture(u_tex0, tex_coord0).r;

    vec2 tex_coord1 = gl_FragCoord.xy * u_pixel_size;
    vec3 color = texture(u_tex1, tex_coord1).rgb;

    if(u_mode == 0)
    {
        o_color.rgb = step(threshold, color);
    }
    else
    {
        float luminance = get_luminance(color);
        o_color.rgb = step(threshold, vec3(luminance));
    }
    o_color.a = 1.0;
}