#version 430

in vec2 fsTCoord;
out vec4 OutColor;

uniform float vertical_scale;
uniform sampler2D diffuse_map;
uniform sampler2D height_map;
uniform vec3 light_direction;

#define ENABLE_MIPMAP

#ifdef ENABLE_MIPMAP
vec3 compute_normal(const in vec2 tc)
{
	vec2 lod = textureQueryLod(height_map, tc);
	float w = textureLodOffset(height_map, tc, lod.x, ivec2(-1, 0)).r;
	float e = textureLodOffset(height_map, tc, lod.x, ivec2( 1, 0)).r;
	float s = textureLodOffset(height_map, tc, lod.x, ivec2( 0,-1)).r;
	float n = textureLodOffset(height_map, tc, lod.x, ivec2( 0, 1)).r;
	return normalize(vec3(w - e, 2.0 / vertical_scale, s - n));
}
#else
vec3 compute_normal(const in vec2 tc)
{
#if 1
	float w = textureOffset(height_map, tc, ivec2(-1, 0)).r;
	float e = textureOffset(height_map, tc, ivec2( 1, 0)).r;
	float s = textureOffset(height_map, tc, ivec2( 0,-1)).r;
	float n = textureOffset(height_map, tc, ivec2( 0, 1)).r;
	return normalize(vec3(w - e, 2.0 / vertical_scale, s - n));
#else
	vec3 e0 = normalize(vec3(1.0, r - l, 0.0));
	vec3 e1 = normalize(vec3(0.0, b - u, 1.0));
	return cross(e1, e0);
#endif
}
#endif

void main(void)
{
#ifdef ENABLE_MIPMAP
	vec3 n = compute_normal(fsTCoord);
#else
	vec3 n = compute_normal(fsTCoord);
#endif
	float d = dot(n, light_direction);
	if(d < 0.0){
		d = 0.0;
	}
#ifdef ENABLE_MIPMAP
	vec2 lod = textureQueryLod(diffuse_map, fsTCoord);
	OutColor.rgb = textureLod(diffuse_map, fsTCoord, lod.x).rgb * d;
#else
	OutColor.rgb = texture2D(diffuse_map, fsTCoord).rgb * d;
#endif
	//OutColor.rgb = vec3(d, d, d);
	//OutColor.rgb = (n + 1.0) * 0.5;
	OutColor.a = 1.0;
}