#pragma once
#include "Utils.hpp"
#include "Forge/Model.hpp"
#include "Forge/Material.hpp"
#include "Forge/InputEvents.hpp"
#include "Forge/FrameContext.hpp"
#include "Forge/Tweens.hpp"

#include <memory>
#include <unordered_map>
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
            for (auto &tween : tweens_)
            {
                tween->Update(delta_time);
            }
            std::erase_if(tweens_, [](const std::shared_ptr<Vec3Tween> &tween)
                          { return tween->IsDone(); });
        }

    protected:
        std::vector<std::shared_ptr<Material>> materials_;
        std::unordered_map<std::shared_ptr<Material>, std::vector<std::shared_ptr<Model>>> materialModels_;
        std::vector<std::shared_ptr<Vec3Tween>> tweens_;

        void AddTween(std::unique_ptr<Vec3Tween> tween)
        {
            tweens_.push_back(std::move(tween));
        }

    public:
        Layer() = default;
        ~Layer() = default;

        // ------------ Base methods start ----------

        /// @return `false` to consume the event and stop propagation to layers below; `true` to pass it on.
        virtual bool OnEvent(Forge::Event &e) = 0;

        /// Per-frame logic, called once per frame before rendering.
        virtual void OnUpdate() = 0;
        // virtual void Transition() = 0;

        /// Is called before rendering the layer's objects
        virtual void OnRender(std::shared_ptr<FrameContext> ctx) const = 0;

        // ------------ Base methods end ------------

        /// Release any layer-owned resources ahead of destruction.
        virtual void Destroy() {};

        /// Calls OnUpdate(), then advances/prunes this layer's tweens. Called once per frame by Scene::Update.
        void Update(GLfloat delta_time)
        {
            OnUpdate();
            UpdateTweens(delta_time);
        }

        /// Submit this layer's draw calls. Typically binds each Material once, then sets `uModel` and draws every Model in its bucket.
        void Render(std::shared_ptr<FrameContext> ctx)
        {
            OnRender(ctx);
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
    };

} // namespace Forge
