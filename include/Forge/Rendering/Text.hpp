#pragma once
#include "Utils.hpp"
#include "Forge/Rendering/Font.hpp"
#include "Forge/Rendering/Shader.hpp"
#include "Forge/Core/ResourceManager.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <memory>
#include <string>

namespace Forge
{
    /**
     * @brief A drawable, screen-space text string: a Font, a baked glyph-quad mesh, a
     * screen position, and a color.
     *
     * Deliberately bypasses the normal Material/Layer::materialModels_ render dispatch.
     * Material::Bind() always uploads the *active camera's* view/projection from
     * FrameContext — there's no hook for "use a fixed pixel-space orthographic projection
     * instead," which is what screen-space text needs regardless of whatever 3D camera the
     * scene is using. So Draw() drives its own shader/texture/GL state directly instead —
     * the same pattern Scene::DrawSkyboxBackground()/DrawSkydomeBackground() already use
     * for their own non-standard rendering needs.
     *
     * Construction is private — use Create(). The glyph-quad mesh is rebuilt (not patched)
     * on every SetString()/SetPosition() call — simple, and cheap at UI-string lengths.
     */
    class Text
    {
    public:
        /**
         * @param rmanager Used to load (and cache) the shared text shader — one compile for
         * every Text in the process, not one per instance.
         * @param font Baked font to render with; must outlive this Text.
         * @param text Initial string to display.
         * @param x,y Screen position, in pixels, of the string's baseline-left origin
         * (origin top-left, y grows downward — same convention as MouseMovedEvent/click-to-pick).
         * @param color RGBA, 0..1 range.
         * @return A new Text, or `nullptr` if the shared text shader failed to load.
         */
        static std::shared_ptr<Text> Create(std::shared_ptr<ResourceManager> rmanager, std::shared_ptr<Font> font,
                                             const std::string &text, float x, float y,
                                             const glm::vec4 &color = glm::vec4(1.0f));

        ~Text();
        Text(const Text &) = delete;
        Text &operator=(const Text &) = delete;

        /// Replaces the displayed string, rebuilding the glyph-quad mesh.
        void SetString(const std::string &text);
        /// Moves the string's origin, rebuilding the glyph-quad mesh.
        void SetPosition(float x, float y);
        /// Flat RGBA color, modulated by each glyph's coverage — no mesh rebuild needed.
        void SetColor(const glm::vec4 &color);

        /// @return This string's on-screen width/height in pixels at its baked font size (e.g. for centering).
        glm::vec2 GetSize() const { return size_; }

        /// @return The top-left corner of this string's actual rendered glyph bounding box,
        /// in the same screen-space pixel coordinates as Create()'s x/y -- NOT the same point
        /// as (x, y) itself, since (x, y) is the baseline-left origin (see Create's doc) and
        /// ascenders/capitals render above the baseline. A caller building a box AROUND the
        /// visible text (e.g. Button's ClickableRegion/background) needs this, not (x, y)
        /// directly, or the box sits too low and tall glyphs poke out its top edge.
        glm::vec2 GetTopLeft() const { return topLeft_; }

        /**
         * @brief Draws this string.
         *
         * For the duration of this call: disables depth testing (text always draws on top
         * of the 3D scene, regardless of draw order) and enables blending (glyph edges are
         * anti-aliased via the atlas's alpha channel) — both restored to their prior state
         * before returning, mirroring Scene::DrawSkyboxBackground()'s save/restore dance
         * around its own special-case GL state.
         * @param windowWidth,windowHeight Current window size in pixels — used to build the
         * fixed screen-space orthographic projection this draw needs (not the scene's own
         * camera projection).
         */
        void Draw(GLint windowWidth, GLint windowHeight) const;

    private:
        Text(std::shared_ptr<Font> font, std::shared_ptr<Shader> shader, const glm::vec4 &color);

        void Rebuild();
        void Destroy();

        std::shared_ptr<Font> font_;
        std::shared_ptr<Shader> shader_;
        glm::vec4 color_;
        std::string text_;
        float x_ = 0.0f;
        float y_ = 0.0f;
        glm::vec2 size_{0.0f, 0.0f};
        glm::vec2 topLeft_{0.0f, 0.0f};

        GLuint VAO_ = 0, VBO_ = 0, EBO_ = 0;
        unsigned int indexCount_ = 0;
    };
} // namespace Forge
