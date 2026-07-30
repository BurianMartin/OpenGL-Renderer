#pragma once
#include "Forge/Layer.hpp"
#include "Forge/ResourceManager.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Forge
{
    /**
     * @brief Debug-draw layer: renders a live-updating wireframe box around every Model
     * belonging to whichever Layer(s) it's told to watch.
     *
     * Exists because there's no text/UI rendering yet (SCOPE.md's Tier 2) — a numeric HUD
     * isn't possible yet, but a *visual* debug aid is, and Model::GetWorldBounds() (the
     * AABB click-to-pick already relies on) is exactly the kind of thing worth being able
     * to actually see rather than trust blindly. Toggle with the inherited
     * Show()/Hide()/SetShow() (e.g. bind F3 to it in whichever Scene owns this) —
     * Layer::Render() skips a hidden layer's draw entirely.
     *
     * Built on the same "shared geometry, per-instance transform" pattern as any other
     * Model: one unit wireframe cube Mesh (GL_LINES, corners at +-0.5) is built once and
     * reused for every tracked object; OnUpdate() re-fits each debug box's
     * position/scale to its tracked Model's current world-space AABB every frame, so it
     * follows moving/animated objects (e.g. this demo's spinning/bobbing cubes) without
     * ever rebuilding geometry.
     */
    class DebugOverlayLayer : public Layer
    {
    public:
        /// @param watchedLayers Every Model currently in each layer's materialModels_ (via
        /// Layer::GetAllModels()) gets its own tracked wireframe box. This is a one-time
        /// snapshot taken in the constructor, not a live subscription — a Model added to a
        /// watched Layer afterward won't get a box.
        DebugOverlayLayer(std::string name, std::shared_ptr<ResourceManager> resourceManager,
                           std::vector<std::shared_ptr<Layer>> watchedLayers);
        ~DebugOverlayLayer() = default;

        /// Pass-through — a debug overlay doesn't consume input (always returns true).
        bool OnEvent(Forge::Event &e, std::shared_ptr<Forge::FrameContext> ctx) override;
        /// Re-fits every tracked wireframe box's position/scale to its source Model's current Model::GetWorldBounds().
        void OnUpdate() override;
        /// No-op — draw dispatch is fully handled by the base Layer::Render().
        void OnRender(std::shared_ptr<Forge::FrameContext> ctx) const override;

    private:
        /// (source model being watched, this layer's debug box Model standing in for it)
        std::vector<std::pair<std::shared_ptr<Model>, std::shared_ptr<Model>>> tracked_;
    };
} // namespace Forge
