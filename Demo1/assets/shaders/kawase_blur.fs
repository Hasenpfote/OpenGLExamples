#version 430

out vec4 o_color;

uniform sampler2D u_tex0;
uniform vec2 u_pixel_size;
uniform float u_iteration;

vec3 kawase_blur_filter(sampler2D tex, vec2 tex_coord, vec2 pixel_size, float iteration)
{
    vec2 half_pixel_size = pixel_size / 2.0f;
    vec2 offset = (pixel_size * iteration) + half_pixel_size;
    vec2 tex_coord_sample;
    vec3 color;

    // Sample top left pixel
    tex_coord_sample.x = tex_coord.x - offset.x;
    tex_coord_sample.y = tex_coord.y + offset.y;
    color = texture(tex, tex_coord_sample).xyz;

    // Sample top right pixel
    tex_coord_sample.x = tex_coord.x + offset.x;
    tex_coord_sample.y = tex_coord.y + offset.y;
    color += texture(tex, tex_coord_sample).xyz;
    
    // Sample bottom right pixel
    tex_coord_sample.x = tex_coord.x + offset.x;
    tex_coord_sample.y = tex_coord.y - offset.y;
    color += texture(tex, tex_coord_sample).xyz;
    
    // Sample bottom left pixel
    tex_coord_sample.x = tex_coord.x - offset.x;
    tex_coord_sample.y = tex_coord.y - offset.y;
    color += texture(tex, tex_coord_sample).xyz;

    // Average
    color *= 0.25f;
    
    return color;
}

void main(void)
{
    vec2 tex_coord = gl_FragCoord.xy * u_pixel_size;
    vec3 color = kawase_blur_filter(u_tex0, tex_coord, u_pixel_size, u_iteration);
    o_color.rgb = color;
    o_color.a = 1.0;
}