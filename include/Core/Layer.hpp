#pragma once
#include "Utils.hpp"
#include "Core/Shader.hpp"
#include "Core/Model.hpp"
#include "Core/InputEvents.hpp"

#include <map>

namespace Core
{
    class Layer
    {
    protected:
        std::map<std::shared_ptr<Shader>, std::vector<std::shared_ptr<Model>>> shaderModels_;
        std::vector<std::shared_ptr<Shader>> shaders_;

    public:
        Layer() = default;
        ~Layer() = default;

        // ------------ Base methods start ----------
        virtual bool OnEvent(Core::Event &e) = 0;
        virtual void OnUpdate() = 0;
        virtual void Transition() = 0;
        virtual void Suspend() const = 0;
        virtual void OnRender() const = 0;

        // ------------ Base methods end ------------

        virtual void Destroy() = 0;
    };

} // namespace Core
