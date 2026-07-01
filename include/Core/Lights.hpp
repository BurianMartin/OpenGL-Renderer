#pragma once

#include "Utils.hpp"
#include "Core/Light.hpp"

namespace Core
{
    class DirectionalLight : public Light
    {
    public:
        DirectionalLight(glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f),
                         glm::vec3 color = glm::vec3(1.0f),
                         float intensity = 1.0f)
            : Light(color, intensity), direction_(glm::normalize(direction)) {}

        LIGHT_CLASS_TYPE(Directional)

        glm::vec3 GetDirection() const { return direction_; }
        void SetDirection(glm::vec3 direction) { direction_ = glm::normalize(direction); }

    private:
        glm::vec3 direction_;
    };

    class PointLight : public Light
    {
    public:
        PointLight(glm::vec3 position = glm::vec3(0.0f),
                   glm::vec3 color = glm::vec3(1.0f),
                   float intensity = 1.0f,
                   float constant = 1.0f, float linear = 0.09f, float quadratic = 0.032f)
            : Light(color, intensity), position_(position),
              constant_(constant), linear_(linear), quadratic_(quadratic) {}

        LIGHT_CLASS_TYPE(Point)

        glm::vec3 GetPosition() const { return position_; }
        float GetConstant() const { return constant_; }
        float GetLinear() const { return linear_; }
        float GetQuadratic() const { return quadratic_; }

        void SetPosition(glm::vec3 position) { position_ = position; }
        void SetAttenuation(float constant, float linear, float quadratic)
        {
            constant_ = constant;
            linear_ = linear;
            quadratic_ = quadratic;
        }

    private:
        glm::vec3 position_;
        float constant_, linear_, quadratic_;
    };

    class SpotLight : public Light
    {
    public:
        SpotLight(glm::vec3 position = glm::vec3(0.0f),
                  glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f),
                  glm::vec3 color = glm::vec3(1.0f),
                  float intensity = 1.0f,
                  float innerCutoff = 12.5f, float outerCutoff = 17.5f,
                  float constant = 1.0f, float linear = 0.09f, float quadratic = 0.032f)
            : Light(color, intensity), position_(position), direction_(glm::normalize(direction)),
              constant_(constant), linear_(linear), quadratic_(quadratic),
              innerCutoff_(glm::cos(glm::radians(innerCutoff))),
              outerCutoff_(glm::cos(glm::radians(outerCutoff))) {}

        LIGHT_CLASS_TYPE(Spot)

        glm::vec3 GetPosition() const { return position_; }
        glm::vec3 GetDirection() const { return direction_; }
        float GetConstant() const { return constant_; }
        float GetLinear() const { return linear_; }
        float GetQuadratic() const { return quadratic_; }
        float GetInnerCutoff() const { return innerCutoff_; }
        float GetOuterCutoff() const { return outerCutoff_; }

        void SetPosition(glm::vec3 position) { position_ = position; }
        void SetDirection(glm::vec3 direction) { direction_ = glm::normalize(direction); }
        void SetAttenuation(float constant, float linear, float quadratic)
        {
            constant_ = constant;
            linear_ = linear;
            quadratic_ = quadratic;
        }

    private:
        glm::vec3 position_;
        glm::vec3 direction_;
        float constant_, linear_, quadratic_;
        float innerCutoff_, outerCutoff_;
    };
} // namespace Core
