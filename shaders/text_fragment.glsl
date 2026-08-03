#version 450 core

in vec2 vTexCoords;
out vec4 FragColor;

uniform sampler2D uAtlas; // Font's single-channel (coverage/alpha) baked glyph atlas
uniform vec4 uColor;

void main()
{
    float coverage = texture(uAtlas, vTexCoords).r;
    FragColor = vec4(uColor.rgb, uColor.a * coverage);
}
