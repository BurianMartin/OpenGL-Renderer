#pragma once
#include "Utils.hpp"
#include "Core/Model.hpp"
#include "Core/Material.hpp"
#include "Core/InputEvents.hpp"
#include "Core/RenderContext.hpp"

#include <memory>
#include <unordered_map>

namespace Core
{
    class Layer
    {
    protected:
        std::vector<std::shared_ptr<Material>> materials_;
        std::unordered_map<std::shared_ptr<Material>, std::vector<std::shared_ptr<Model>>>
            materialModels_;

    public:
        Layer() = default;
        ~Layer() = default;

        // ------------ Base methods start ----------
        virtual bool OnEvent(Core::Event &e) = 0;
        virtual void OnUpdate() = 0;
        // virtual void Transition() = 0;
        // virtual void Suspend() const = 0;
        virtual void OnRender(std::shared_ptr<RenderContext> ctx) const = 0;

        // ------------ Base methods end ------------

        virtual void Destroy() = 0;
    };

} // namespace Core
