#version 430

out vec4 o_color;

uniform sampler2D u_tex0;
uniform vec2 u_src_resolution;
uniform vec2 u_dst_resolution;

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
    float adr1d = address(gl_FragCoord.xy, u_dst_resolution);
    vec2 adr2d = address(adr1d, u_src_resolution);

    vec3 color = texture(u_tex0, adr2d / u_src_resolution).rgb;

    o_color = vec4(color, 1.0);
}