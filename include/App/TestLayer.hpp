#pragma once
#include "Core/Layer.hpp"
#include "Core/Model.hpp"
#include "Core/Mesh.hpp"
#include "Core/Shader.hpp"
#include "App/Colors.hpp"
#include "Core/InputEvents.hpp"

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
        void OnRender() const override;
        void Destroy() override;
    };

} // namespace Solitare
