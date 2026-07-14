#pragma once
#include "Utils.hpp"
#include "Forge/Layer.hpp"
#include "Forge/Lights.hpp"
#include "Forge/Camera.hpp"
#include "Forge/Mesh.hpp"
#include "Forge/Shader.hpp"
#include "Forge/InputEvents.hpp"
#include "Forge/ResourceManager.hpp"

#include <ranges>
#include <vector>

namespace Forge
{
    /// Which DrawBackground() strategy is active; set via SetBackgroundColor()/SetSkyboxBackground()/SetSkydomeBackground().
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

        void LoadLights();

        /// Refreshes `fctx_`'s camera-derived fields from the active Camera. Logs and returns early (via `debug_error`) if `cameras_` is empty, rather than asserting — this check survives in release builds.
        void UpdateFrameContext(GLint camera_index);

        // Lazily created on first use by DrawSkyboxBackground/DrawSkydomeBackground.
        std::shared_ptr<Shader> skyboxShader_;
        std::shared_ptr<Mesh> skyboxMesh_;
        std::shared_ptr<Shader> skydomeShader_;
        GLuint skydomeVAO_ = 0; // intentionally has no bound attributes/buffers — see DrawSkydomeBackground

    protected:
        glm::vec4 backgroundColor_ = glm::vec4(0);
        Background_Type backgroundType_ = Background_Type::None;
        std::vector<std::shared_ptr<Layer>> layers_;
        std::vector<Camera> cameras_;
        std::vector<std::shared_ptr<Light>> lights_;
        GLint active_camera_ = 0; // index in the cameras vector

        std::shared_ptr<FrameContext> fctx_;
        std::shared_ptr<ResourceManager> rmanager_;

        void AddLight(std::shared_ptr<Light> light);

    public:
        Scene() = default;

        /// Releases `skydomeVAO_` if `DrawSkydomeBackground` ever lazily created one.
        virtual ~Scene();

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

        /// Loops over every camera in `cameras_` — ApplyViewport, UpdateFrameContext, a scissored depth clear, DrawBackground, then every layer's Render() — wrapped in glEnable/glDisable(GL_SCISSOR_TEST) so one camera's clear can't bleed into another's viewport.
        void Render();

        /// Appends a layer to the render/event-dispatch list; order matters (see Layer). Only ever called by subclasses from OnSceneBoot(), never externally.
        void AddLayer(std::shared_ptr<Layer> layer);

        /// Sets a solid background color and switches `backgroundType_` to `Solid`. Only ever called by subclasses from OnSceneBoot(), never externally.
        void SetBackgroundColor(glm::vec4 color);

        /// Switches `backgroundType_` to `Skybox` — a hardcoded, position-only unit cube with a procedural gradient+sun shader, forced to the far plane. No texture/cubemap needed. Only ever called by subclasses from OnSceneBoot(), never externally.
        void SetSkyboxBackground();

        /// Switches `backgroundType_` to `Skydome` — a screen-space full-screen triangle reconstructing the view ray per-pixel; a different technique from Skybox (no mesh/vertex buffer at all), same visual idea. Only ever called by subclasses from OnSceneBoot(), never externally.
        void SetSkydomeBackground();

        /// Calls OnUpdate(delta_time), then every layer's OnUpdate(). Called once per frame.
        void Update(float delta_time);

        /// Calls Destroy() on every layer, ahead of scene teardown.
        void Destroy();

        /// Applies `cameras_[camera_index]`'s Viewport as both the GL viewport and scissor rect (forwards to Camera::ApplyViewport).
        void ApplyViewport(GLint camera_index);

        /// Resets the active camera's mouse tracking (see Camera::ResetMouseTracking) so its next ProcessMousePosition call primes instead of jumping. Call whenever this scene becomes newly active — e.g. on a scene switch — since its camera's last-known cursor position may be stale from whenever it was last active.
        void ResetMouse();

        /// Scene-switch hook: called by Resume() whenever this scene becomes active through Engine::SetScene (e.g. Tab-cycling). Deliberately NOT called for the first scene's initial activation at startup — AddScene marks the first registered scene active immediately, so Run()'s switch branch never fires on frame one. Put initial-activation work in OnSceneBoot(); use this hook for work needed whenever the engine switches (back) to this scene. See CLAUDE.md's Engine bullet for the full lifecycle note.
        virtual void OnResume(std::shared_ptr<Forge::FrameContext> fctx) = 0;
        /// Scene-switch hook: called on the outgoing scene just before the incoming scene's Resume(). Default is a no-op. Same startup caveat as OnResume — never runs at startup, and the scene active at window close gets Destroy() only, never a final Suspend().
        virtual void Suspend() {}

        /// Called by Engine::Run when a pending scene switch lands (never for the first scene at startup — see OnResume): resets the GL viewport to the full window, re-syncs every camera's stored window size (resize events only reach the active scene, so an inactive scene's cameras go stale across resizes), then calls the OnResume hook.
        void Resume()
        {
            glViewport(0, 0, fctx_->window_width_, fctx_->window_height_);
            for (auto &camera : cameras_)
                camera.UpdateViewportSize(fctx_->window_width_, fctx_->window_height_);
            OnResume(fctx_);
        }
    };
} // namespace Forge