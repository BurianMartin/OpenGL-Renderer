#include "Core/Scene.hpp"
#include "Scene.hpp"

namespace Core
{

    void Scene::DrawSolidBackground()
    {
        glClearColor(backgroundColor_.r, backgroundColor_.g, backgroundColor_.b, backgroundColor_.a);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Scene::DrawSkyboxBackground()
    {
        // TODO: Finish after adding textures to models
    }

    void Scene::DrawSkydomeBackground()
    {
        // TODO: Finish after adding textures to models
    }

    Scene::Scene()
    {
    }
    void Scene::AddLayer(std::shared_ptr<Layer> layer)
    {
        layers_.push_back(layer);
    }

    const std::vector<std::shared_ptr<Layer>> &Scene::GetLayers() const
    {
        return layers_;
    }

    void Scene::Destroy()
    {
        for (auto &layer : layers_)
        {
            layer->Destroy();
        }
    }
    void Scene::DrawBackground()
    {
        switch (backgroundType_)
        {
        case Background_Type::Solid:
            DrawSolidBackground();
            break;

        case Background_Type::Skybox:
            DrawSkyboxBackground();
            break;

        case Background_Type::Skydome:
            DrawSkydomeBackground();
            break;

        case Background_Type::None:
            // Do nothing
            break;

        default:
            break;
        }
    }

    void Scene::SetBackgroundColor(glm::vec4 color)
    {
        backgroundType_ = Background_Type::Solid;
        backgroundColor_ = color;
    }

} // namespace Core
