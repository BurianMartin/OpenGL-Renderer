#pragma once

#include "Utils.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Core
{
    enum class LightType
    {
        Directional = 0,
        Point,
        Spot,
        Area
    };

#define LIGHT_CLASS_TYPE(type)                                                  \
    static LightType GetStaticType() { return LightType::type; }                \
    virtual LightType GetLightType() const override { return GetStaticType(); } \
    virtual const char *GetName() const override { return #type; }

    class Light
    {
    private:
        glm::vec3 color_;
        float intensity_;

    public:
        Light(glm::vec3 color = glm::vec3(1.0f), float intensity = 1.0f) : color_(color), intensity_(intensity) {}
        virtual ~Light() = default;

        virtual LightType GetLightType() const = 0;
        virtual const char *GetName() const = 0;
        virtual std::string ToString() const { return GetName(); }

    protected:
        glm::vec3 GetColor() const { return color_; }
        float GetIntensity() const { return intensity_; }
    };
} // namespace Core
