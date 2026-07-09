#pragma once
#include "Utils.hpp"
#include "Forge/Scene.hpp"
#include "Forge/FrameContext.hpp"

#include <memory>

namespace Forge
{
    /**
     * @brief Drives one frame of drawing for the active Scene.
     *
     * Owns the single FrameContext shared with every Scene/Layer/Material
     * this frame. Engine calls `RenderScene` once per frame after
     * `Update`.
     */
    class Renderer
    {
    private:
        std::shared_ptr<FrameContext> fctx_;

    public:
        /// @param windowWidth,windowHeight Initial window size, stored in the FrameContext.
        Renderer(GLint windowWidth, GLint windowHeight);
        ~Renderer() = default;

        /// Updates `fctx_`'s time/delta_time, then calls `scene->Render()` — which clears depth and draws the background once per camera, inside that camera's own scissored viewport. Logs and returns early (via `debug_error`) if `scene` is null.
        void RenderScene(std::shared_ptr<Scene> scene, GLfloat delta_time);

        /// @return The shared FrameContext this Renderer owns.
        std::shared_ptr<FrameContext> GetFrameContext();
    };

} // namespace Forge
