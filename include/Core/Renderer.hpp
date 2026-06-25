#pragma once
#include "Utils.hpp"
#include "Core/Scene.hpp"
#include "Core/RenderContext.hpp"

#include <memory>

namespace Core
{
    class Renderer
    {
    private:
        std::shared_ptr<RenderContext> rctx_;

    public:
        Renderer(GLfloat aspect_ratio);
        ~Renderer() = default;

        void RenderScene(std::shared_ptr<Scene> scene, GLfloat delta_time);

        std::shared_ptr<RenderContext> GetRenderContext();
    };

} // namespace Core
