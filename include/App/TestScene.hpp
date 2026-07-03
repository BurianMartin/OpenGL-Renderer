#pragma once
#include "Utils.hpp"
#include "Core/Scene.hpp"
#include "Core/Layer.hpp"
#include "Core/Camera.hpp"
#include "Core/InputEvents.hpp"
#include "Core/ResourceManager.hpp"

#include "App/TestLayer.hpp"
#include "App/InputConfig.hpp"

#include <ranges>
#include <vector>
#include <unordered_map>

/**
 * @namespace Test
 * @brief Application-layer demo code built on top of the `Core` engine (`TestScene`, `TestLayer`).
 *
 * Depends on `Core` but not vice versa — see CLAUDE.md's "Core/App
 * separation" notes. Not yet fully documented; this pass focused on `Core`.
 */
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