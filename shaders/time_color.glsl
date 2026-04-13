#version 450 core

out vec4 FragColor;

uniform float uTime;

void main()
{
    float r = sin(uTime) * 0.5 + 0.5;  // oscillates 0..1
    float g = sin(uTime + 2.0) * 0.5 + 0.5;
    float b = sin(uTime + 4.0) * 0.5 + 0.5;
    FragColor = vec4(r, g, b, 1.0);
}