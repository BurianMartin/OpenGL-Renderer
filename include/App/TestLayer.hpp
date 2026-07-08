#pragma once
#include "Core/Layer.hpp"
#include "Core/Model.hpp"
#include "Core/Mesh.hpp"
#include "Core/Shader.hpp"
#include "Core/InputEvents.hpp"
#include "Core/RenderContext.hpp"
#include "Core/ResourceManager.hpp"

#include "Core/Colors.hpp"
#include "App/InputConfig.hpp"

#include <memory>

#include <glad/gl.h>

namespace Test
{
    /**
     * @brief Demo `Core::Layer` used to exercise the rendering/lighting pipeline against a real scene.
     *
     * Loads a handful of cube models and a textured crate, assigns each a different
     * `Core::Material` preset (Gold/Silver/Ruby/Emerald, plus a textured crate material), and
     * populates `materialModels_`/`materials_` so `OnRender` has something to draw against the
     * multi-light shader loop. Tab (`GLFW_KEY_TAB`) randomizes one material's flat color via
     * `Core::Material::SetColor`.
     */
    class TestLayer : public Core::Layer
    {
    public:
        /// Builds the demo scene's models/materials via `resourceManager`, seeding `materialModels_`/`materials_`.
        TestLayer(std::shared_ptr<Core::ResourceManager> resourceManager);
        ~TestLayer() = default;

        /// Handles Tab (randomize a material's color); consumes every event (always returns `true`, i.e. never blocks propagation to layers below).
        bool OnEvent(Core::Event &e) override;
        /// No-op — this demo layer has no per-frame logic of its own.
        void OnUpdate() override;
        // void Transition() override;
        // void Suspend() const override;
        /// Binds each material once, then sets `uModel`/`uNormalMatrix` and draws every model in its bucket.
        void OnRender(std::shared_ptr<Core::RenderContext> ctx) const override;
        /// No-op — this demo layer owns no resources beyond what `shared_ptr` already manages.
        void Destroy() override;
    };

} // namespace Test
