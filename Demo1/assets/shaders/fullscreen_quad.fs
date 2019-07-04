#version 430

in vec2 fs_tex_coord;

uniform sampler2D u_tex0;

void main(void)
{
    vec3 color = texture(u_tex0, fs_tex_coord).rgb;
    gl_FragColor = vec4(color, 1.0);
}