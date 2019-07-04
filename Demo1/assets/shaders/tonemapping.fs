#version 430

in vec2 fs_tex_coord;
out vec4 o_color;

uniform sampler2D u_tex0;
uniform vec2 u_pixel_size;
uniform float u_exposure;


vec3 degamma_correction(vec3 color)
{
    return pow(color, vec3(2.2));
}

vec3 gamma_correction(vec3 color)
{
    return pow(color, vec3(1.0 / 2.2));
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
    vec2 tex_coord = gl_FragCoord.xy * u_pixel_size;
    vec3 color = texture(u_tex0, tex_coord).rgb;
    //color = degamma_correction(color);

    vec3 mapped = ACESFilm(u_exposure * color);

    // gamma correction
    mapped = gamma_correction(mapped);

    o_color = vec4(mapped, 1.0);
}
