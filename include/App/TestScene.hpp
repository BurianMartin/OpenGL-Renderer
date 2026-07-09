#pragma once
#include "Utils.hpp"
#include "Forge/Scene.hpp"
#include "Forge/Layer.hpp"
#include "Forge/Camera.hpp"
#include "Forge/InputEvents.hpp"
#include "Forge/ResourceManager.hpp"

#include "App/TestLayer.hpp"
#include "App/InputConfig.hpp"

#include <ranges>
#include <vector>
#include <unordered_map>

/**
 * @namespace Test
 * @brief Engine-layer demo code built on top of the `Forge` engine (`TestScene`, `TestLayer`).
 *
 * Depends on `Forge` but not vice versa — see CLAUDE.md's "Forge/App
 * separation" notes.
 */
namespace Test
{

    /**
     * @brief Demo `Forge::Scene` — a single free-fly camera, one `TestLayer`, and a small
     *        multi-light setup (three point lights plus one directional light).
     */
    class TestScene : public Forge::Scene
    {
    public:
        TestScene() = default;
        ~TestScene() = default;

        /// Resets the active camera's mouse tracking so re-capturing the cursor doesn't cause a look-direction jump.
        virtual void OnMouseCapture() override;

        /// Advances the active camera by `delta_time` (integrates any currently-held movement).
        virtual void OnUpdate(float delta_time) override;

        /// Translates raw input events into camera control: Left Ctrl toggles a movement speed boost, `key_map` drives WASD/Space/Shift movement, mouse motion/scroll drive look/zoom, and window resize updates the camera's aspect ratio. Anything unhandled falls through to the layer stack.
        virtual void OnEvent(Forge::Event &event) override;

        /// Creates the scene's camera, adds `TestLayer`, sets a black background, and adds the demo's point/directional lights.
        virtual void OnSceneBoot() override;
    };
}