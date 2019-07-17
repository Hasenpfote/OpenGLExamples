#version 430

in vec2 fs_tex_coord;
out vec4 o_color;

uniform sampler2D u_tex0;
uniform float u_exposure;

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

vec3 ACESFilm(vec3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), vec3(0.0), vec3(1.0));
}

void main(void)
{
    vec3 color = texture(u_tex0, fs_tex_coord).rgb;

    vec3 mapped = ACESFilm(u_exposure * color);

    // Linear to sRGB.
    mapped = linear_to_srgb(mapped);

    o_color = vec4(mapped, 1.0);
}