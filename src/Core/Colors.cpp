#include "Core/Colors.hpp"

const glm::vec4 Core::Color_A1::RandomColor()
{
    int color = rand() % 15;
    switch (color)
    {
    case 0:
        return Red;
    case 1:
        return Green;
    case 2:
        return Blue;
    case 3:
        return White;
    case 4:
        return Black;
    case 5:
        return Gray;
    case 6:
        return Yellow;
    case 7:
        return Cyan;
    case 8:
        return Magenta;
    case 9:
        return Orange;
    case 10:
        return Purple;
    case 11:
        return Pink;
    case 12:
        return Lime;
    case 13:
        return SkyBlue;
    case 14:
        return Brown;
    default:
        return White;
    }
}