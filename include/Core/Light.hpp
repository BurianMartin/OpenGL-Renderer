#pragma once

#include "Utils.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Core
{
    /// Discriminant for every concrete Light subclass. @note `Area` is reserved — there is currently no AreaLight class.
    enum class LightType
    {
        Directional = 0,
        Point,
        Spot,
        Area
    };

    struct GPULight
    {
        glm::vec4 position;  // xyz = position; w = light type (0=dir,1=point,2=spot)
        glm::vec4 direction; // xyz = direction; w = intensity
        glm::vec4 color;     // rgb = color; a unused
        glm::vec4 params;    // x=constant, y=linear, z=quadratic, w=innerCutoff
        glm::vec4 params2;   // x=outerCutoff, yzw unused/padding
    };

    /**
     * @brief Generates the boilerplate RTTI-style overrides every concrete
     * Light subclass needs (mirrors `EVENT_CLASS_TYPE`).
     *
     * Invoke inside the class body with the matching `LightType` enumerator
     * name, e.g. `LIGHT_CLASS_TYPE(Point)`. Defines `GetStaticType()`,
     * `GetLightType()`, and `GetName()`.
     */
#define LIGHT_CLASS_TYPE(type)                                                  \
    static LightType GetStaticType() { return LightType::type; }                \
    virtual LightType GetLightType() const override { return GetStaticType(); } \
    virtual const char *GetName() const override { return #type; }

    /**
     * @brief Abstract base for every light type (DirectionalLight, PointLight, SpotLight — see Lights.hpp).
     *
     * Holds only what every light has in common: a color and an intensity
     * scalar. Subclasses add position/direction/attenuation as needed and
     * are added to a Scene via `Scene::AddLight`.
     */
    class Light
    {
    private:
        glm::vec3 color_;
        float intensity_;

    public:
        Light(glm::vec3 color = glm::vec3(1.0f), float intensity = 1.0f) : color_(color), intensity_(intensity) {}
        virtual ~Light() = default;

        /// @return The concrete type of this light, for switch-based dispatch.
        virtual LightType GetLightType() const = 0;
        virtual const char *GetName() const = 0;
        virtual std::string ToString() const { return GetName(); }

        virtual GPULight ToGPULight() const = 0;

    protected:
        glm::vec3 GetColor() const { return color_; }
        float GetIntensity() const { return intensity_; }
    };
} // namespace Core
