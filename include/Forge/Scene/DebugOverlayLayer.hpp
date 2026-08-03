#pragma once
#include "Forge/Scene/Layer.hpp"
#include "Forge/Core/ResourceManager.hpp"
#include "Forge/Rendering/Font.hpp"
#include "Forge/Rendering/Text.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Forge
{
    /**
     * @brief Debug-draw layer: renders a live-updating wireframe box around every Model
     * belonging to whichever Layer(s) it's told to watch, plus a Minecraft-F3-style text
     * readout (frame time/FPS) over the top.
     *
     * Model::GetWorldBounds() (the AABB click-to-pick already relies on) is exactly the
     * kind of thing worth being able to actually see rather than trust blindly, and once
     * Forge::Text existed there was no reason for this to stay wireframes-only — a single
     * F3-toggled overlay covering both is more Minecraft-F3-shaped than two separate
     * layers would be. Toggle with the inherited Show()/Hide()/SetShow() (e.g. bind F3 in
     * whichever Scene owns this) — Layer::Render() skips a hidden layer's draw entirely.
     *
     * Registration order matters for the text half: Layer::Render() calls OnRender()
     * *before* that layer's own materialModels_ draw loop, so for this layer's text to sit
     * on top of the *scene's* 3D content (not just its own wireframe boxes), this layer as
     * a whole needs to be added to the Scene *after* whatever it's watching — same
     * requirement the wireframe boxes already had, so no new constraint.
     *
     * Wireframe boxes: built on the same "shared geometry, per-instance transform" pattern
     * as any other Model — one unit wireframe cube Mesh (GL_LINES, corners at +-0.5) built
     * once and reused for every tracked object; OnUpdate() re-fits each debug box's
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
        /// Re-fits every tracked wireframe box's position/scale to its source Model's current
        /// Model::GetWorldBounds(), and refreshes the live frame-time/FPS text from
        /// ctx->delta_time_ — the concrete example of "Layer access to per-frame context in
        /// OnUpdate": this couldn't be done from OnUpdate() at all before that gap closed,
        /// and had to live in OnRender() instead (which does get a FrameContext) purely to
        /// reach delta_time_, mixing "update state" and "draw" responsibilities that don't
        /// otherwise belong together.
        void OnUpdate(std::shared_ptr<Forge::FrameContext> ctx) override;
        /// Draws both text lines — see class doc for why this needs to run before this layer's own model draw loop, not after.
        void OnRender(std::shared_ptr<Forge::FrameContext> ctx) const override;

    private:
        /// (source model being watched, this layer's debug box Model standing in for it)
        std::vector<std::pair<std::shared_ptr<Model>, std::shared_ptr<Model>>> tracked_;

        std::shared_ptr<Font> font_;
        std::shared_ptr<Text> infoText_;  // static: what F3/this overlay is, how many boxes are tracked
        std::shared_ptr<Text> statsText_; // live: frame time / FPS, refreshed every OnUpdate()
    };
} // namespace Forge
