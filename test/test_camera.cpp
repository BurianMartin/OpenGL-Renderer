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
