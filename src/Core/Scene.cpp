#include "Core/Scene.hpp"

Core::Scene::Scene()
{
}
void Core::Scene::AddLayer(std::shared_ptr<Core::Layer> layer)
{
    layers_.push_back(layer);
}

const std::vector<std::shared_ptr<Core::Layer>> &Core::Scene::GetLayers() const
{
    return layers_;
}

void Core::Scene::Destroy()
{
    for (auto &layer : layers_)
    {
        layer->Destroy();
    }
}