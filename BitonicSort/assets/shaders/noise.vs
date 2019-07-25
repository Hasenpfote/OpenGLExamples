#version 430

layout(location = 0) in vec3 vs_position;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main(void)
{
    gl_Position = vec4(vs_position.x, vs_position.y, 0.0, 1.0);
}