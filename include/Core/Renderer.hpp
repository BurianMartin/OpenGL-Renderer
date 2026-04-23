#pragma once
#include "Utils.hpp"
#include "Scene.hpp"

#include <memory>

namespace Core
{
    class Renderer
    {
    private:
        /* data */
    public:
        Renderer() {}
        ~Renderer() {}

        void RenderScene(std::shared_ptr<Scene> scene);
    };

} // namespace Core
