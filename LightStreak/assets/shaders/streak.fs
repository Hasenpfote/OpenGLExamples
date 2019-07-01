#version 430

out vec4 o_color;

uniform sampler2D u_tex0;
uniform vec2 u_pixel_size;
uniform vec2 u_direction;
uniform vec2 u_params;

#define NUM_SAMPLES 4

vec3 steerable_streak_filter(sampler2D tex, vec2 tex_coord, vec2 pixel_size, vec2 dir, float attenuation, int pass)
{
    vec2 tex_coord_sample;
    vec3 color = vec3(0.0, 0.0, 0.0);
    float b = pow(NUM_SAMPLES, pass);
    for(int s = 0; s < NUM_SAMPLES; s++)
    {
        float weight = clamp(pow(attenuation, b * s), 0.0, 1.0);
        tex_coord_sample = tex_coord + (dir * b * s * pixel_size);
        color += weight * texture(tex, tex_coord_sample).xyz;
    }
    return clamp(color, vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0));
}

void main(void)
{
    vec2 tex_coord = gl_FragCoord.xy * u_pixel_size.xy;
    vec3 color = steerable_streak_filter(u_tex0, tex_coord, u_pixel_size, u_direction, u_params.x, int(u_params.y));
    o_color.rgb = color;
    o_color.a = 1.0;
}