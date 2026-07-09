#pragma once
#include "Forge/Layer.hpp"
#include "Forge/Model.hpp"
#include "Forge/Mesh.hpp"
#include "Forge/Shader.hpp"
#include "Forge/InputEvents.hpp"
#include "Forge/FrameContext.hpp"
#include "Forge/ResourceManager.hpp"

#include "Forge/Colors.hpp"
#include "App/InputConfig.hpp"

#include <memory>

namespace Test
{
    /**
     * @brief Demo `Forge::Layer` used to exercise the rendering/lighting pipeline against a real scene.
     *
     * Loads a handful of cube models and a textured crate, assigns each a different
     * `Forge::Material` preset (Gold/Silver/Ruby/Emerald, plus a textured crate material), and
     * populates `materialModels_`/`materials_` so `OnRender` has something to draw against the
     * multi-light shader loop. Tab (`Forge::Key::Tab`) randomizes one material's flat color via
     * `Forge::Material::SetColor`.
     */
    class TestLayer : public Forge::Layer
    {
    public:
        /// Builds the demo scene's models/materials via `resourceManager`, seeding `materialModels_`/`materials_`.
        TestLayer(std::shared_ptr<Forge::ResourceManager> resourceManager);
        ~TestLayer() = default;

        /// Handles Tab (randomize a material's color); consumes every event (always returns `true`, i.e. never blocks propagation to layers below).
        bool OnEvent(Forge::Event &e) override;
        /// No-op — this demo layer has no per-frame logic of its own.
        void OnUpdate() override;
        // void Transition() override;
        // void Suspend() const override;
        /// Binds each material once, then sets `uModel`/`uNormalMatrix` and draws every model in its bucket.
        void OnRender(std::shared_ptr<Forge::FrameContext> ctx) const override;
        /// No-op — this demo layer owns no resources beyond what `shared_ptr` already manages.
        void Destroy() override;
    };

} // namespace Test
