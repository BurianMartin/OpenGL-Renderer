#include "Core/Scene.hpp"

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

    void Scene::HandleEvent(Event &event)
    {

        // TODO: Possibly consume the event in scene and then no propagation trough layers
        // For now, leave the Scene capture
        if (false && event.GetEventType() == Core::EventType::KeyPressed)
        {
            auto ev = static_cast<KeyPressedEvent &>(event);
            switch (ev.GetKeyCode())
            {
            case GLFW_KEY_TAB:
                return;
                break;

            default:
                break;
            }
        }

        for (std::shared_ptr<Core::Layer> &layer : std::views::reverse(layers_))
        {
            if (!layer->OnEvent(event))
            {
                // Event was consumed by this layer
                break;
            }
        }
    }

} // namespace Core
