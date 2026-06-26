#pragma once
#include "Utils.hpp"
#include "Core/Scene.hpp"
#include "Core/Layer.hpp"
#include "Core/Camera.hpp"
#include "Core/InputEvents.hpp"
#include "Core/ResourceManager.hpp"

#include "App/TestLayer.hpp"

#include <ranges>
#include <vector>
#include <unordered_map>

namespace Test
{
    class TestScene : public Core::Scene
    {
    public:
        TestScene() = default;
        ~TestScene() = default;

        virtual void OnMouseCapture() override;

        virtual void OnUpdate(float delta_time) override;

        virtual void OnEvent(Core::Event &event) override;

        virtual void OnSceneBoot() override;
    };
}