#pragma once
#include "Utils.hpp"
#include "Layer.hpp"

#include <vector>

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

        /* TODO: Add event system that propagates events trough layers
        ** Must be independent, may be owned by a custom Application class.
        ** Has to be connected to the layer system to propagate well
        ** Maybe scene.HandleEvent(Event e) -> propagates trough layers ? ..
        ** .. So the scene can consume the event or leaves it to layers ?
        */

    public:
        Scene();
        ~Scene() = default;

        void AddLayer(std::shared_ptr<Layer> layer);

        const std::vector<std::shared_ptr<Layer>> &GetLayers() const;

        void Destroy();

        void DrawBackground();

        void SetBackgroundColor(glm::vec4 color);

        void HandleEvent(/*Event event*/); // TODO: Implement this function
    };
}