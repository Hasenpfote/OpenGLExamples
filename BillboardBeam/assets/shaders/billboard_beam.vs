#version 430

layout(location = 0) in vec4 vsPosition;
layout(location = 1) in vec2 vsTCoord;

out gl_PerVertex
{
    vec4 gl_Position;
};

out vec2 fsTCoord;
out vec4 fsColor;

uniform float size;
uniform mat4 bb[2];	// billboard beam matrices.
uniform vec4 color[2];
uniform mat4 mvp;


void main(void)
{
	int index = int(vsPosition.w);
	gl_Position = mvp * bb[index] * vec4(vsPosition.xyz * size, 1.0);
	fsTCoord = vsTCoord;
	fsColor = color[index];
}