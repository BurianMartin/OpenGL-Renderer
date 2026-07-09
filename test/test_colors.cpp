#include <gtest/gtest.h>
#include "Forge/Colors.hpp"

#include <algorithm>
#include <vector>

namespace
{
    const std::vector<glm::vec4> kNamedColors = {
        Forge::Color_A1::Red,     Forge::Color_A1::Green,   Forge::Color_A1::Blue,
        Forge::Color_A1::White,   Forge::Color_A1::Black,   Forge::Color_A1::Gray,
        Forge::Color_A1::Yellow,  Forge::Color_A1::Cyan,    Forge::Color_A1::Magenta,
        Forge::Color_A1::Orange,  Forge::Color_A1::Purple,  Forge::Color_A1::Pink,
        Forge::Color_A1::Lime,    Forge::Color_A1::SkyBlue, Forge::Color_A1::Brown,
    };
}

// RandomColor() can't be tested against one precomputed expected value the way a deterministic
// formula can - there's nothing to precompute. Instead these check the properties any correct
// implementation must have: every result is one of the 15 named colors, and repeated calls
// actually vary (a `rand() % 15` that always took the same branch would be a real, catchable
// bug that a single-call test could never see).

TEST(ColorsTest, RandomColorAlwaysReturnsANamedColor)
{
    for (int i = 0; i < 200; ++i)
    {
        glm::vec4 color = Forge::Color_A1::RandomColor();
        bool isNamed = std::find(kNamedColors.begin(), kNamedColors.end(), color) != kNamedColors.end();
        EXPECT_TRUE(isNamed) << "RandomColor() returned a value that isn't one of the 15 named colors";
    }
}

TEST(ColorsTest, RandomColorProducesMoreThanOneDistinctValue)
{
    glm::vec4 first = Forge::Color_A1::RandomColor();
    bool foundDifferent = false;
    for (int i = 0; i < 200; ++i)
    {
        if (Forge::Color_A1::RandomColor() != first)
        {
            foundDifferent = true;
            break;
        }
    }
    EXPECT_TRUE(foundDifferent) << "200 calls to RandomColor() all returned the same value";
}
