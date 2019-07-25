#version 430

out vec4 o_color;

uniform vec2 u_pixel_size;

float hash(in vec2 p)
{
    return fract(sin(dot(p.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

float noise(in vec2 x)
{
    vec2 i = floor(x);
    vec2 f = fract(x);

    // Four corners.
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    // Cubic Hermite Curve.
    vec2 u = smoothstep(0.0, 1.0, f);

    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

#define OCTAVES 6
float fbm(in vec2 st)
{
    float value = 0.0;
    float amplitude = .5;
    float frequency = 0.;

    for(int i = 0; i < OCTAVES; i++)
    {
        value += amplitude * noise(st);
        st *= 2.;
        amplitude *= .5;
    }
    return value;
}

void main(void)
{
    vec2 p = gl_FragCoord.xy * u_pixel_size;

    vec3 color = vec3(fbm(p*3.0));

    o_color = vec4(color, 1.0);
}