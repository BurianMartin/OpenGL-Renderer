#pragma once
#include "Utils.hpp"
#include "Forge/Rendering/Model.hpp"
#include "Forge/Rendering/Material.hpp"
#include "Forge/Core/InputEvents.hpp"
#include "Forge/Scene/FrameContext.hpp"
#include "Forge/Rendering/RayCast.hpp"
#include "Forge/Rendering/RayCast.hpp"
#include "Forge/Rendering/Tweens.hpp"
#include "Forge/Core/Keys.hpp"

#include <limits>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Forge
{
    /**
     * @brief Extension point for application/game logic within a Scene.
     *
     * A Scene owns an ordered list of Layers; Scene::Render calls
     * `OnRender` on each in order (layer ordering matters for draw order),
     * and Scene::OnEvent dispatches to layers in *reverse* order, stopping
     * at the first one that returns `false` from `OnEvent` (consumed).
     *
     * Subclasses drive rendering through the two members below: `materials_`
     * holds every Material this layer uses (for tag-based lookup via
     * `std::ranges::find_if` on `GetTag()`); `materialModels_` maps each
     * Material to the Models drawn with it and is the actual authority
     * `Render()` iterates over — binding each Material once, then setting
     * `uModel`/`uNormalMatrix` and drawing every Model in its bucket. Every
     * shader bound by a Material is expected to declare uniforms under
     * these exact names. `OnRender` is a hook for anything layer-specific
     * that needs to happen before that draw loop runs; it can be a no-op.
     * Keep `materials_`/`materialModels_` in sync when adding a Material —
     * the map key is a `shared_ptr`, so the same Material instance must be
     * reused consistently rather than reconstructed.
     */
    class Layer
    {
    private:
        void UpdateTweens(GLfloat delta_time)
        {
            // Snapshot the count rather than range-for'ing over tweens_ directly: a finish
            // callback (Vec3Tween::SetFinishFunction) can call AddTween(), which push_backs
            // into this same vector mid-loop and can reallocate it, invalidating a
            // range-based for's begin/end iterators out from under it. Indexing by a
            // pre-loop count instead only ever touches slots that existed before this call
            // started — safe under reallocation — and simply defers any tween added by a
            // finish callback to next frame's Update() instead of ticking it same-frame.
            size_t count = tweens_.size();
            for (size_t i = 0; i < count; ++i)
            {
                tweens_[i]->Update(delta_time);
            }
            std::erase_if(tweens_, [](const std::shared_ptr<Vec3Tween> &tween)
                          { return tween->IsDone(); });
        }

    protected:
        std::vector<std::shared_ptr<Material>> materials_;
        std::unordered_map<std::shared_ptr<Material>, std::vector<std::shared_ptr<Model>>> materialModels_;
        std::vector<std::shared_ptr<Vec3Tween>> tweens_;
        std::string name_;
        bool show_ = true;

        void AddTween(std::unique_ptr<Vec3Tween> tween)
        {
            tweens_.push_back(std::move(tween));
        }

        /**
         * @brief Casts `ray` against every Model in `materialModels_` and returns the
         * Material+Model of the closest hit (smallest ray parameter `t`), or
         * `std::nullopt` if nothing was hit.
         *
         * Generic across any Layer subclass — walks the same `materialModels_` map
         * `Render()` already iterates, so nothing extra needs registering just to be
         * clickable. Only tests each Model's Forge::Model::GetWorldBounds() (an AABB),
         * not exact mesh geometry — fine for click-to-pick, where "close enough" is
         * the norm (see e.g. Unity/Unreal's own default picking behavior).
         */
        std::optional<std::pair<std::shared_ptr<Material>, std::shared_ptr<Model>>> GetClickedObj(const Ray &ray) const
        {
            float bestT = std::numeric_limits<float>::max();
            std::shared_ptr<Material> bestMaterial;
            std::shared_ptr<Model> bestModel;

            for (const auto &[material, models] : materialModels_)
            {
                for (const auto &model : models)
                {
                    float t;
                    if (IntersectRayAABB(ray, model->GetWorldBounds(), t) && t < bestT)
                    {
                        bestT = t;
                        bestMaterial = material;
                        bestModel = model;
                    }
                }
            }

            if (!bestModel)
                return std::nullopt;
            return std::make_pair(bestMaterial, bestModel);
        }

        std::optional<std::pair<std::shared_ptr<Material>, std::shared_ptr<Model>>>
        GetClickedOnObj(MouseButtonPressedEvent &ev, std::shared_ptr<FrameContext> ctx) const
        {
            if (ev.GetMouseButton() != Forge::MouseButton::Left)
                return std::nullopt;

            // While the cursor is captured (FPS-style mouse-look), GLFW's reported cursor
            // position is a virtual, unbounded value used only for look deltas — it doesn't
            // mean "pixel (x, y) on screen" anymore, and there's no visible cursor to click
            // with anyway. Minecraft-style: the crosshair is always dead-center, so aim from
            // the window's center pixel instead of the (meaningless, here) event coordinates.
            // Unprojecting NDC (0,0) lands exactly on the camera's own forward direction for
            // any symmetric frustum (true for every camera in this engine), so this is the
            // same ray a dedicated "camera position + forward vector" path would produce.
            bool captured = ctx->cursor_mode_ == Forge::CursorMode::Captured;
            GLfloat screenX = captured ? ctx->window_width_ / 2.0f : static_cast<GLfloat>(ev.GetX());
            GLfloat screenY = captured ? ctx->window_height_ / 2.0f : static_cast<GLfloat>(ev.GetY());

            Ray ray;
            if (!Forge::UnprojectScreenPoint(screenX, screenY, 0, 0, ctx->window_width_, ctx->window_height_,
                                             ctx->view_, ctx->projection_, ray))
                return std::nullopt;

            return GetClickedObj(ray);
        }

    public:
        Layer(std::string name) : name_(std::move(name)) {}
        ~Layer() = default;

        // ------------ Base methods start ----------

        /// @return `false` to consume the event and stop propagation to layers below; `true` to pass it on.
        virtual bool OnEvent(Forge::Event &e, std::shared_ptr<Forge::FrameContext> ctx) = 0;

        /// Per-frame logic, called once per frame before rendering. Receives the same
        /// per-frame FrameContext OnEvent/OnRender do — e.g. for view/projection-dependent
        /// updates or ctx->delta_time_-driven state that isn't a Vec3Tween.
        virtual void OnUpdate(std::shared_ptr<Forge::FrameContext> ctx) = 0;
        // virtual void Transition() = 0;

        /// Screen-space/layer-specific drawing, independent of any one camera — called
        /// exactly once per frame by Scene::Render() via RenderScreenSpace() (below),
        /// after every camera's own 3D pass, with a full-window glViewport. This is
        /// where UILayer/DebugOverlayLayer draw their Text — a fixed-ortho screen-space
        /// element would otherwise redraw once per camera and only "work" by scissor-rect
        /// position coincidence. Not the place for camera-dependent drawing; there isn't
        /// currently a per-camera hook, since nothing has needed one yet.
        virtual void OnRender(std::shared_ptr<FrameContext> ctx) const = 0;

        // ------------ Base methods end ------------

        /// Release any layer-owned resources ahead of destruction.
        virtual void Destroy() {};

        /// Calls OnUpdate(ctx), then advances/prunes this layer's tweens. Called once per frame by Scene::Update.
        void Update(GLfloat delta_time, std::shared_ptr<Forge::FrameContext> ctx)
        {
            OnUpdate(ctx);
            UpdateTweens(delta_time);
        }

        /// Submit this layer's camera-dependent 3D draw calls: binds each Material once,
        /// then sets `uModel`/`uNormalMatrix` and draws every Model in its bucket. Called
        /// once per camera by Scene::Render() (each call sees that camera's own view/
        /// projection via `ctx`) — does NOT call OnRender() anymore; see
        /// RenderScreenSpace() for the once-per-frame screen-space pass. No-op while
        /// Hide()/SetShow(false) has this layer hidden.
        void Render(std::shared_ptr<FrameContext> ctx)
        {
            if (!show_)
                return;

            for (const auto &[material, models] : materialModels_)
            {
                material->Bind(ctx);

                for (const auto &model : models)
                {
                    glm::mat4 modelMatrix = model->GetModelMatrix();
                    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(modelMatrix)));

                    material->GetShader()->SetMat4("uModel", modelMatrix);
                    material->GetShader()->SetMat3("uNormalMatrix", normalMatrix);

                    model->Draw();
                }
            }
        }

        /// Submit this layer's screen-space content by calling OnRender(ctx). Called
        /// exactly once per frame by Scene::Render(), after every camera's own Render()
        /// pass above, with a full-window glViewport (no per-camera scissor) — so a
        /// fixed-ortho screen-space draw (Text, UI) only ever issues once. No-op while
        /// Hide()/SetShow(false) has this layer hidden.
        void RenderScreenSpace(std::shared_ptr<FrameContext> ctx) const
        {
            if (!show_)
                return;
            OnRender(ctx);
        }

        /// Every Model registered across materialModels_, flattened — e.g. so a debug
        /// overlay (see DebugOverlayLayer) can visualize this layer's models without
        /// needing access to its internal material buckets.
        std::vector<std::shared_ptr<Model>> GetAllModels() const
        {
            std::vector<std::shared_ptr<Model>> all;
            for (const auto &[material, models] : materialModels_)
                all.insert(all.end(), models.begin(), models.end());
            return all;
        }

        std::string GetName() const { return name_; }

        bool IsShown() const { return show_; }

        void Hide() { show_ = false; }

        void Show() { show_ = true; }

        void SetShow(bool show) { show_ = show; }
    };

} // namespace Forge
