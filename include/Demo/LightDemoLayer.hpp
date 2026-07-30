#pragma once
#include "Forge/Scene/Layer.hpp"
#include "Forge/Rendering/Model.hpp"
#include "Forge/Rendering/Mesh.hpp"
#include "Forge/Rendering/Shader.hpp"
#include "Forge/Core/InputEvents.hpp"
#include "Forge/Scene/FrameContext.hpp"
#include "Forge/Core/ResourceManager.hpp"

#include "Forge/Rendering/Colors.hpp"
#include "Demo/InputConfig.hpp"

#include <memory>

namespace Demo
{
    /**
     * @brief Demo `Forge::Layer` used to exercise the rendering/lighting pipeline against a real scene.
     *
     * Builds a floor (hardcoded quad, no OBJ) plus six "stations" laid out along it — one
     * cube per Blinn-Phong preset (Gold/Silver/Ruby/Emerald), each paired with a dedicated
     * light in `LightDemoScene::OnSceneBoot` so that light's effect dominates its own station,
     * a fifth station for the textured crate (diffuse+specular mapping, as opposed to
     * the flat presets), and a sixth for a sub-meshed signpost (`models/signpost.obj`, two
     * `usemtl` groups loaded via `Mesh::CreateGroups` into a `Forge::Prop` so its post and
     * board move as one object). Populates `materialModels_`/`materials_` so `OnRender` has
     * something to draw against the multi-light shader loop. `Q` (not `Tab` — `Tab` is
     * reserved by `Engine` for scene switching) randomizes the Gold material's flat color via
     * `Forge::Material::SetColor`.
     */
    class LightDemoLayer : public Forge::Layer
    {
    public:
        /// Builds the demo scene's models/materials via `resourceManager`, seeding `materialModels_`/`materials_`.
        LightDemoLayer(std::string name, std::shared_ptr<Forge::ResourceManager> resourceManager);
        ~LightDemoLayer() = default;

        /// Handles Q (randomize the Gold material's color); consumes every event (always returns `true`, i.e. never blocks propagation to layers below).
        bool OnEvent(Forge::Event &e, std::shared_ptr<Forge::FrameContext> ctx) override;
        /// No-op — this demo layer has no per-frame logic of its own.
        void OnUpdate() override;

        /// Binds each material once, then sets `uModel`/`uNormalMatrix` and draws every model in its bucket.
        void OnRender(std::shared_ptr<Forge::FrameContext> ctx) const override;
        /// No-op — this demo layer owns no resources beyond what `shared_ptr` already manages.
        void Destroy() override;
    };
} // namespace Demo
