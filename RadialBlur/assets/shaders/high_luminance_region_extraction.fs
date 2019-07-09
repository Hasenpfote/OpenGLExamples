#version 430

out vec4 o_color;

uniform sampler2D u_tex0;
uniform vec2 u_pixel_size;
uniform float u_exposure;
uniform float u_threshold;
uniform float u_soft_threshold;


float get_luminance(vec3 color)
{
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

vec3 prefilter(vec3 color, float threshold, float soft_threshold)
{
    // 0 <= soft_threshold <= 1
    float luminance = get_luminance(color);
    float knee = threshold * soft_threshold;
    float soft = luminance - threshold + knee;
    soft = clamp(soft, 0.0, 2.0 * knee);
    soft = soft * soft / (4.0 * knee + 0.00001);
    float contribution = max(soft, luminance - threshold);
    contribution /= max(luminance, 0.00001);
    return color * contribution;
}

void main(void)
{
    vec2 tex_coord = gl_FragCoord.xy * u_pixel_size;

#if 0
    vec3 color = texture(u_tex0, tex_coord).rgb;
#else
    // 1/4 x 1/4 downsampling.
    vec3 color0 = textureOffset(u_tex0, tex_coord, ivec2(-2,  0)).rgb;
    vec3 color1 = textureOffset(u_tex0, tex_coord, ivec2( 2,  0)).rgb;
    vec3 color2 = textureOffset(u_tex0, tex_coord, ivec2( 0, -2)).rgb;
    vec3 color3 = textureOffset(u_tex0, tex_coord, ivec2( 0,  2)).rgb;

    vec3 color = (color0 + color1 + color2 + color3) * 0.25;
#endif

    o_color.rgb = prefilter(u_exposure * color, u_threshold, u_soft_threshold);
    o_color.a = 1.0;
}