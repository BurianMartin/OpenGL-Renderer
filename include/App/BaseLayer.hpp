#pragma once
#include "Core/Layer.hpp"
#include "Model.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"

#include <glad/gl.h>

namespace Solitare
{
    class BaseLayer : public Core::Layer
    {
    public:
        BaseLayer();
        ~BaseLayer() = default;

        void OnEvent() override;
        void OnUpdate() override;
        void Transition() override;
        void Suspend() const override;
        void OnRender() const override;
        void Destroy() override;
    };

} // namespace Solitare
