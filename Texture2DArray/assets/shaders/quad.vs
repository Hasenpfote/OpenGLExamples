#version 430

layout(location = 0) in vec3 vsPosition;
layout(location = 1) in vec3 vsTCoord;

out gl_PerVertex
{
    vec4 gl_Position;
};

out vec3 fsTCoord;

uniform mat4 mvp;
uniform sampler2D texture;


void main(void)
{
	gl_Position = mvp * vec4(vsPosition.x, vsPosition.y, 0.0, 1.0);
	fsTCoord = vsTCoord;
}