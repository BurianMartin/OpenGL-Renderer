#pragma once
#include "Utils.hpp"
#include "Forge/Layer.hpp"
#include "Forge/Lights.hpp"
#include "Forge/Camera.hpp"
#include "Forge/InputEvents.hpp"
#include "Forge/ResourceManager.hpp"

#include <ranges>
#include <vector>

namespace Forge
{
    /// Which DrawBackground() strategy is active; set indirectly via SetBackgroundColor() (Skybox/Skydome have no public setter yet — see their TODOs).
    enum class Background_Type
    {
        Solid,
        Skybox,
        Skydome,
        None
    };

    /**
     * @brief Owns a scene's Layers and Cameras and drives one frame of update/render.
     *
     * Subclasses implement the pure-virtual hooks (`OnEvent`, `OnUpdate`,
     * `OnMouseCapture`, `OnSceneBoot`) to define actual scene behavior;
     * everything else here is `final` — Engine/Renderer call `Update`/
     * `Render` without knowing the concrete Scene type. `OnLoad` is called
     * once by `Engine::AddScene` to inject the shared ResourceManager/
     * FrameContext and then run `OnSceneBoot()`.
     */
    class Scene
    {
    private:
        void DrawSolidBackground();
        void DrawSkyboxBackground();
        void DrawSkydomeBackground();

        /// Refreshes `rctx_`'s camera-derived fields from the active Camera. Logs and returns early (via `debug_error`) if `cameras_` is empty, rather than asserting — this check survives in release builds.
        void UpdateFrameContext();

        void LoadLights();

    protected:
        glm::vec4 backgroundColor_ = glm::vec4(0);
        Background_Type backgroundType_ = Background_Type::None;
        std::vector<std::shared_ptr<Layer>> layers_;
        std::vector<Camera> cameras_;
        std::vector<std::shared_ptr<Light>> lights_;
        GLint active_camera_ = 0; // index in the cameras vector

        std::shared_ptr<FrameContext> rctx_;
        std::shared_ptr<ResourceManager> rmanager_;

        void AddLight(std::shared_ptr<Light> light);

    public:
        Scene() = default;
        virtual ~Scene() = default;

        /// Handle input and forward to layers as appropriate; called by Engine for every raised Event.
        virtual void OnEvent(Event &event) = 0;

        /// Called when cursor capture is toggled on — reset mouse tracking (e.g. Camera::ResetMouseTracking) here to avoid a look-direction jump.
        virtual void OnMouseCapture() = 0;

        /// Per-frame scene logic (e.g. camera update); called before every layer's own `OnUpdate`.
        virtual void OnUpdate(float delta_time) = 0;

        /// Called once during OnLoad, after the ResourceManager/FrameContext are available — create cameras, add layers, add lights, set the background here.
        virtual void OnSceneBoot() = 0;

        /// Stores the shared ResourceManager/FrameContext, then calls OnSceneBoot(). Called once by Engine::AddScene.
        void OnLoad(std::shared_ptr<ResourceManager> rmanager, std::shared_ptr<FrameContext> rctx);

        /// Dispatches to the active Background_Type's draw routine.
        void DrawBackground();

        /// Refreshes the render context from the active camera, then calls every layer's OnRender() in order.
        void Render();

        /// Appends a layer to the render/event-dispatch list; order matters (see Layer). Only ever called by subclasses from OnSceneBoot(), never externally.
        void AddLayer(std::shared_ptr<Layer> layer);

        /// Sets a solid background color and switches `backgroundType_` to `Solid`. Only ever called by subclasses from OnSceneBoot(), never externally.
        void SetBackgroundColor(glm::vec4 color);

        /// Calls OnUpdate(delta_time), then every layer's OnUpdate(). Called once per frame.
        void Update(float delta_time);

        /// Calls Destroy() on every layer, ahead of scene teardown.
        void Destroy();
    };
} // namespace Forge