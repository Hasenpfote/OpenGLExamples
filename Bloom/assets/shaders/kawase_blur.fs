#version 430

out vec4 OutColor;

uniform sampler2D texture0;
uniform vec2 pixel_size;
uniform vec3 params;

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
    vec2 tex_coord = gl_FragCoord.xy * params.xy;
    vec3 color = kawase_blur_filter(texture0, tex_coord, params.xy, params.z);
    OutColor.rgb = color;
    OutColor.a = 1.0;
}