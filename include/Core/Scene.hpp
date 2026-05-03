#pragma once
#include "Utils.hpp"
#include "Core/Layer.hpp"
#include "Core/InputEvents.hpp"

#include <vector>
#include <ranges>

namespace Core
{
    enum class Background_Type
    {
        Solid,
        Skybox,
        Skydome,
        None
    };

    class Scene
    {
    private:
        glm::vec4 backgroundColor_ = glm::vec4(0);
        Background_Type backgroundType_ = Background_Type::None;
        std::vector<std::shared_ptr<Layer>> layers_;

        void DrawSolidBackground();
        void DrawSkyboxBackground();
        void DrawSkydomeBackground();

    public:
        Scene();
        ~Scene() = default;

        void AddLayer(std::shared_ptr<Layer> layer);

        const std::vector<std::shared_ptr<Layer>> &GetLayers() const;

        void Destroy();

        void DrawBackground();

        void SetBackgroundColor(glm::vec4 color);

        void HandleEvent(Event &event);
    };
}