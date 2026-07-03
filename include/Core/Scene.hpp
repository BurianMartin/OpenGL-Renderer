#pragma once
#include "Utils.hpp"
#include "Core/Layer.hpp"
#include "Core/Camera.hpp"
#include "Core/InputEvents.hpp"
#include "Core/ResourceManager.hpp"

#include <ranges>
#include <vector>

namespace Core
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
     * everything else here is `final` — Application/Renderer call `Update`/
     * `Render` without knowing the concrete Scene type. `OnLoad` is called
     * once by `Application::AddScene` to inject the shared ResourceManager/
     * RenderContext and then run `OnSceneBoot()`.
     */
    class Scene
    {
    private:
        void DrawSolidBackground();
        void DrawSkyboxBackground();
        void DrawSkydomeBackground();

        /// Refreshes `rctx_`'s camera-derived fields from the active Camera. @warning Asserts `!cameras_.empty()`, which compiles out in release builds — calling Render() before any camera exists is undefined behavior there.
        void UpdateRenderContext();

    protected:
        glm::vec4 backgroundColor_ = glm::vec4(0);
        Background_Type backgroundType_ = Background_Type::None;
        std::vector<std::shared_ptr<Layer>> layers_;
        std::vector<Camera> cameras_;
        GLint active_camera_ = 0; // index in the cameras vector

        std::shared_ptr<RenderContext> rctx_;
        std::shared_ptr<ResourceManager> rmanager_;

    public:
        Scene() = default;
        virtual ~Scene() = default;

        /// Handle input and forward to layers as appropriate; called by Application for every raised Event.
        virtual void OnEvent(Event &event) = 0;

        /// Called when cursor capture is toggled on — reset mouse tracking (e.g. Camera::ResetMouseTracking) here to avoid a look-direction jump.
        virtual void OnMouseCapture() = 0;

        /// Per-frame scene logic (e.g. camera update); called before every layer's own `OnUpdate`.
        virtual void OnUpdate(float delta_time) = 0;

        /// Called once during OnLoad, after the ResourceManager/RenderContext are available — create cameras, add layers, add lights, set the background here.
        virtual void OnSceneBoot() = 0;

        /// Stores the shared ResourceManager/RenderContext, then calls OnSceneBoot(). Called once by Application::AddScene.
        void OnLoad(std::shared_ptr<ResourceManager> rmanager, std::shared_ptr<RenderContext> rctx);

        /// Calls OnUpdate(delta_time), then every layer's OnUpdate(). Called once per frame.
        virtual void Update(float delta_time) final;

        /// Appends a layer to the render/event-dispatch list; order matters (see Layer).
        virtual void AddLayer(std::shared_ptr<Layer> layer) final;

        /// Calls Destroy() on every layer, ahead of scene teardown.
        virtual void Destroy() final;

        /// Dispatches to the active Background_Type's draw routine.
        virtual void DrawBackground() final;

        /// Sets a solid background color and switches `backgroundType_` to `Solid`.
        virtual void SetBackgroundColor(glm::vec4 color) final;

        /// Refreshes the render context from the active camera, then calls every layer's OnRender() in order.
        virtual void Render() final;
    };
} // namespace Core