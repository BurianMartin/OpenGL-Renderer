#include <gtest/gtest.h>
#include "Forge/Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

namespace
{
    // glm::mat4 has no epsilon-aware equality, so compare element-by-element.
    void ExpectMat4Near(const glm::mat4 &actual, const glm::mat4 &expected, float epsilon)
    {
        for (int col = 0; col < 4; ++col)
        {
            for (int row = 0; row < 4; ++row)
            {
                EXPECT_NEAR(actual[col][row], expected[col][row], epsilon);
            }
        }
    }

    bool Mat4ApproxEqual(const glm::mat4 &a, const glm::mat4 &b, float epsilon)
    {
        for (int col = 0; col < 4; ++col)
        {
            for (int row = 0; row < 4; ++row)
            {
                if (std::fabs(a[col][row] - b[col][row]) > epsilon)
                {
                    return false;
                }
            }
        }
        return true;
    }
}

TEST(CameraTest, FovLowerBound)
{
    Forge::Camera test_cam(Forge::Viewport(0, 0, 1.0f, 1.0f, 1080, 1080));
    EXPECT_FLOAT_EQ(test_cam.GetFOV(), 45.0f);
    test_cam.Zoom(90.0f);
    EXPECT_FLOAT_EQ(test_cam.GetFOV(), 1.0f);
}

TEST(CameraTest, FovUpperBound)
{
    Forge::Camera test_cam(Forge::Viewport(0, 0, 1.0f, 1.0f, 1080, 1080));
    EXPECT_FLOAT_EQ(test_cam.GetFOV(), 45.0f);
    test_cam.Zoom(-90.0f);
    EXPECT_FLOAT_EQ(test_cam.GetFOV(), 45.0f);
}

TEST(CameraTest, FovZeroShift)
{
    Forge::Camera test_cam(Forge::Viewport(0, 0, 1.0f, 1.0f, 1080, 1080));
    EXPECT_FLOAT_EQ(test_cam.GetFOV(), 45.0f);
    test_cam.Zoom(0.0f);
    EXPECT_FLOAT_EQ(test_cam.GetFOV(), 45.0f);
}

TEST(CameraTest, FovWithinBoundsDecreasesByOffset)
{
    Forge::Camera test_cam(Forge::Viewport(0, 0, 1.0f, 1.0f, 1080, 1080));
    test_cam.Zoom(10.0f);
    EXPECT_FLOAT_EQ(test_cam.GetFOV(), 35.0f);
}

TEST(CameraTest, FovSequentialZoomsAccumulate)
{
    Forge::Camera test_cam(Forge::Viewport(0, 0, 1.0f, 1.0f, 1080, 1080));
    test_cam.Zoom(5.0f);
    test_cam.Zoom(5.0f);
    EXPECT_FLOAT_EQ(test_cam.GetFOV(), 35.0f);
}

// Default pose (position (0,0,3), front (0,0,-1), up (0,1,0)) has no rotation relative to
// world axes, so the view matrix collapses to a pure translation - a rare case where the
// expected matrix can be hand-derived and checked exactly instead of just "did it change".
TEST(CameraTest, ViewMatrixInitiallyPureTranslation)
{
    Forge::Camera test_cam(Forge::Viewport(0, 0, 1.0f, 1.0f, 1080, 1080));
    glm::mat4 expected = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
    ExpectMat4Near(test_cam.GetViewMatrix(), expected, 1e-5f);
}

// proj[0][0] == 1 / (aspect * tan(fovy/2)) - inversely proportional to aspect ratio with fov
// held constant, so doubling the aspect ratio must exactly halve it.
TEST(CameraTest, ProjectionScalesInverselyWithAspectRatio)
{
    Forge::Camera test_cam(Forge::Viewport(0, 0, 1.0f, 1.0f, 1080, 1080));
    float before = test_cam.GetProjectionMatrix()[0][0];
    test_cam.UpdateViewportSize(2160, 1080);
    float after = test_cam.GetProjectionMatrix()[0][0];
    EXPECT_NEAR(after, before / 2.0f, 1e-4f);
}

TEST(CameraTest, MoveForwardUpdatesPosition)
{
    Forge::Camera test_cam(Forge::Viewport(0, 0, 1.0f, 1.0f, 1080, 1080));
    test_cam.CameraMove(Forge::CamMove::FORWARD, true);
    test_cam.Update(1.0f);
    glm::vec3 position = test_cam.GetPosition();
    EXPECT_FLOAT_EQ(position.x, 0.0f);
    EXPECT_FLOAT_EQ(position.y, 0.0f);
    EXPECT_FLOAT_EQ(position.z, 2.0f);
}

// cross(front, up) drives strafing; get the operand order backwards and this silently reverses.
TEST(CameraTest, MoveRightUpdatesPositionViaCrossProduct)
{
    Forge::Camera test_cam(Forge::Viewport(0, 0, 1.0f, 1.0f, 1080, 1080));
    test_cam.CameraMove(Forge::CamMove::RIGHT, true);
    test_cam.Update(1.0f);
    glm::vec3 position = test_cam.GetPosition();
    EXPECT_FLOAT_EQ(position.x, 1.0f);
    EXPECT_FLOAT_EQ(position.y, 0.0f);
    EXPECT_FLOAT_EQ(position.z, 3.0f);
}

