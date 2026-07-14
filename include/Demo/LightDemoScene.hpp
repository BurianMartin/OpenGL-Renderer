#pragma once
#include "Utils.hpp"
#include "Forge/Scene.hpp"
#include "Forge/Layer.hpp"
#include "Forge/Camera.hpp"
#include "Forge/InputEvents.hpp"
#include "Forge/ResourceManager.hpp"

#include "Demo/LightDemoLayer.hpp"
#include "Demo/InputConfig.hpp"

#include <ranges>
#include <vector>
#include <unordered_map>

/**
 * @namespace Demo
 * @brief Engine-layer demo code built on top of the `Forge` engine (`LightDemoScene`,
 * `LightDemoLayer`, `MultiCameraDemoScene`).
 *
 * Depends on `Forge` but not vice versa — see CLAUDE.md's "Forge/Demo
 * separation" notes.
 */
namespace Demo
{

    /**
     * @brief Demo `Forge::Scene` — a single free-fly camera over a `LightDemoLayer` floor,
     *        with a `Skybox` background and one dedicated light per "station" so all three
     *        `Forge::Light` subclasses (three point lights, one directional, one spot) each
     *        have a clearly dominant effect on their own object, instead of blending
     *        uniformly over a scattered pile.
     */
    class LightDemoScene : public Forge::Scene
    {
    public:
        LightDemoScene() = default;
        ~LightDemoScene() = default;

        /// Resets the active camera's mouse tracking so re-capturing the cursor doesn't cause a look-direction jump.
        virtual void OnMouseCapture() override;

        /// Advances the active camera by `delta_time` (integrates any currently-held movement).
        virtual void OnUpdate(float delta_time) override;

        /// Translates raw input events into camera control: Left Ctrl toggles a movement speed boost, `key_map` drives WASD/Space/Shift movement, mouse motion/scroll drive look/zoom, and window resize updates every camera's aspect ratio. Anything unhandled falls through to the layer stack.
        virtual void OnEvent(Forge::Event &event) override;

        /// Creates the scene's camera, adds `LightDemoLayer`, sets the Skybox background, and adds one light per station (see `OnSceneBoot`'s own comments for which light pairs with which object).
        virtual void OnSceneBoot() override;

        virtual void OnResume(std::shared_ptr<Forge::FrameContext> fctx);

        // virtual void Suspend() override;
    };
}
