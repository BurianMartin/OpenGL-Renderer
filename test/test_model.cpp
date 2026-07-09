#include <gtest/gtest.h>
#include "Forge/Model.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace
{
    void ExpectMat4Near(const glm::mat4 &actual, const glm::mat4 &expected, float epsilon = 1e-5f)
    {
        for (int col = 0; col < 4; ++col)
        {
            for (int row = 0; row < 4; ++row)
            {
                EXPECT_NEAR(actual[col][row], expected[col][row], epsilon);
            }
        }
    }

    void ExpectVec3Near(const glm::vec3 &actual, const glm::vec3 &expected, float epsilon = 1e-5f)
    {
        EXPECT_NEAR(actual.x, expected.x, epsilon);
        EXPECT_NEAR(actual.y, expected.y, epsilon);
        EXPECT_NEAR(actual.z, expected.z, epsilon);
    }
}

// Model's constructor only stores the Mesh pointer - GetModelMatrix()/SetPosition()/SetScale()/
// SetRotation() never dereference it - so passing nullptr here is safe and avoids needing a GL
// context just to test pure transform-composition math.

TEST(ModelTest, DefaultModelMatrixIsIdentity)
{
    Forge::Model model(nullptr);
    ExpectMat4Near(model.GetModelMatrix(), glm::mat4(1.0f));
}

TEST(ModelTest, TranslationOnlyProducesTranslationMatrix)
{
    Forge::Model model(nullptr);
    model.SetPosition(glm::vec3(2.0f, 3.0f, 4.0f));
    glm::mat4 expected = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 3.0f, 4.0f));
    ExpectMat4Near(model.GetModelMatrix(), expected);
}

TEST(ModelTest, UniformScaleOnlyProducesScaleMatrix)
{
    Forge::Model model(nullptr);
    model.SetScale(2.0f);
    glm::mat4 expected = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f));
    ExpectMat4Near(model.GetModelMatrix(), expected);
}

// The model matrix must apply scale BEFORE rotation (in local space), not after - getting this
// backwards is a real, easy mistake (swapping two lines) that "looks fine" for uniform scale
// but visibly skews anything non-uniformly scaled. A 2x scale on X only, then a 90° rotation
// about Z, moves local point (1,0,0) to (0,2,0): scale first gives (2,0,0), then a 90° rotation
// about Z (which maps (x,y) -> (-y,x)) gives (0,2,0). If rotation ran first instead, (1,0,0)
// would rotate to (0,1,0), and scaling X afterward wouldn't touch it at all - landing on
// (0,1,0), an easily distinguishable wrong answer.
TEST(ModelTest, ScaleIsAppliedBeforeRotationInLocalSpace)
{
    Forge::Model model(nullptr);
    model.SetScale(glm::vec3(2.0f, 1.0f, 1.0f));
    model.SetRotation(glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::vec4 transformed = model.GetModelMatrix() * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    ExpectVec3Near(glm::vec3(transformed), glm::vec3(0.0f, 2.0f, 0.0f));
}
