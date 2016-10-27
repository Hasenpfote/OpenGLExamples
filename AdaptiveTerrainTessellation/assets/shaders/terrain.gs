#version 430

layout(triangles, invocations = 1) in;
layout(triangle_strip, max_vertices = 3) out;

in gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
} gl_in[];

out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

in vec2 gsTCoord[];
out vec2 fsTCoord;

void main(void)
{
	gl_Position = gl_in[0].gl_Position;
	fsTCoord = gsTCoord[0];
	EmitVertex();

	gl_Position = gl_in[1].gl_Position;
	fsTCoord = gsTCoord[1];
	EmitVertex();

	gl_Position = gl_in[2].gl_Position;
	fsTCoord = gsTCoord[2];
	EmitVertex();
	
	EndPrimitive();
}
