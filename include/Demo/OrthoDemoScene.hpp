#pragma once
#include "Utils.hpp"
#include "Forge/Scene.hpp"
#include "Forge/Layer.hpp"
#include "Forge/Camera.hpp"
#include "Forge/InputEvents.hpp"
#include "Forge/ResourceManager.hpp"

#include "Demo/LightDemoLayer.hpp"

#include <ranges>

namespace Demo
{
    /**
     * @brief Demo `Forge::Scene` exercising the orthographic camera.
     *
     * A single fixed camera (`Camera::SetPosition`/`SetYawPitch`, never touched again — same
     * pattern as `MultiCameraDemoScene`'s picture-in-picture camera) positioned high above the
     * `LightDemoLayer` station layout, looking straight down. `Camera::SetOrthographic` switches
     * its projection from perspective to orthographic, and `Camera::SetUp` supplies a horizontal
     * reference vector in place of the default world-up `(0,1,0)` — which degenerates for a
     * straight-down camera (front nearly parallel to up, breaking `glm::lookAt`'s internal cross
     * product). No WASD/mouse-look wired here: unlike the free-fly cameras in the other two demo
     * scenes, this one is purely observational, so parallel edges staying parallel (and the Ruby/
     * Emerald cubes' tweened motion showing no perspective foreshortening) can be compared
     * directly against the perspective views in `LightDemoScene`/`MultiCameraDemoScene`.
     */
    class OrthoDemoScene : public Forge::Scene
    {
    public:
        OrthoDemoScene() = default;
        ~OrthoDemoScene() = default;

        /// No-op — this scene's camera never receives mouse input.
        virtual void OnMouseCapture() override;

        /// No-op — the camera is fixed; nothing to integrate per frame.
        virtual void OnUpdate(float delta_time) override;

        /// Handles window resize (updates the camera's viewport/aspect ratio); everything else falls through to the layer stack.
        virtual void OnEvent(Forge::Event &event) override;

        /// Creates the fixed top-down orthographic camera, adds a `LightDemoLayer`, a solid background, and one light per station (same layout as the other demo scenes).
        virtual void OnSceneBoot() override;

        virtual void OnResume(std::shared_ptr<Forge::FrameContext> fctx) override {}
    };
}
