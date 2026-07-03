#include "Core/Lights.hpp"

namespace Core
{
    DirectionalLight::DirectionalLight(glm::vec3 direction, glm::vec3 color, float intensity)
        : Light(color, intensity), direction_(glm::normalize(direction))
    {
    }

    glm::vec3 DirectionalLight::GetDirection() const { return direction_; }

    void DirectionalLight::SetDirection(glm::vec3 direction) { direction_ = glm::normalize(direction); }

    PointLight::PointLight(glm::vec3 position, glm::vec3 color, float intensity,
                           float constant, float linear, float quadratic)
        : Light(color, intensity), position_(position),
          constant_(constant), linear_(linear), quadratic_(quadratic)
    {
    }

    glm::vec3 PointLight::GetPosition() const { return position_; }
    float PointLight::GetConstant() const { return constant_; }
    float PointLight::GetLinear() const { return linear_; }
    float PointLight::GetQuadratic() const { return quadratic_; }

    void PointLight::SetPosition(glm::vec3 position) { position_ = position; }
    void PointLight::SetAttenuation(float constant, float linear, float quadratic)
    {
        constant_ = constant;
        linear_ = linear;
        quadratic_ = quadratic;
    }

    SpotLight::SpotLight(glm::vec3 position, glm::vec3 direction, glm::vec3 color, float intensity,
                         float innerCutoff, float outerCutoff,
                         float constant, float linear, float quadratic)
        : Light(color, intensity), position_(position), direction_(glm::normalize(direction)),
          constant_(constant), linear_(linear), quadratic_(quadratic),
          innerCutoff_(glm::cos(glm::radians(innerCutoff))),
          outerCutoff_(glm::cos(glm::radians(outerCutoff)))
    {
    }

    glm::vec3 SpotLight::GetPosition() const { return position_; }
    glm::vec3 SpotLight::GetDirection() const { return direction_; }
    float SpotLight::GetConstant() const { return constant_; }
    float SpotLight::GetLinear() const { return linear_; }
    float SpotLight::GetQuadratic() const { return quadratic_; }
    float SpotLight::GetInnerCutoff() const { return innerCutoff_; }
    float SpotLight::GetOuterCutoff() const { return outerCutoff_; }

    void SpotLight::SetPosition(glm::vec3 position) { position_ = position; }
    void SpotLight::SetDirection(glm::vec3 direction) { direction_ = glm::normalize(direction); }
    void SpotLight::SetAttenuation(float constant, float linear, float quadratic)
    {
        constant_ = constant;
        linear_ = linear;
        quadratic_ = quadratic;
    }
} // namespace Core
