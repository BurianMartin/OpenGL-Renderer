#pragma once
#include "Utils.hpp"
#include "Shader.hpp"
#include "Model.hpp"

#include <map>

namespace Core
{
    class Layer
    {
    protected:
        std::map<std::shared_ptr<Shader>, std::vector<std::shared_ptr<Model>>> shaderModels_;

    public:
        Layer() = default;
        ~Layer() = default;

        // ------------ Base methods start ----------
        virtual void OnEvent() = 0;
        virtual void OnUpdate() = 0;
        virtual void Transition() = 0;
        virtual void Suspend() const = 0;
        virtual void OnRender() const = 0;

        // ------------ Base methods end ------------

        virtual void Destroy() = 0;
    };

} // namespace Core
