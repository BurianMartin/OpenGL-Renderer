#include "Forge/Lights.hpp"

namespace Forge
{
    std::shared_ptr<DirectionalLight> DirectionalLight::Create(glm::vec3 direction, glm::vec3 color, float intensity)
    {
        try
        {
            return std::shared_ptr<DirectionalLight>(new DirectionalLight(direction, color, intensity));
        }
        catch (const std::exception &e)
        {
            debug_warn("DirectionalLight creation failed: " << e.what());
            return nullptr;
        }
    }

    DirectionalLight::DirectionalLight(glm::vec3 direction, glm::vec3 color, float intensity)
        : Light(color, intensity), direction_(glm::normalize(direction))
    {
    }

    glm::vec3 DirectionalLight::GetDirection() const { return direction_; }

    void DirectionalLight::SetDirection(glm::vec3 direction) { direction_ = glm::normalize(direction); }

    GPULight DirectionalLight::ToGPULight() const
    {
        GPULight gpul;

        gpul.position = glm::vec4(0.0f, 0.0f, 0.0f, static_cast<float>(GetLightType())); // xyz = position; w = light type (0=dir,1=point,2=spot)
        gpul.direction = glm::vec4(direction_, GetIntensity());                          // xyz = direction; w = intensity
        gpul.color = glm::vec4(GetColor(), 0.0f);                                        // rgb = color; a unused
        gpul.params = glm::vec4(0.0f);                                                   // x=constant, y=linear, z=quadratic, w=innerCutoff
        gpul.params2 = glm::vec4(0.0f);                                                  // x=outerCutoff, yzw unused/padding
        return gpul;
    }

    std::shared_ptr<PointLight> PointLight::Create(glm::vec3 position, glm::vec3 color, float intensity,
                                                   float constant, float linear, float quadratic)
    {
        try
        {
            return std::shared_ptr<PointLight>(new PointLight(position, color, intensity, constant, linear, quadratic));
        }
        catch (const std::exception &e)
        {
            debug_warn("PointLight creation failed: " << e.what());
            return nullptr;
        }
    }

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

    GPULight PointLight::ToGPULight() const
    {
        GPULight gpul;

        gpul.position = glm::vec4(position_, static_cast<float>(GetLightType())); // xyz = position; w = light type (0=dir,1=point,2=spot)
        gpul.direction = glm::vec4(0.0f, 0.0f, 0.0f, GetIntensity());             // xyz = direction; w = intensity
        gpul.color = glm::vec4(GetColor(), 0.0f);                                 // rgb = color; a unused
        gpul.params = glm::vec4(constant_, linear_, quadratic_, 0.0f);            // x=constant, y=linear, z=quadratic, w=innerCutoff
        gpul.params2 = glm::vec4(0.0f);                                           // x=outerCutoff, yzw unused/padding
        return gpul;
    }

    std::shared_ptr<SpotLight> SpotLight::Create(glm::vec3 position, glm::vec3 direction, glm::vec3 color, float intensity,
                                                 float innerCutoff, float outerCutoff,
                                                 float constant, float linear, float quadratic)
    {
        try
        {
            return std::shared_ptr<SpotLight>(new SpotLight(position, direction, color, intensity,
                                                             innerCutoff, outerCutoff, constant, linear, quadratic));
        }
        catch (const std::exception &e)
        {
            debug_warn("SpotLight creation failed: " << e.what());
            return nullptr;
        }
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

    GPULight SpotLight::ToGPULight() const
    {
        GPULight gpul;

        gpul.position = glm::vec4(position_, static_cast<float>(GetLightType())); // xyz = position; w = light type (0=dir,1=point,2=spot)
        gpul.direction = glm::vec4(direction_, GetIntensity());                   // xyz = direction; w = intensity
        gpul.color = glm::vec4(GetColor(), 0.0f);                                 // rgb = color; a unused
        gpul.params = glm::vec4(constant_, linear_, quadratic_, innerCutoff_);    // x=constant, y=linear, z=quadratic, w=innerCutoff
        gpul.params2 = glm::vec4(outerCutoff_, 0.0f, 0.0f, 0.0f);                 // x=outerCutoff, yzw unused/padding
        return gpul;
    }
} // namespace Forge
