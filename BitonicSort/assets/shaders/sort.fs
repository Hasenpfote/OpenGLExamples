#version 430

out vec4 o_color;

uniform sampler2D u_tex0;
uniform vec2 u_resolution;
uniform float u_seq_size;
uniform float u_offset;
uniform float u_range;

float address(vec2 address2d, vec2 resolution)
{
    vec2 temp = floor(address2d);
    return temp.y * resolution.x + temp.x;
}

vec2 address(float address1d, vec2 resolution)
{
    vec2 temp;
    temp.x = mod(address1d, resolution.x);
    temp.y = address1d / resolution.x;
    return floor(temp) + 0.5;
}

void main(void)
{
    float own_adr1d = address(gl_FragCoord.xy, u_resolution);

    float fetch_dir = (mod(own_adr1d, u_range) < u_offset)? 1 : -1;
    float sort_dir = (mod(floor(own_adr1d / u_seq_size), 2.0) <= 0.5)? 1 : -1;

    float partner_adr1d = own_adr1d + fetch_dir * u_offset;
    vec2 partner_adr2d = address(partner_adr1d, u_resolution);

    vec3 own_color = texture(u_tex0, gl_FragCoord.xy / u_resolution).rgb;
    vec3 partner_color = texture(u_tex0, partner_adr2d / u_resolution).rgb;

    vec3 min_color = (own_color.x < partner_color.x)? own_color : partner_color;
    vec3 max_color = (own_color.x < partner_color.x)? partner_color : own_color;

    vec3 color = (fetch_dir == sort_dir)? min_color : max_color;

    o_color = vec4(color, 1.0);
}