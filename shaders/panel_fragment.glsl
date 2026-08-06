#version 450 core

in vec2 vLocal;
out vec4 FragColor;

uniform vec4 uFillColor;
uniform vec4 uBorderColor;
uniform vec2 uBorderThicknessUV; // border thickness as a fraction of width/height; (0,0) means no border at all

void main()
{
    bool inBorder = vLocal.x < uBorderThicknessUV.x || vLocal.x > 1.0 - uBorderThicknessUV.x ||
                    vLocal.y < uBorderThicknessUV.y || vLocal.y > 1.0 - uBorderThicknessUV.y;
    FragColor = inBorder ? uBorderColor : uFillColor;
}
