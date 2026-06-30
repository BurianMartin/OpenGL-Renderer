#pragma once
#include "Core/Layer.hpp"
#include "Core/Model.hpp"
#include "Core/Mesh.hpp"
#include "Core/Shader.hpp"
#include "Core/InputEvents.hpp"
#include "Core/RenderContext.hpp"
#include "Core/ResourceManager.hpp"

#include "Core/Colors.hpp"
#include "App/InputConfig.hpp"

#include <memory>

#include <glad/gl.h>

namespace Test
{
    class TestLayer : public Core::Layer
    {
    public:
        TestLayer(std::shared_ptr<Core::ResourceManager> resourceManager);
        ~TestLayer() = default;

        bool OnEvent(Core::Event &e) override;
        void OnUpdate() override;
        // void Transition() override;
        // void Suspend() const override;
        void OnRender(std::shared_ptr<Core::RenderContext> ctx) const override;
        void Destroy() override;
    };

} // namespace Test
