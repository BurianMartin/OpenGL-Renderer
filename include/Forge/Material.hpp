#pragma once
#include "Utils.hpp"
#include "Forge/Shader.hpp"
#include "Forge/Texture.hpp"
#include "Forge/FrameContext.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <memory>
#include <string>

namespace Forge
{
    /**
     * @brief A shader plus the Blinn-Phong surface parameters and optional
     * texture bound to it — the unit Layer dispatches rendering by.
     *
     * Construction is private — use `Create`, or one of the classic-OpenGL
     * Blinn-Phong presets below (`Gold`, `Silver`, etc.), each of which
     * builds a Material on the given shader with pre-set ambient/diffuse/
     * specular/shininess values. `Bind` uploads every uniform this class
     * owns; per-model uniforms (`uModel`) are the caller's responsibility
     * via `GetShader()`.
     */
    class Material
    {
    public:
        /// @return A new Material with default Blinn-Phong values (ambient 0.1, diffuse 1.0, specular 0.5, shininess 32, tag as given), or `nullptr` on failure.
        static std::shared_ptr<Material> Create(std::shared_ptr<Shader> shader, const std::string &tag);

        static std::shared_ptr<Material> Create(std::shared_ptr<Shader> shader, const std::string &tag,
                                                glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float shininess);

        // --- Classic-OpenGL Blinn-Phong material presets ---
        // Fixed ambient/diffuse/specular/shininess values from the well-known classic-OpenGL
        // material reference chart, applied via SetAmbient/SetDiffuse/SetSpecular/SetShininess.

        /// Warm yellow metal, high specular response (shininess 51.2).
        static std::shared_ptr<Material> Gold(std::shared_ptr<Shader> shader);
        /// Neutral grey metal, high specular response (shininess 51.2).
        static std::shared_ptr<Material> Silver(std::shared_ptr<Shader> shader);
        /// Warm brown-orange metal, moderate specular response (shininess 25.6).
        static std::shared_ptr<Material> Bronze(std::shared_ptr<Shader> shader);
        /// Bright neutral metal with a strong, tight highlight (shininess 76.8).
        static std::shared_ptr<Material> Chrome(std::shared_ptr<Shader> shader);
        /// Saturated green gemstone with a strong, tight highlight (shininess 76.8).
        static std::shared_ptr<Material> Emerald(std::shared_ptr<Shader> shader);
        /// Saturated red gemstone with a strong, tight highlight (shininess 76.8).
        static std::shared_ptr<Material> Ruby(std::shared_ptr<Shader> shader);
        /// Pale pink, low-shininess, soft highlight (shininess 11.264) — mimics a matte gemstone.
        static std::shared_ptr<Material> Pearl(std::shared_ptr<Shader> shader);
        /// Very dark, low-reflectance volcanic glass (shininess 38.4).
        static std::shared_ptr<Material> Obsidian(std::shared_ptr<Shader> shader);
        /// Flat, near-white, maximum specular tightness (shininess 128) — intended for the visual marker of a light source itself, not a lit surface.
        static std::shared_ptr<Material> Light(std::shared_ptr<Shader> shader);
        /// Warm orange-red metal, low specular response (shininess 12.8).
        static std::shared_ptr<Material> Copper(std::shared_ptr<Shader> shader);
        ~Material() = default;

        /**
         * @brief Activates the shader and uploads every uniform this material owns.
         *
         * Sets `uView`/`uProjection`/`uCameraPos`/`uTime` from `ctx`, the
         * `uMaterial.*` Blinn-Phong fields, `uBaseColor`, `uHasTexture`,
         * and binds the texture to slot 0 if one is set. Call once per
         * material before drawing every Model in its bucket — `uModel`
         * still needs to be set per-model afterward via `GetShader()`.
         */
        void Bind(std::shared_ptr<FrameContext> ctx) const;

        /// Sets the base-color/diffuse texture, sampled in place of `uBaseColor` when present.
        void SetTexture(std::shared_ptr<Texture> texture);
        /// Sets the specular map, bound to texture unit 1 (unit 0 is reserved for the base-color texture) when present.
        void SetSpecularTexture(std::shared_ptr<Texture> tex);

        /// @return True if a texture has been set via SetTexture().
        bool HasTexture() const;
        /// @return True if a specular map has been set via SetSpecularTexture().
        bool HasSpecularMap() const;

        /// Flat base color. In `fragment.glsl` this is used only when no texture is set — a set texture replaces it entirely rather than blending.
        void SetColor(const glm::vec4 &color);
        /// Blinn-Phong ambient reflectance — the color/intensity contributed regardless of any light direction.
        void SetAmbient(const glm::vec3 &ambient);
        /// Blinn-Phong diffuse reflectance — the main lit-surface color, scaled by the light/normal angle.
        void SetDiffuse(const glm::vec3 &diffuse);
        /// Blinn-Phong specular reflectance — the highlight color, scaled by `shininess` and the view/half-vector angle.
        void SetSpecular(const glm::vec3 &specular);
        /// Blinn-Phong shininess exponent — higher values produce a smaller, tighter specular highlight.
        void SetShininess(float shininess);

        /// @return The tag this material was created with (used for lookup via `std::ranges::find_if` in Layer).
        std::string GetTag() const;

        /// @return The underlying Shader, for setting per-model uniforms (e.g. `uModel`) that Bind() doesn't cover.
        std::shared_ptr<Shader> GetShader() const;

    private:
        Material(std::shared_ptr<Shader> shader, const std::string &tag);

        std::shared_ptr<Shader> shader_;
        std::shared_ptr<Texture> texture_;
        std::shared_ptr<Texture> specularTexture_;
        std::string tag_;

        glm::vec3 ambient_;
        glm::vec3 diffuse_;
        glm::vec3 specular_;
        glm::vec4 color_;
        float shininess_;
    };
} // namespace Forge
