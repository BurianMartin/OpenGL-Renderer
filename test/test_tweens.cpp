#include <gtest/gtest.h>
#include "Forge/Rendering/Tweens.hpp"

#include <glm/gtc/constants.hpp>

namespace
{
    void ExpectVec3Near(const glm::vec3 &actual, const glm::vec3 &expected, float epsilon = 1e-4f)
    {
        EXPECT_NEAR(actual.x, expected.x, epsilon);
        EXPECT_NEAR(actual.y, expected.y, epsilon);
        EXPECT_NEAR(actual.z, expected.z, epsilon);
    }
}

// Vec3Tween only ever calls Model::SetPosition/SetRotation, neither of which dereferences
// the underlying Mesh - a Model(nullptr) is safe here too, same trick test_model.cpp uses to
// avoid needing a GL context just to test transform/interpolation math.

TEST(Vec3TweenTest, LinearEaseAtHalfDurationGivesExactMidpoint)
{
    auto model = std::make_shared<Forge::Model>(nullptr);
    Forge::Vec3Tween tween(model, glm::vec3(0.0f), glm::vec3(10.0f, 0.0f, 0.0f), 2.0f, Forge::Vec3Tween::Ease::Linear);

    tween.Update(1.0f); // halfway through a 2-second duration

    ExpectVec3Near(tween.GetValuePosition(), glm::vec3(5.0f, 0.0f, 0.0f));
}

// EaseOutQuad(0.5) = 1 - 0.5^2 = 0.75, further along than Linear's 0.5 at the same elapsed
// time - a decelerating curve front-loads progress instead of moving at constant speed.
TEST(Vec3TweenTest, EaseOutQuadIsFurtherAlongThanLinearAtHalfDuration)
{
    auto model = std::make_shared<Forge::Model>(nullptr);
    Forge::Vec3Tween tween(model, glm::vec3(0.0f), glm::vec3(10.0f, 0.0f, 0.0f), 2.0f, Forge::Vec3Tween::Ease::EaseOutQuad);

    tween.Update(1.0f);

    ExpectVec3Near(tween.GetValuePosition(), glm::vec3(7.5f, 0.0f, 0.0f));
}

TEST(Vec3TweenTest, IsNotDoneBeforeDurationElapses)
{
    auto model = std::make_shared<Forge::Model>(nullptr);
    Forge::Vec3Tween tween(model, glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), 2.0f);

    tween.Update(1.0f);
    EXPECT_FALSE(tween.IsDone());
}

TEST(Vec3TweenTest, IsDoneOnceDurationFullyElapsesWithNoRepeat)
{
    auto model = std::make_shared<Forge::Model>(nullptr);
    Forge::Vec3Tween tween(model, glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), 2.0f);

    tween.Update(2.0f);
    EXPECT_TRUE(tween.IsDone());
}

// Update() clamps elapsed_ to duration_ under Repeat::None - overshooting the delta in one
// call must not let the position overshoot past the end pose.
TEST(Vec3TweenTest, NoRepeatClampsAtEndPoseEvenIfDeltaOvershoots)
{
    auto model = std::make_shared<Forge::Model>(nullptr);
    Forge::Vec3Tween tween(model, glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), 2.0f, Forge::Vec3Tween::Ease::Linear);

    tween.Update(5.0f); // way past the 2-second duration
    EXPECT_TRUE(tween.IsDone());
    ExpectVec3Near(tween.GetValuePosition(), glm::vec3(1.0f, 0.0f, 0.0f));
}

TEST(Vec3TweenTest, RepeatLoopNeverReportsDone)
{
    auto model = std::make_shared<Forge::Model>(nullptr);
    Forge::Vec3Tween tween(model, glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), 2.0f, Forge::Vec3Tween::Ease::Linear);
    tween.SetRepeat(Forge::Vec3Tween::Repeat::Loop);

    tween.Update(10.0f); // many multiples of the 2-second duration
    EXPECT_FALSE(tween.IsDone());
}

// Loop wraps elapsed_ via fmod(elapsed_, duration_) - 2.5s into a 2s loop should read exactly
// like 0.5s into a fresh one, not clamp at the end pose the way Repeat::None would.
TEST(Vec3TweenTest, RepeatLoopWrapsElapsedTimeViaFmod)
{
    auto model = std::make_shared<Forge::Model>(nullptr);
    Forge::Vec3Tween tween(model, glm::vec3(0.0f), glm::vec3(10.0f, 0.0f, 0.0f), 2.0f, Forge::Vec3Tween::Ease::Linear);
    tween.SetRepeat(Forge::Vec3Tween::Repeat::Loop);

    tween.Update(2.5f); // 0.5s into the second lap

    ExpectVec3Near(tween.GetValuePosition(), glm::vec3(2.5f, 0.0f, 0.0f));
}

