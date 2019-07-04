#version 430

layout(location = 0) in vec3 vs_position;

out gl_PerVertex
{
    vec4 gl_Position;
};

out vec2 fs_tex_coord;

void main(void)
{
    gl_Position = vec4(vs_position.x, vs_position.y, 0.0, 1.0);
    fs_tex_coord = vec2(vs_position.x * 0.5 + 0.5, -vs_position.y * 0.5 + 0.5);
}