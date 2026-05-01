#version 330 core
uniform vec4 triangle_color;
out vec4 FragColor;

void main()
{
    FragColor = triangle_color;
}