// Update() applies every held direction with independent ifs, not else-if - holding two
// directions at once must move diagonally, not just pick one.
TEST(CameraTest, MoveForwardAndRightComposeDiagonally)
{
    Forge::Camera test_cam(Forge::Viewport(0, 0, 1.0f, 1.0f, 1080, 1080));
    test_cam.CameraMove(Forge::CamMove::FORWARD, true);
    test_cam.CameraMove(Forge::CamMove::RIGHT, true);
    test_cam.Update(1.0f);
    glm::vec3 position = test_cam.GetPosition();
    EXPECT_FLOAT_EQ(position.x, 1.0f);
    EXPECT_FLOAT_EQ(position.y, 0.0f);
    EXPECT_FLOAT_EQ(position.z, 2.0f);
}

TEST(CameraTest, MoveStopsAfterDisablingFlag)
{
    Forge::Camera test_cam(Forge::Viewport(0, 0, 1.0f, 1.0f, 1080, 1080));
    test_cam.CameraMove(Forge::CamMove::FORWARD, true);
    test_cam.Update(1.0f);
    test_cam.CameraMove(Forge::CamMove::FORWARD, false);
    test_cam.Update(1.0f);
    glm::vec3 position = test_cam.GetPosition();
    EXPECT_FLOAT_EQ(position.x, 0.0f);
    EXPECT_FLOAT_EQ(position.y, 0.0f);
    EXPECT_FLOAT_EQ(position.z, 2.0f);
}

// The very first ProcessMousePosition call after construction only primes last_x_/last_y_ -
// it must not rotate the camera, or the very first frame of mouse capture would jump.
TEST(CameraTest, FirstMouseCallOnlyPrimesTracking)
{
    Forge::Camera test_cam(Forge::Viewport(0, 0, 1.0f, 1.0f, 1080, 1080));
    glm::mat4 before = test_cam.GetViewMatrix();
    test_cam.ProcessMousePosition(100.0f, 100.0f);
    glm::mat4 after = test_cam.GetViewMatrix();
    ExpectMat4Near(after, before, 1e-5f);
}

TEST(CameraTest, SecondMouseCallRotatesCamera)
{
    Forge::Camera test_cam(Forge::Viewport(0, 0, 1.0f, 1.0f, 1080, 1080));
    test_cam.ProcessMousePosition(100.0f, 100.0f); // primes tracking, no rotation yet
    glm::mat4 before = test_cam.GetViewMatrix();
    test_cam.ProcessMousePosition(150.0f, 100.0f); // real delta, must rotate
    glm::mat4 after = test_cam.GetViewMatrix();
    EXPECT_FALSE(Mat4ApproxEqual(after, before, 1e-4f));
}

// Constructor defaults (near=0.1, far=100.0) are relied on here since Camera exposes no getter
// for them — matches how other tests already hardcode known constructor defaults (e.g. fov=45).
TEST(CameraTest, OrthographicProjectionMatchesGlmOrtho)
{
    Forge::Camera test_cam(Forge::Viewport(0, 0, 1.0f, 1.0f, 1080, 1080)); // aspect 1:1
    test_cam.SetOrthographic(5.0f);
    glm::mat4 expected = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
    ExpectMat4Near(test_cam.GetProjectionMatrix(), expected, 1e-5f);
}

// Unlike FOV-based perspective, an orthographic half-height is a fixed world-space size —
// widening the aspect ratio must widen the left/right bounds (half_height * aspect) while
// leaving top/bottom untouched, not just uniformly rescale like FovWithinBoundsDecreasesByOffset.
TEST(CameraTest, OrthographicProjectionScalesWidthWithAspectRatio)
{
    Forge::Camera test_cam(Forge::Viewport(0, 0, 1.0f, 1.0f, 1080, 1080));
    test_cam.SetOrthographic(5.0f);
    test_cam.UpdateViewportSize(2160, 1080); // aspect 2:1
    glm::mat4 expected = glm::ortho(-10.0f, 10.0f, -5.0f, 5.0f, 0.1f, 100.0f);
    ExpectMat4Near(test_cam.GetProjectionMatrix(), expected, 1e-5f);
}

// Regression guard for the documented straight-down degeneracy: with the default up (0,1,0),
// pitch -90 makes front_ nearly antiparallel to up_, collapsing glm::lookAt's internal
// cross(front, up) toward zero and corrupting the view matrix's rotation basis. SetUp must fix
// this by supplying a horizontal reference instead — checked here by confirming the resulting
// basis vectors are still unit-length and mutually perpendicular, not just "some matrix came back".
TEST(CameraTest, SetUpProducesValidOrthonormalBasisWhenLookingStraightDown)
{
    Forge::Camera test_cam(Forge::Viewport(0, 0, 1.0f, 1.0f, 1080, 1080));
    test_cam.SetYawPitch(-90.0f, -90.0f); // front_ becomes ~(0,-1,0)
    test_cam.SetUp(glm::vec3(0.0f, 0.0f, -1.0f));

    glm::mat4 view = test_cam.GetViewMatrix();
    // glm::lookAt packs the right/up basis vectors into the view matrix's rows (not columns):
    // row 0 = right, row 1 = up (see glm::lookAtRH's [col][row] assignments).
    glm::vec3 right(view[0][0], view[1][0], view[2][0]);
    glm::vec3 viewUp(view[0][1], view[1][1], view[2][1]);

    EXPECT_NEAR(glm::length(right), 1.0f, 1e-4f);
    EXPECT_NEAR(glm::length(viewUp), 1.0f, 1e-4f);
    EXPECT_NEAR(glm::dot(right, viewUp), 0.0f, 1e-4f);
}
