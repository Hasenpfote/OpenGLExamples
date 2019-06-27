#version 430

layout(location = 0) in vec3 vsPosition;

out gl_PerVertex
{
    vec4 gl_Position;
};

out vec2 fsTCoord;

//uniform sampler2D texture0;

void main(void)
{
    gl_Position = vec4(vsPosition.x, vsPosition.y, 0.0, 1.0);
    fsTCoord = vec2(vsPosition.x * 0.5 + 0.5, -vsPosition.y * 0.5 + 0.5);
}