// PingPong's period is duration_ * 2; past the midpoint of that period the tween must be
// travelling back toward the start pose instead of continuing toward the end pose.
TEST(Vec3TweenTest, RepeatPingPongReversesDirectionPastDuration)
{
    auto model = std::make_shared<Forge::Model>(nullptr);
    Forge::Vec3Tween tween(model, glm::vec3(0.0f), glm::vec3(10.0f, 0.0f, 0.0f), 2.0f, Forge::Vec3Tween::Ease::Linear);
    tween.SetRepeat(Forge::Vec3Tween::Repeat::PingPong);

    tween.Update(3.0f); // 1s past the 2s forward leg -> 1s back from the end pose
    ExpectVec3Near(tween.GetValuePosition(), glm::vec3(5.0f, 0.0f, 0.0f));
}

// A full 360-degree turn and "no rotation" are the same orientation, so slerp between two
// identical (start/end) quaternions can't represent a spin at all - it just sits still.
// SetSpin exists specifically to sidestep that; verify a full-turn spin at its halfway point
// has actually rotated 180 degrees, not collapsed to the identity the way plain slerp would.
TEST(Vec3TweenTest, SetSpinRepresentsAFullTurnThatSlerpCannot)
{
    auto model = std::make_shared<Forge::Model>(nullptr);
    Forge::Vec3Tween tween(model, glm::vec3(0.0f), glm::vec3(0.0f), 2.0f, Forge::Vec3Tween::Ease::Linear);
    tween.SetSpin(glm::vec3(0.0f, 1.0f, 0.0f), 2.0f * glm::pi<float>());

    tween.Update(1.0f); // halfway through the spin
    glm::quat expected = glm::angleAxis(glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat actual = tween.GetValueRotation();

    // Same orientation regardless of quaternion sign (q and -q represent the same rotation).
    EXPECT_NEAR(std::abs(glm::dot(actual, expected)), 1.0f, 1e-4f);
}

TEST(Vec3TweenTest, FinishCallbackFiresExactlyOnceWhenTweenCompletes)
{
    auto model = std::make_shared<Forge::Model>(nullptr);
    Forge::Vec3Tween tween(model, glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), 2.0f);
    int callCount = 0;
    tween.SetFinishFunction([&callCount]()
                            { callCount++; });

    tween.Update(1.0f); // not done yet
    EXPECT_EQ(callCount, 0);
    tween.Update(1.0f); // now done
    EXPECT_EQ(callCount, 1);
    tween.Update(1.0f); // stays done - must not refire
    EXPECT_EQ(callCount, 1);
}

// Repeating tweens fire the finish callback once per completed lap, not once ever (which
// would never happen under Loop) - it's the "lap complete" signal, not "the tween is over".
TEST(Vec3TweenTest, FinishCallbackFiresOncePerCompletedLapWhenLooping)
{
    auto model = std::make_shared<Forge::Model>(nullptr);
    Forge::Vec3Tween tween(model, glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), 2.0f);
    tween.SetRepeat(Forge::Vec3Tween::Repeat::Loop);
    int callCount = 0;
    tween.SetFinishFunction([&callCount]()
                            { callCount++; });

    tween.Update(1.0f); // half a lap
    EXPECT_EQ(callCount, 0);
    tween.Update(1.0f); // completes lap 1
    EXPECT_EQ(callCount, 1);
    tween.Update(2.0f); // completes lap 2
    EXPECT_EQ(callCount, 2);
}

TEST(Vec3TweenTest, UpdateWritesValueIntoTheOwnedModel)
{
    auto model = std::make_shared<Forge::Model>(nullptr);
    Forge::Vec3Tween tween(model, glm::vec3(0.0f), glm::vec3(10.0f, 0.0f, 0.0f), 2.0f, Forge::Vec3Tween::Ease::Linear);

    tween.Update(1.0f);

    ExpectVec3Near(glm::vec3(model->GetModelMatrix()[3]), glm::vec3(5.0f, 0.0f, 0.0f));
}
