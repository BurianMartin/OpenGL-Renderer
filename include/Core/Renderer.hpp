#pragma once
#include "Utils.hpp"
#include "Core/Scene.hpp"
#include "Core/RenderContext.hpp"

#include <memory>

namespace Core
{
    /**
     * @brief Drives one frame of drawing for the active Scene.
     *
     * Owns the single RenderContext shared with every Scene/Layer/Material
     * this frame. Application calls `RenderScene` once per frame after
     * `Update`.
     */
    class Renderer
    {
    private:
        std::shared_ptr<RenderContext> rctx_;

    public:
        /// @param aspect_ratio Initial viewport aspect ratio, stored in the RenderContext.
        Renderer(GLfloat aspect_ratio);
        ~Renderer() = default;

        /// Clears the depth buffer, updates `rctx_`'s time/delta_time, then calls `scene->DrawBackground()` and `scene->Render()`. Logs and returns early (via `debug_error`) if `scene` is null.
        void RenderScene(std::shared_ptr<Scene> scene, GLfloat delta_time);

        /// @return The shared RenderContext this Renderer owns.
        std::shared_ptr<RenderContext> GetRenderContext();
    };

} // namespace Core
