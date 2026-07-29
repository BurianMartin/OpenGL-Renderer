#include <gtest/gtest.h>
#include "Forge/Camera.hpp"

// GetAspectRatio() = (window_width * width_fraction) / (window_height * height_fraction),
// recomputed by RecomputeAspectRatio() whenever the window size or the viewport's own
// fraction changes - exercised directly here rather than only indirectly through Camera's
// projection-matrix tests. Apply() is deliberately not tested - it issues real
// glViewport/glScissor calls and needs a live GL context, like every other GPU-touching path.

TEST(ViewportTest, FullWindowAspectRatioMatchesWindowDimensions)
{
    Forge::Viewport viewport(0.0f, 0.0f, 1.0f, 1.0f, 1920, 1080);
    EXPECT_NEAR(viewport.GetAspectRatio(), 1920.0f / 1080.0f, 1e-5f);
}

TEST(ViewportTest, SquareWindowGivesUnitAspectRatio)
{
    Forge::Viewport viewport(0.0f, 0.0f, 1.0f, 1.0f, 1080, 1080);
    EXPECT_FLOAT_EQ(viewport.GetAspectRatio(), 1.0f);
}

// A picture-in-picture inset (e.g. 30%x30% of the window) derives its aspect ratio from its
// own fraction of each axis - here that fraction is uniform (0.3/0.3), so the ratio must
// still match the whole window's, not be skewed by the inset itself.
TEST(ViewportTest, PartialViewportFractionAffectsAspectRatio)
{
    Forge::Viewport viewport(0.68f, 0.68f, 0.3f, 0.3f, 1920, 1080);
    EXPECT_NEAR(viewport.GetAspectRatio(), (1920.0f * 0.3f) / (1080.0f * 0.3f), 1e-5f);
}

TEST(ViewportTest, SetWindowSizeRecomputesAspectRatioAgainstNewSize)
{
    Forge::Viewport viewport(0.0f, 0.0f, 1.0f, 1.0f, 1080, 1080);
    ASSERT_FLOAT_EQ(viewport.GetAspectRatio(), 1.0f);

    viewport.SetWindowSize(1920, 1080);
    EXPECT_NEAR(viewport.GetAspectRatio(), 1920.0f / 1080.0f, 1e-5f);
}

// SetSize changes the viewport's own normalized fraction, not the window it's measured
// against - halving both width and height fractions leaves the ratio between them
// unchanged, so the aspect ratio must stay the same.
TEST(ViewportTest, SetSizeWithSameRatioLeavesAspectRatioUnchanged)
{
    Forge::Viewport viewport(0.0f, 0.0f, 1.0f, 1.0f, 1920, 1080);
    float before = viewport.GetAspectRatio();

    viewport.SetSize(0.5f, 0.5f);
    EXPECT_NEAR(viewport.GetAspectRatio(), before, 1e-5f);
}

// A non-square fraction of a square window - guards against width_/height_ being swapped or
// one of them being silently ignored.
TEST(ViewportTest, SetSizeWithDifferentRatioChangesAspectRatio)
{
    Forge::Viewport viewport(0.0f, 0.0f, 1.0f, 1.0f, 1000, 1000);
    viewport.SetSize(0.5f, 0.25f);
    EXPECT_NEAR(viewport.GetAspectRatio(), 2.0f, 1e-5f);
}

// SetViewportPos only moves the normalized origin - it must not touch size or aspect ratio.
TEST(ViewportTest, SetViewportPosDoesNotAffectAspectRatio)
{
    Forge::Viewport viewport(0.0f, 0.0f, 1.0f, 1.0f, 1920, 1080);
    float before = viewport.GetAspectRatio();

    viewport.SetViewportPos(0.25f, 0.25f);
    EXPECT_NEAR(viewport.GetAspectRatio(), before, 1e-5f);
}
