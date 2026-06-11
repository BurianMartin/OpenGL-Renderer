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
        RenderContext rctx_;

    public:
        Renderer() = default;
        ~Renderer() = default;

        void RenderScene(std::shared_ptr<Scene> scene);
    };

} // namespace Core
