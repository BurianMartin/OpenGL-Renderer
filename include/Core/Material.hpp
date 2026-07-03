#pragma once
#include "Utils.hpp"
#include "Core/Shader.hpp"
#include "Core/Texture.hpp"
#include "Core/RenderContext.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <memory>
#include <string>

namespace Core
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

        // --- Classic-OpenGL Blinn-Phong material presets ---
        static std::shared_ptr<Material> Gold(std::shared_ptr<Shader> shader);
        static std::shared_ptr<Material> Silver(std::shared_ptr<Shader> shader);
        static std::shared_ptr<Material> Bronze(std::shared_ptr<Shader> shader);
        static std::shared_ptr<Material> Chrome(std::shared_ptr<Shader> shader);
        static std::shared_ptr<Material> Emerald(std::shared_ptr<Shader> shader);
        static std::shared_ptr<Material> Ruby(std::shared_ptr<Shader> shader);
        static std::shared_ptr<Material> Pearl(std::shared_ptr<Shader> shader);
        static std::shared_ptr<Material> Obsidian(std::shared_ptr<Shader> shader);
        static std::shared_ptr<Material> Light(std::shared_ptr<Shader> shader);
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
        void Bind(std::shared_ptr<RenderContext> ctx) const;

        /// Sets the base-color/diffuse texture, sampled in place of `uBaseColor` when present.
        void SetTexture(std::shared_ptr<Texture> texture);

        /// @return True if a texture has been set via SetTexture().
        bool HasTexture() const;

        /// Flat base color. In `fragment.glsl` this is used only when no texture is set — a set texture replaces it entirely rather than blending.
        void SetColor(const glm::vec4 &color);
        void SetAmbient(const glm::vec3 &ambient);
        void SetDiffuse(const glm::vec3 &diffuse);
        void SetSpecular(const glm::vec3 &specular);
        void SetShininess(float shininess);

        /// @return The tag this material was created with (used for lookup via `std::ranges::find_if` in Layer).
        std::string GetTag() const;

        /// @return The underlying Shader, for setting per-model uniforms (e.g. `uModel`) that Bind() doesn't cover.
        std::shared_ptr<Shader> GetShader() const;

    private:
        Material(std::shared_ptr<Shader> shader, const std::string &tag);

        std::shared_ptr<Shader> shader_;
        std::shared_ptr<Texture> texture_;
        std::string tag_;

        glm::vec3 ambient_;
        glm::vec3 diffuse_;
        glm::vec3 specular_;
        glm::vec4 color_;
        float shininess_;
    };
} // namespace Core
