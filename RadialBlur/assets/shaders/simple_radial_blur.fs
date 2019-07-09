#version 430

out vec4 o_color;

uniform sampler2D u_tex0;
uniform vec2 u_pixel_size;
uniform vec2 u_origin;
uniform float u_attenuation;

#define NUM_SAMPLES 128

vec3 radial_blur_filter(sampler2D tex, vec2 tex_coord, vec2 pixel_size, vec2 origin, float attenuation)
{
    vec3 color = vec3(0.0, 0.0, 0.0);
    vec2 dir = (origin - tex_coord) / float(NUM_SAMPLES);
    
    vec2 tex_coord_sample = tex_coord;
    float total_weight = 0.0;
    
    for(int s = 0; s < NUM_SAMPLES; s++)
    {
        float weight = pow(attenuation, s);
        color += weight * texture(tex, tex_coord_sample).rgb;
        tex_coord_sample += dir;
        total_weight += weight;
    }
    return color / total_weight;
}

void main(void)
{
    vec2 tex_coord = gl_FragCoord.xy * u_pixel_size;
    o_color.rgb = radial_blur_filter(u_tex0, tex_coord, u_pixel_size, u_origin, u_attenuation);
    o_color.a = 1.0;
}