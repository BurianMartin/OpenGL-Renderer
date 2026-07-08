#include <gtest/gtest.h>
#include "Core/Lights.hpp"

namespace
{
    void ExpectVec3Near(const glm::vec3 &actual, const glm::vec3 &expected, float epsilon = 1e-5f)
    {
        EXPECT_NEAR(actual.x, expected.x, epsilon);
        EXPECT_NEAR(actual.y, expected.y, epsilon);
        EXPECT_NEAR(actual.z, expected.z, epsilon);
    }

    void ExpectVec4Near(const glm::vec4 &actual, const glm::vec4 &expected, float epsilon = 1e-5f)
    {
        EXPECT_NEAR(actual.x, expected.x, epsilon);
        EXPECT_NEAR(actual.y, expected.y, epsilon);
        EXPECT_NEAR(actual.z, expected.z, epsilon);
        EXPECT_NEAR(actual.w, expected.w, epsilon);
    }
}

// --- Type identity -----------------------------------------------------------------------
//
// GetLightType()/GetStaticType()/GetName() are generated once per class by the
// LIGHT_CLASS_TYPE(x) macro. A copy-paste mistake in that one-line invocation (e.g. SpotLight
// accidentally writing LIGHT_CLASS_TYPE(Point)) would compile fine and silently misidentify
// the light everywhere its type is checked - these are cheap insurance against exactly that.

TEST(LightsTest, DirectionalLightReportsCorrectType)
{
    auto light = Core::DirectionalLight::Create();
    ASSERT_NE(light, nullptr);
    EXPECT_EQ(light->GetLightType(), Core::LightType::Directional);
    EXPECT_EQ(Core::DirectionalLight::GetStaticType(), Core::LightType::Directional);
    EXPECT_STREQ(light->GetName(), "Directional");
}

TEST(LightsTest, PointLightReportsCorrectType)
{
    auto light = Core::PointLight::Create();
    ASSERT_NE(light, nullptr);
    EXPECT_EQ(light->GetLightType(), Core::LightType::Point);
    EXPECT_EQ(Core::PointLight::GetStaticType(), Core::LightType::Point);
    EXPECT_STREQ(light->GetName(), "Point");
}

TEST(LightsTest, SpotLightReportsCorrectType)
{
    auto light = Core::SpotLight::Create();
    ASSERT_NE(light, nullptr);
    EXPECT_EQ(light->GetLightType(), Core::LightType::Spot);
    EXPECT_EQ(Core::SpotLight::GetStaticType(), Core::LightType::Spot);
    EXPECT_STREQ(light->GetName(), "Spot");
}

// --- Direction normalization -------------------------------------------------------------
//
// Both the constructor and SetDirection() normalize internally, so callers never have to.
// (3,4,0) has length 5 - a clean 3-4-5 triangle - so the expected unit vector (0.6,0.8,0) can
// be checked by hand instead of trusting a square root.

TEST(LightsTest, DirectionalLightConstructorNormalizesDirection)
{
    auto light = Core::DirectionalLight::Create(glm::vec3(3.0f, 4.0f, 0.0f));
    ASSERT_NE(light, nullptr);
    ExpectVec3Near(light->GetDirection(), glm::vec3(0.6f, 0.8f, 0.0f));
}

TEST(LightsTest, DirectionalLightSetDirectionRenormalizes)
{
    auto light = Core::DirectionalLight::Create();
    light->SetDirection(glm::vec3(0.0f, 0.0f, 5.0f));
    ExpectVec3Near(light->GetDirection(), glm::vec3(0.0f, 0.0f, 1.0f));
}

TEST(LightsTest, SpotLightConstructorNormalizesDirection)
{
    auto light = Core::SpotLight::Create(glm::vec3(0.0f), glm::vec3(3.0f, 4.0f, 0.0f));
    ASSERT_NE(light, nullptr);
    ExpectVec3Near(light->GetDirection(), glm::vec3(0.6f, 0.8f, 0.0f));
}

