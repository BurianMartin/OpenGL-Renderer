#pragma once
#include "Core/Layer.hpp"
#include "Core/Model.hpp"
#include "Core/Mesh.hpp"
#include "Core/Shader.hpp"
#include "App/Colors.hpp"
#include "Core/InputEvents.hpp"
#include "Core/RenderContext.hpp"

#include <glad/gl.h>

namespace Solitare
{
    class TestLayer : public Core::Layer
    {
    public:
        TestLayer();
        ~TestLayer() = default;

        bool OnEvent(Core::Event &e) override;
        void OnUpdate() override;
        void Transition() override;
        void Suspend() const override;
        void OnRender(Core::RenderContext &ctx) const override;
        void Destroy() override;
    };

} // namespace Solitare
