#pragma once
#include "Utils.hpp"
#include "Layer.hpp"

#include <vector>

namespace Core
{
    class Scene
    {
    private:
        std::vector<std::shared_ptr<Layer>> layers_;

    public:
        Scene();
        ~Scene() = default;

        void AddLayer(std::shared_ptr<Layer> layer);

        const std::vector<std::shared_ptr<Layer>> &GetLayers() const;

        void Destroy();
    };
}