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

namespace Demo
{
    /**
     * @brief Demo `Forge::Scene` exercising split-screen rendering.
     *
     * Two cameras share the same `LightDemoLayer` scene content: a free-fly
     * main camera filling the whole window, and a second, fixed camera
     * (positioned once via `Camera::SetPosition`/`SetYawPitch`, never
     * touched again) rendering into a picture-in-picture inset in the
     * top-right 30%x30% of the window (see `Forge::Viewport`) — an elevated
     * overview looking down over the whole station layout, genuinely
     * different from the main camera's ground-level free-fly. The second
     * camera never receives input — it's never made `active_camera_` — but
     * `Scene::Render()` still draws it every frame, in its own scissored
     * viewport, same as the main one. Uses the `Skydome` background
     * (`LightDemoScene` already exercises `Skybox`), which also happens to
     * be a good check that the sky looks correct from two very different
     * camera angles at once.
     */
    class MultiCameraDemoScene : public Forge::Scene
    {
    public:
        MultiCameraDemoScene() = default;
        ~MultiCameraDemoScene() = default;

        /// Resets the active camera's mouse tracking so re-capturing the cursor doesn't cause a look-direction jump.
        virtual void OnMouseCapture() override;

        /// Advances the active camera by `delta_time` (integrates any currently-held movement).
        virtual void OnUpdate(float delta_time) override;

        /// Same input handling as `LightDemoScene` — WASD/Space/Shift movement, mouse-look/zoom, and window resize updates every camera's aspect ratio, not just the active one.
        virtual void OnEvent(Forge::Event &event) override;

        /// Creates and positions the main + picture-in-picture cameras, adds a `LightDemoLayer`, sets the Skydome background, and adds one light per station (same layout as `LightDemoScene`).
        virtual void OnSceneBoot() override;
    };
}