// --- SpotLight cutoff: stored as cos(radians(degrees)), not the raw degree value ----------
//
// GetInnerCutoff()/GetOuterCutoff() intentionally do NOT return what was passed to Create() -
// the shader compares against a cosine directly, so the conversion happens once here instead
// of in every shader invocation. 0/60/90/120 degrees are chosen because their cosines
// (1, 0.5, 0, -0.5) are exact, clean values - no calculator needed to know if this is right.

TEST(LightsTest, SpotLightZeroAndNinetyDegreeCutoffsConvertToCosine)
{
    auto spot = Core::SpotLight::Create(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f),
                                        glm::vec3(1.0f), 1.0f, 0.0f, 90.0f);
    ASSERT_NE(spot, nullptr);
    EXPECT_NEAR(spot->GetInnerCutoff(), 1.0f, 1e-5f); // cos(0°)
    EXPECT_NEAR(spot->GetOuterCutoff(), 0.0f, 1e-5f); // cos(90°)
}

TEST(LightsTest, SpotLightSixtyAndOneTwentyDegreeCutoffsConvertToCosine)
{
    auto spot = Core::SpotLight::Create(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f),
                                        glm::vec3(1.0f), 1.0f, 60.0f, 120.0f);
    ASSERT_NE(spot, nullptr);
    EXPECT_NEAR(spot->GetInnerCutoff(), 0.5f, 1e-5f);  // cos(60°)
    EXPECT_NEAR(spot->GetOuterCutoff(), -0.5f, 1e-5f); // cos(120°)
}

// --- ToGPULight(): fixed slot layout the shader reads directly ----------------------------
//
// Getting a slot wrong here doesn't crash - the shader just silently reads the wrong field for
// that light, a much harder bug to notice than a crash. These pin down exactly which field
// goes in which slot, matching the layout documented in GPULight's own comments.

TEST(LightsTest, DirectionalLightPacksIntoGPULightLayout)
{
    auto light = Core::DirectionalLight::Create(glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 2.0f);
    ASSERT_NE(light, nullptr);
    Core::GPULight gpu = light->ToGPULight();

    ExpectVec4Near(gpu.position, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));   // w = LightType::Directional (0)
    ExpectVec4Near(gpu.direction, glm::vec4(0.0f, 0.0f, -1.0f, 2.0f)); // xyz = direction, w = intensity
    ExpectVec4Near(gpu.color, glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
    ExpectVec4Near(gpu.params, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
    ExpectVec4Near(gpu.params2, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
}

TEST(LightsTest, PointLightPacksIntoGPULightLayout)
{
    auto light = Core::PointLight::Create(glm::vec3(1.0f, 2.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), 3.0f,
                                          1.0f, 0.09f, 0.032f);
    ASSERT_NE(light, nullptr);
    Core::GPULight gpu = light->ToGPULight();

    ExpectVec4Near(gpu.position, glm::vec4(1.0f, 2.0f, 3.0f, 1.0f));   // w = LightType::Point (1)
    ExpectVec4Near(gpu.direction, glm::vec4(0.0f, 0.0f, 0.0f, 3.0f));  // xyz unused, w = intensity
    ExpectVec4Near(gpu.color, glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
    ExpectVec4Near(gpu.params, glm::vec4(1.0f, 0.09f, 0.032f, 0.0f));
    ExpectVec4Near(gpu.params2, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
}

TEST(LightsTest, SpotLightPacksIntoGPULightLayout)
{
    auto light = Core::SpotLight::Create(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f),
                                         glm::vec3(0.0f, 0.0f, 1.0f), 1.5f,
                                         0.0f, 90.0f, 1.0f, 0.09f, 0.032f);
    ASSERT_NE(light, nullptr);
    Core::GPULight gpu = light->ToGPULight();

    ExpectVec4Near(gpu.position, glm::vec4(1.0f, 0.0f, 0.0f, 2.0f));    // w = LightType::Spot (2)
    ExpectVec4Near(gpu.direction, glm::vec4(0.0f, -1.0f, 0.0f, 1.5f));  // xyz = direction, w = intensity
    ExpectVec4Near(gpu.color, glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
    ExpectVec4Near(gpu.params, glm::vec4(1.0f, 0.09f, 0.032f, 1.0f));   // w = innerCutoff = cos(0°)
    ExpectVec4Near(gpu.params2, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));     // x = outerCutoff = cos(90°)
}
