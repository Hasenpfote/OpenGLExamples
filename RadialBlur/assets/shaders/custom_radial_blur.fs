#version 430

out vec4 o_color;

uniform sampler2D u_tex0;
uniform vec2 u_pixel_size;
uniform vec2 u_origin;
uniform vec3 u_params;

#define NUM_SAMPLES 4

vec4 radial_blur_filter(sampler2D tex, vec2 tex_coord, vec2 pixel_size, vec2 origin, float attenuation, int pass)
{
    const float b = pow(NUM_SAMPLES, pass);
	const float aspect = pixel_size.y / pixel_size.x;

    vec2 to_origin = origin - tex_coord;
	vec2 dir = normalize(to_origin * vec2(aspect, 1.0)) * pixel_size;

	// Calculate the distance in unnormalized texture space.
	float dist2 = length(to_origin / pixel_size);

    vec4 color = vec4(0.0);

    for(int s = 0; s < NUM_SAMPLES; s++)
    {
        float d = b * s;
        float weight = (d <= dist2)? pow(attenuation, d) : 0.0;
		vec2 tex_coord_sample = tex_coord + (dir * d);
        vec4 sampling_color = texture(tex, tex_coord_sample);

        color.rgba += weight * sampling_color.rgba;
    }

    return color;
}

void main(void)
{
    vec2 tex_coord = gl_FragCoord.xy * u_pixel_size;
    vec4 color = radial_blur_filter(u_tex0, tex_coord, u_pixel_size, u_origin, u_params.x, int(u_params.y));

	// This fragment shader returns the normalized color only for the final pass.
    float divisor = ((int(u_params.y) + 1) == int(u_params.z))? color.a : 1.0;
    color.rgb /= divisor;

    o_color = color;
}