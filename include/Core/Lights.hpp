#pragma once

#include "Utils.hpp"
#include "Core/Light.hpp"

#include <memory>

namespace Core
{
    /// A light with a direction but no position — illuminates every fragment equally regardless of distance (e.g. sunlight).
    class DirectionalLight : public Light
    {
    public:
        /**
         * @brief Creates a DirectionalLight.
         * @param direction Normalized internally, so callers don't need to pre-normalize it.
         * @param color Light color.
         * @param intensity Scalar multiplier applied to `color` in the lighting equation.
         * @return A new DirectionalLight, or `nullptr` on failure.
         */
        static std::shared_ptr<DirectionalLight> Create(glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f),
                                                        glm::vec3 color = glm::vec3(1.0f),
                                                        float intensity = 1.0f);

        LIGHT_CLASS_TYPE(Directional)

        glm::vec3 GetDirection() const;

        /// @param direction Normalized internally.
        void SetDirection(glm::vec3 direction);

        virtual GPULight ToGPULight() const override;

    private:
        DirectionalLight(glm::vec3 direction, glm::vec3 color, float intensity);

        glm::vec3 direction_;
    };

    /// A light at a fixed world position that falls off with distance (constant/linear/quadratic attenuation), radiating in all directions.
    class PointLight : public Light
    {
    public:
        /**
         * @brief Creates a PointLight.
         * @param position World-space position the light radiates from.
         * @param color Light color.
         * @param intensity Scalar multiplier applied to `color` in the lighting equation.
         * @param constant,linear,quadratic Attenuation coefficients; the defaults approximate a ~50-unit effective range.
         * @return A new PointLight, or `nullptr` on failure.
         */
        static std::shared_ptr<PointLight> Create(glm::vec3 position = glm::vec3(0.0f),
                                                  glm::vec3 color = glm::vec3(1.0f),
                                                  float intensity = 1.0f,
                                                  float constant = 1.0f, float linear = 0.09f, float quadratic = 0.032f);

        LIGHT_CLASS_TYPE(Point)

        glm::vec3 GetPosition() const;
        float GetConstant() const;
        float GetLinear() const;
        float GetQuadratic() const;

        void SetPosition(glm::vec3 position);
        void SetAttenuation(float constant, float linear, float quadratic);

        virtual GPULight ToGPULight() const override;

    private:
        PointLight(glm::vec3 position, glm::vec3 color, float intensity,
                  float constant, float linear, float quadratic);

        glm::vec3 position_;
        float constant_, linear_, quadratic_;
    };

    /// A PointLight further restricted to a cone: only fragments within `outerCutoff` of `direction` are lit, with smooth falloff between `innerCutoff` and `outerCutoff` (e.g. a flashlight/spotlight).
    class SpotLight : public Light
    {
    public:
        /**
         * @brief Creates a SpotLight.
         * @param position World-space position the light radiates from.
         * @param direction Normalized internally.
         * @param color Light color.
         * @param intensity Scalar multiplier applied to `color` in the lighting equation.
         * @param innerCutoff Cone half-angle in degrees within which light is at full strength.
         * @param outerCutoff Cone half-angle in degrees beyond which light is zero; stored internally as `cos(radians(outerCutoff))` via GetOuterCutoff(), matching the shader-side comparison.
         * @param constant,linear,quadratic Attenuation coefficients; see PointLight for the same fields.
         * @return A new SpotLight, or `nullptr` on failure.
         */
        static std::shared_ptr<SpotLight> Create(glm::vec3 position = glm::vec3(0.0f),
                                                 glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f),
                                                 glm::vec3 color = glm::vec3(1.0f),
                                                 float intensity = 1.0f,
                                                 float innerCutoff = 12.5f, float outerCutoff = 17.5f,
                                                 float constant = 1.0f, float linear = 0.09f, float quadratic = 0.032f);

        LIGHT_CLASS_TYPE(Spot)

        glm::vec3 GetPosition() const;
        glm::vec3 GetDirection() const;
        float GetConstant() const;
        float GetLinear() const;
        float GetQuadratic() const;

        /// @return `cos(radians(innerCutoff))`, not the raw degree value passed to the constructor.
        float GetInnerCutoff() const;

        /// @return `cos(radians(outerCutoff))`, not the raw degree value passed to the constructor.
        float GetOuterCutoff() const;

        void SetPosition(glm::vec3 position);

        /// @param direction Normalized internally.
        void SetDirection(glm::vec3 direction);
        void SetAttenuation(float constant, float linear, float quadratic);

        virtual GPULight ToGPULight() const override;

    private:
        SpotLight(glm::vec3 position, glm::vec3 direction, glm::vec3 color, float intensity,
                 float innerCutoff, float outerCutoff,
                 float constant, float linear, float quadratic);

        glm::vec3 position_;
        glm::vec3 direction_;
        float constant_, linear_, quadratic_;
        float innerCutoff_, outerCutoff_;
    };
} // namespace Core
