#include <gtest/gtest.h>
#include "Forge/Rendering/ClickableRegion.hpp"

namespace
{
    Forge::MouseButtonPressedEvent LeftClickAt(double x, double y)
    {
        return Forge::MouseButtonPressedEvent(Forge::MouseButton::Left, x, y);
    }
}

TEST(ClickableRegionTest, ContainsIsTrueInsideTheRect)
{
    Forge::ClickableRegion region(10.0f, 20.0f, 100.0f, 50.0f);
    EXPECT_TRUE(region.Contains(50.0f, 40.0f));
}

TEST(ClickableRegionTest, ContainsIncludesAllFourEdges)
{
    Forge::ClickableRegion region(10.0f, 20.0f, 100.0f, 50.0f);
    EXPECT_TRUE(region.Contains(10.0f, 20.0f));   // top-left corner
    EXPECT_TRUE(region.Contains(110.0f, 20.0f));  // top-right corner
    EXPECT_TRUE(region.Contains(10.0f, 70.0f));   // bottom-left corner
    EXPECT_TRUE(region.Contains(110.0f, 70.0f));  // bottom-right corner
}

TEST(ClickableRegionTest, ContainsIsFalseOutsideTheRect)
{
    Forge::ClickableRegion region(10.0f, 20.0f, 100.0f, 50.0f);
    EXPECT_FALSE(region.Contains(9.0f, 40.0f));    // just left of it
    EXPECT_FALSE(region.Contains(111.0f, 40.0f));  // just right of it
    EXPECT_FALSE(region.Contains(50.0f, 19.0f));   // just above it
    EXPECT_FALSE(region.Contains(50.0f, 71.0f));   // just below it
}

TEST(ClickableRegionTest, SetRectMovesAndResizesTheHitArea)
{
    Forge::ClickableRegion region(0.0f, 0.0f, 10.0f, 10.0f);
    EXPECT_TRUE(region.Contains(5.0f, 5.0f));

    region.SetRect(100.0f, 100.0f, 10.0f, 10.0f);
    EXPECT_FALSE(region.Contains(5.0f, 5.0f)) << "old rect should no longer count as a hit";
    EXPECT_TRUE(region.Contains(105.0f, 105.0f));
}

TEST(ClickableRegionTest, HandleClickInvokesCallbackOnLeftClickInside)
{
    Forge::ClickableRegion region(0.0f, 0.0f, 100.0f, 100.0f);
    int clickCount = 0;
    region.SetOnClick([&clickCount]()
                       { ++clickCount; });

    auto ev = LeftClickAt(50.0, 50.0);
    bool handled = region.HandleClick(ev);

    EXPECT_TRUE(handled);
    EXPECT_EQ(clickCount, 1);
}

TEST(ClickableRegionTest, HandleClickIgnoresClicksOutsideTheRegion)
{
    Forge::ClickableRegion region(0.0f, 0.0f, 100.0f, 100.0f);
    int clickCount = 0;
    region.SetOnClick([&clickCount]()
                       { ++clickCount; });

    auto ev = LeftClickAt(500.0, 500.0);
    bool handled = region.HandleClick(ev);

    EXPECT_FALSE(handled);
    EXPECT_EQ(clickCount, 0);
}

TEST(ClickableRegionTest, HandleClickIgnoresNonLeftButtons)
{
    Forge::ClickableRegion region(0.0f, 0.0f, 100.0f, 100.0f);
    int clickCount = 0;
    region.SetOnClick([&clickCount]()
                       { ++clickCount; });

    Forge::MouseButtonPressedEvent rightClick(Forge::MouseButton::Right, 50.0, 50.0);
    bool handled = region.HandleClick(rightClick);

    EXPECT_FALSE(handled);
    EXPECT_EQ(clickCount, 0);
}

TEST(ClickableRegionTest, HandleClickReturnsTrueEvenWithNoCallbackSet)
{
    // A region with no SetOnClick() call is still a valid, "handled" hit — HandleClick's
    // return value means "this event was consumed", independent of whether a callback
    // happened to be attached.
    Forge::ClickableRegion region(0.0f, 0.0f, 100.0f, 100.0f);
    auto ev = LeftClickAt(50.0, 50.0);
    EXPECT_TRUE(region.HandleClick(ev));
}

TEST(FindTopmostContainingTest, ReturnsMinusOneWhenNothingContainsThePoint)
{
    std::vector<Forge::ClickableRegion> regions{
        Forge::ClickableRegion(0.0f, 0.0f, 10.0f, 10.0f),
        Forge::ClickableRegion(20.0f, 20.0f, 10.0f, 10.0f),
    };
    EXPECT_EQ(Forge::FindTopmostContaining(regions, 15.0f, 15.0f), -1);
}

TEST(FindTopmostContainingTest, ReturnsTheOnlyMatchWhenRegionsDontOverlap)
{
    std::vector<Forge::ClickableRegion> regions{
        Forge::ClickableRegion(0.0f, 0.0f, 10.0f, 10.0f),
        Forge::ClickableRegion(20.0f, 20.0f, 10.0f, 10.0f),
    };
    EXPECT_EQ(Forge::FindTopmostContaining(regions, 5.0f, 5.0f), 0);
    EXPECT_EQ(Forge::FindTopmostContaining(regions, 25.0f, 25.0f), 1);
}

TEST(FindTopmostContainingTest, PrefersTheLastRegionWhenOverlapping)
{
    // Vector order doubles as z-order (last = drawn/registered most recently = on top) --
    // see the function's own doc comment.
    std::vector<Forge::ClickableRegion> regions{
        Forge::ClickableRegion(0.0f, 0.0f, 100.0f, 100.0f),
        Forge::ClickableRegion(10.0f, 10.0f, 20.0f, 20.0f),
    };
    EXPECT_EQ(Forge::FindTopmostContaining(regions, 15.0f, 15.0f), 1);
}

TEST(FindTopmostContainingTest, EmptyCollectionReturnsMinusOne)
{
    std::vector<Forge::ClickableRegion> regions;
    EXPECT_EQ(Forge::FindTopmostContaining(regions, 0.0f, 0.0f), -1);
}

TEST(ClickableRegionTest, SetOnClickReplacesThePreviousCallback)
{
    Forge::ClickableRegion region(0.0f, 0.0f, 100.0f, 100.0f);
    int firstCount = 0, secondCount = 0;
    region.SetOnClick([&firstCount]()
                       { ++firstCount; });
    region.SetOnClick([&secondCount]()
                       { ++secondCount; });

    region.HandleClick(LeftClickAt(50.0, 50.0));

    EXPECT_EQ(firstCount, 0);
    EXPECT_EQ(secondCount, 1);
}
