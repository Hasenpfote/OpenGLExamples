#version 430

layout(location = 0) in vec3 vsPosition;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main(void)
{
    gl_Position = vec4(vsPosition.x, vsPosition.y, 0.0, 1.0);
}