#include "Core/Scene.hpp"
#include "Scene.hpp"

namespace Core
{

    void Scene::DrawSolidBackground()
    {
        glClearColor(backgroundColor_.r, backgroundColor_.g, backgroundColor_.b, backgroundColor_.a);
        glClear(GL_COLOR_BUFFER_BIT); // Has to be called to fill the screen with the color
    }

    void Scene::DrawSkyboxBackground()
    {
        // TODO: Finish after adding textures to models
    }

    void Scene::DrawSkydomeBackground()
    {
        // TODO: Finish after adding textures to models
    }

    Scene::Scene(float aspect_ratio)
    {
        cameras_.emplace_back(aspect_ratio); // Base camera
        active_camera_ = 0;
    }

    void Scene::Update(GLfloat delta_time)
    {
        cameras_[active_camera_].Update(delta_time);
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
            debug_warn("No background set.");
            break;

        default:
            break;
        }
        glClear(GL_DEPTH_BUFFER_BIT); // Backround drawn, clear depth buffer to draw everything over it
    }

    void Scene::SetBackgroundColor(glm::vec4 color)
    {
        backgroundType_ = Background_Type::Solid;
        backgroundColor_ = color;
    }

    void Scene::HandleEvent(Event &event)
    {
        // Optional Scene event consumption example:
        if (event.GetEventType() == Core::EventType::KeyPressed)
        {
            auto ev = static_cast<KeyPressedEvent &>(event);
            auto it = Core::key_map.find(ev.GetKeyCode());
            if (it != Core::key_map.end())
                cameras_[active_camera_].CameraMove(it->second, true);
        }
        else if (event.GetEventType() == Core::EventType::KeyReleased)
        {
            auto ev = static_cast<KeyReleasedEvent &>(event);
            auto it = Core::key_map.find(ev.GetKeyCode());
            if (it != Core::key_map.end())
                cameras_[active_camera_].CameraMove(it->second, false);
        }
        else if (event.GetEventType() == Core::EventType::MouseMoved)
        {
            auto ev = static_cast<MouseMovedEvent &>(event);
            if (mouse_captured_) // only rotate when right mouse held
                cameras_[active_camera_].ProcessMousePosition(ev.GetX(), ev.GetY());
                }
        else if (event.GetEventType() == Core::EventType::MouseButtonPressed)
        {
            mouse_captured_ = true;
        }
        else if (event.GetEventType() == Core::EventType::MouseButtonReleased)
        {
            mouse_captured_ = false;
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

    void Scene::FillRenderContext(Core::RenderContext &ctx)
    {
        ctx.camera_position_ = cameras_[active_camera_].GetPosition();
        ctx.view_ = cameras_[active_camera_].GetViewMatrix();
        ctx.projection_ = cameras_[active_camera_].GetProjectionMatrix();
    }

} // namespace Core
