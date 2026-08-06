#pragma once
#include "Utils.hpp"
#include "Forge/Rendering/Shader.hpp"
#include "Forge/Rendering/Texture.hpp"
#include "Forge/Core/ResourceManager.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <memory>

namespace Forge
{
    /**
     * @brief A drawable, screen-space colored rectangle with an optional border — Forge's
     * generic screen-space quad primitive. This was a real, long-standing gap: `Button`'s
     * own class doc has said since it was written that "this engine has no generic
     * screen-space quad primitive yet" to draw a background/border with, so a Button was
     * just a label with an invisible click target around it. `Panel` fills that gap.
     *
     * Same bypass-Material/Layer::materialModels_ pattern as `Text` (see its own class doc)
     * — a fixed pixel-space orthographic projection has no natural home in the normal
     * Material::Bind() uniform set, so Draw() drives its own shader/GL state directly
     * instead. Construction is private — use Create(). The border is drawn by the fragment
     * shader from a single quad (no separate border geometry) — `borderThicknessPx` of 0
     * (the default) means no border at all, `fillColor` alone covers the whole rect.
     *
     * Fill is a vertical gradient, not a flat color, drawn by the fragment shader as
     * `mix(top, bottom, vLocal.y)` — a single flat `fillColor` (the `Create`/`SetFillColor`
     * overloads that take one color) is auto-shaded into a subtle top-lighter/bottom-darker
     * pair (see `AutoShade` in the .cpp) so every existing caller gets a surface that reads
     * as a card/panel instead of a paint swatch with zero call-site changes; `SetFillGradient`
     * exists for a caller that wants the two tones explicit instead (e.g. a domain accent).
     * Optional texture (card art): `SetTexture` gives the Panel a `Forge::Texture` to sample
     * instead of the flat/gradient fill — mirroring the exact `uHasTexture`/`uTexture`
     * boolean-gated convention `fragment.glsl`/`Material` already use for 3D models, not a
     * new pattern. A textured Panel is tinted by its top fill color alone (no gradient over
     * the art); `SetTexture(nullptr)` (also the default — every existing caller) reverts to
     * today's flat/gradient behavior. Rounded corners and the border apply on top either way.
     * Corners are rounded via a signed-distance-field test in the fragment shader keyed off
     * `cornerRadiusPx` (0, the default, is a plain sharp-cornered rect — fully opt-in, same
     * convention as `borderThicknessPx`); the border's own edge follows the same rounded
     * silhouette. Both the rounding test and the border thickness are computed in real pixel
     * units off `uSizePx`, not the 0..1 `vLocal` fraction, so neither one goes anisotropic on
     * a non-square Panel the way a UV-fraction border used to.
     */
    class Panel
    {
    public:
        /// @param x,y Top-left corner, screen-space pixels (origin top-left, y down — same
        /// convention as Text/ClickableRegion/MouseButtonEvent).
        /// @param cornerRadiusPx 0 (default) is a plain sharp-cornered rect.
        /// @return A new Panel, or `nullptr` if the shared panel shader failed to load.
        static std::shared_ptr<Panel> Create(std::shared_ptr<ResourceManager> rmanager, float x, float y,
                                              float width, float height,
                                              const glm::vec4 &fillColor = glm::vec4(1.0f),
                                              const glm::vec4 &borderColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
                                              float borderThicknessPx = 0.0f, float cornerRadiusPx = 0.0f);

        ~Panel();
        Panel(const Panel &) = delete;
        Panel &operator=(const Panel &) = delete;

        /// Moves the rect's top-left corner, keeping its current size. Rebuilds the quad.
        void SetPosition(float x, float y);
        /// Resizes in place (top-left corner unchanged). Rebuilds the quad.
        void SetSize(float width, float height);
        /// Repositions and resizes in one rebuild.
        void SetRect(float x, float y, float width, float height);

        /// Auto-shades `color` into a subtle top/bottom gradient — see the class doc.
        void SetFillColor(const glm::vec4 &color);
        /// Sets the top/bottom gradient colors explicitly, no auto-shading.
        void SetFillGradient(const glm::vec4 &topColor, const glm::vec4 &bottomColor)
        {
            fillColorTop_ = topColor;
            fillColorBottom_ = bottomColor;
        }
        void SetBorderColor(const glm::vec4 &color) { borderColor_ = color; }
        /// 0 disables the border entirely.
        void SetBorderThickness(float px) { borderThicknessPx_ = px; }
        /// 0 (the default) is a plain sharp-cornered rect.
        void SetCornerRadius(float px) { cornerRadiusPx_ = px; }
        /// `nullptr` (the default) disables texturing -- flat/gradient fill instead. See the
        /// class doc's "Optional texture" note.
        void SetTexture(std::shared_ptr<Texture> texture) { texture_ = std::move(texture); }

        float GetX() const { return x_; }
        float GetY() const { return y_; }
        float GetWidth() const { return width_; }
        float GetHeight() const { return height_; }

        /// @param windowWidth,windowHeight Current window size in pixels — used to build the
        /// fixed screen-space orthographic projection this draw needs (not the scene's own
        /// camera projection), same as Text::Draw.
        void Draw(GLint windowWidth, GLint windowHeight) const;

    private:
        Panel(std::shared_ptr<Shader> shader, const glm::vec4 &fillColor, const glm::vec4 &borderColor,
              float borderThicknessPx, float cornerRadiusPx);

        void Rebuild();
        void Destroy();

        std::shared_ptr<Shader> shader_;
        glm::vec4 fillColorTop_;
        glm::vec4 fillColorBottom_;
        glm::vec4 borderColor_;
        float borderThicknessPx_;
        float cornerRadiusPx_;
        std::shared_ptr<Texture> texture_;
        float x_ = 0.0f, y_ = 0.0f, width_ = 0.0f, height_ = 0.0f;

        GLuint VAO_ = 0, VBO_ = 0, EBO_ = 0;
    };
} // namespace Forge
