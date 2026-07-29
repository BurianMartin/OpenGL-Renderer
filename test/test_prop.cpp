#include <gtest/gtest.h>
#include "Forge/Prop.hpp"

#include <glm/gtc/quaternion.hpp>

namespace
{
    void ExpectVec3Near(const glm::vec3 &actual, const glm::vec3 &expected, float epsilon = 1e-5f)
    {
        EXPECT_NEAR(actual.x, expected.x, epsilon);
        EXPECT_NEAR(actual.y, expected.y, epsilon);
        EXPECT_NEAR(actual.z, expected.z, epsilon);
    }
}

// Prop only ever calls Model's transform setters, never dereferencing the mesh pointer -
// Model(nullptr) is safe here too, same trick test_model.cpp/test_tweens.cpp use.

TEST(PropTest, AddPartAddsToGetParts)
{
    Forge::Prop prop;
    auto model = std::make_shared<Forge::Model>(nullptr);
    prop.AddPart(model);

    ASSERT_EQ(prop.GetParts().size(), 1u);
    EXPECT_EQ(prop.GetParts()[0], model);
}

TEST(PropTest, SetPositionAppliesToEveryPart)
{
    Forge::Prop prop;
    auto partA = std::make_shared<Forge::Model>(nullptr);
    auto partB = std::make_shared<Forge::Model>(nullptr);
    prop.AddPart(partA);
    prop.AddPart(partB);

    prop.SetPosition(glm::vec3(1.0f, 2.0f, 3.0f));

    ExpectVec3Near(glm::vec3(partA->GetModelMatrix()[3]), glm::vec3(1.0f, 2.0f, 3.0f));
    ExpectVec3Near(glm::vec3(partB->GetModelMatrix()[3]), glm::vec3(1.0f, 2.0f, 3.0f));
}

TEST(PropTest, SetScaleVec3AppliesToEveryPart)
{
    Forge::Prop prop;
    auto partA = std::make_shared<Forge::Model>(nullptr);
    auto partB = std::make_shared<Forge::Model>(nullptr);
    prop.AddPart(partA);
    prop.AddPart(partB);

    prop.SetScale(glm::vec3(2.0f, 3.0f, 4.0f));

    glm::vec4 transformedA = partA->GetModelMatrix() * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    glm::vec4 transformedB = partB->GetModelMatrix() * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    ExpectVec3Near(glm::vec3(transformedA), glm::vec3(2.0f, 3.0f, 4.0f));
    ExpectVec3Near(glm::vec3(transformedB), glm::vec3(2.0f, 3.0f, 4.0f));
}

TEST(PropTest, SetScaleUniformIsEquivalentToVec3Scale)
{
    Forge::Prop prop;
    auto part = std::make_shared<Forge::Model>(nullptr);
    prop.AddPart(part);

    prop.SetScale(2.0f);

    glm::vec4 transformed = part->GetModelMatrix() * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    ExpectVec3Near(glm::vec3(transformed), glm::vec3(2.0f, 2.0f, 2.0f));
}

TEST(PropTest, SetRotationQuatAppliesToEveryPart)
{
    Forge::Prop prop;
    auto partA = std::make_shared<Forge::Model>(nullptr);
    auto partB = std::make_shared<Forge::Model>(nullptr);
    prop.AddPart(partA);
    prop.AddPart(partB);

    glm::quat rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    prop.SetRotation(rotation);

    glm::vec4 transformedA = partA->GetModelMatrix() * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 transformedB = partB->GetModelMatrix() * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    ExpectVec3Near(glm::vec3(transformedA), glm::vec3(0.0f, 1.0f, 0.0f));
    ExpectVec3Near(glm::vec3(transformedB), glm::vec3(0.0f, 1.0f, 0.0f));
}

TEST(PropTest, SetRotationAngleAxisAppliesToEveryPart)
{
    Forge::Prop prop;
    auto part = std::make_shared<Forge::Model>(nullptr);
    prop.AddPart(part);

    prop.SetRotation(glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::vec4 transformed = part->GetModelMatrix() * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    ExpectVec3Near(glm::vec3(transformed), glm::vec3(0.0f, 1.0f, 0.0f));
}

// A Prop with no parts yet (e.g. constructed before AddPart is ever called) is a real state -
// every setter must be a safe no-op over an empty vector, not crash.
TEST(PropTest, SettersOnEmptyPropDoNotCrash)
{
    Forge::Prop prop;
    EXPECT_NO_THROW(prop.SetPosition(glm::vec3(1.0f)));
    EXPECT_NO_THROW(prop.SetScale(2.0f));
    EXPECT_NO_THROW(prop.SetRotation(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
}
