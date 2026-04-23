#pragma once
#include "Utils.hpp"
#include "Layer.hpp"

#include <vector>

namespace Core
{
    class Scene
    {
    private:
        std::vector<Layer> layers_;

    public:
        Scene();
        ~Scene();

        void AddLayer(const Layer &layer);

        const std::vector<Layer> &GetLayers() const;
    };
}