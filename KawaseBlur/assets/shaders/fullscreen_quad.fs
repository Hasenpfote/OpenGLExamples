#version 430

in vec2 fsTCoord;
out vec4 OutColor;

uniform sampler2D texture0;

void main(void)
{
    //vec2 fsTCoord = gl_FragCoord.xy * vec2(1.0/640.0, 1.0/480.0);
    //vec2 lod = textureQueryLod(texture, fsTCoord.xy);
    //OutColor.rgb = textureLod(texture, fsTCoord.xy, lod.x).rgb;

    vec3 col = texture(texture0, fsTCoord).xyz;
    OutColor.rgb = col;
    OutColor.a = 1.0;
}