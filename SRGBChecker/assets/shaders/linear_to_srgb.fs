#version 430

out vec4 o_color;

uniform sampler2D u_tex0;
uniform vec2 u_pixel_size;

float linear_to_srgb(float u)
{
    if(u <= 0.0031308)
        return 12.92 * u;

    return 1.055 * pow(u, 1.0 / 2.4) - 0.055;
}

vec3 linear_to_srgb(vec3 u)
{
    vec3 lower = 12.92 * u;
    vec3 higher = 1.055 * pow(u, vec3(1.0 / 2.4)) - 0.055;

    return mix(higher, lower, step(u, vec3(0.0031308)));
}

float srgb_to_linear(float u)
{
    if(u <= 0.04045)
        return u / 12.92;

    return pow((u + 0.055) / 1.055, 2.4);
}

vec3 srgb_to_linear(vec3 u)
{
    vec3 lower = u / 12.92;
    vec3 higher = pow((u + 0.055) / 1.055, vec3(2.4));

    return mix(higher, lower, step(u, vec3(0.04045)));
}

void main(void)
{
    vec2 tex_coord = gl_FragCoord.xy * u_pixel_size;

    vec3 color = texture(u_tex0, tex_coord).rgb;
    
    // Linear to sRGB.
    color = linear_to_srgb(color);

    o_color = vec4(color, 1.0);
}