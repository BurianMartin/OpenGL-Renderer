#pragma once
#include "Forge/Layer.hpp"
#include "Forge/Model.hpp"
#include "Forge/Mesh.hpp"
#include "Forge/Shader.hpp"
#include "Forge/InputEvents.hpp"
#include "Forge/FrameContext.hpp"
#include "Forge/ResourceManager.hpp"

#include "Forge/Colors.hpp"
#include "Demo/InputConfig.hpp"

#include <memory>

namespace Demo
{
    /**
     * @brief Demo `Forge::Layer` used to exercise the rendering/lighting pipeline against a real scene.
     *
     * Builds a floor (hardcoded quad, no OBJ) plus five "stations" laid out along it — one
     * cube per Blinn-Phong preset (Gold/Silver/Ruby/Emerald), each paired with a dedicated
     * light in `LightDemoScene::OnSceneBoot` so that light's effect dominates its own station,
     * plus a fifth station for the textured crate (diffuse+specular mapping, as opposed to
     * the flat presets). Populates `materialModels_`/`materials_` so `OnRender` has something
     * to draw against the multi-light shader loop. `Q` (not `Tab` — `Tab` is reserved by
     * `Engine` for scene switching) randomizes the Gold material's flat color via
     * `Forge::Material::SetColor`.
     */
    class LightDemoLayer : public Forge::Layer
    {
    public:
        /// Builds the demo scene's models/materials via `resourceManager`, seeding `materialModels_`/`materials_`.
        LightDemoLayer(std::shared_ptr<Forge::ResourceManager> resourceManager);
        ~LightDemoLayer() = default;

        /// Handles Q (randomize the Gold material's color); consumes every event (always returns `true`, i.e. never blocks propagation to layers below).
        bool OnEvent(Forge::Event &e) override;
        /// No-op — this demo layer has no per-frame logic of its own.
        void OnUpdate() override;

        /// Binds each material once, then sets `uModel`/`uNormalMatrix` and draws every model in its bucket.
        void OnRender(std::shared_ptr<Forge::FrameContext> ctx) const override;
        /// No-op — this demo layer owns no resources beyond what `shared_ptr` already manages.
        void Destroy() override;
    };

} // namespace Demo
