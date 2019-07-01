#version 430

out vec4 OutColor;

uniform sampler2D texture0;
uniform vec2 pixel_size;

void main(void)
{
    vec2 tex_coord = gl_FragCoord.xy * pixel_size;
    vec3 color = texture(texture0, tex_coord).xyz;

    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    if(luminance > 0.95)
        OutColor.rgb = color;
    else
        OutColor.rgb = vec3(0.0, 0.0, 0.0);

    OutColor.a = 1.0;
}