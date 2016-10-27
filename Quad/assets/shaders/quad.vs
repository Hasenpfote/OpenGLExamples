#version 430

layout(location = 0) in vec3 vsPosition;
layout(location = 1) in vec2 vsTCoord;

out gl_PerVertex
{
    vec4 gl_Position;
};

out vec2 fsTCoord;

uniform mat4 mvp;
uniform sampler2D texture;


void main(void)
{
#if 0
	float height = dot(texture2D(texture, vsTCoord).rgb, vec3(0.299, 0.587, 0.114));
	gl_Position = mvp * vec4(vsPosition.x, vsPosition.y, height, 1.0);
#else
	gl_Position = mvp * vec4(vsPosition.x, vsPosition.y, 0.0, 1.0);
#endif	
	fsTCoord = vsTCoord;
}