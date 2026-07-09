#include "Demo/MultiCameraDemoScene.hpp"
#include "Forge/Camera.hpp"
#include "Forge/Lights.hpp"

namespace Demo
{
    void MultiCameraDemoScene::OnMouseCapture()
    {
        cameras_[active_camera_].ResetMouseTracking();
    }

    void MultiCameraDemoScene::OnUpdate(float delta_time)
    {
        cameras_[active_camera_].Update(delta_time);
    }

    void MultiCameraDemoScene::OnEvent(Forge::Event &event)
    {
        switch (event.GetEventType())
        {
        case Forge::EventType::KeyPressed:
        {
            auto ev = static_cast<Forge::KeyPressedEvent &>(event);

            switch (ev.GetKeyCode())
            {
            case Forge::Key::LeftControl:
                cameras_[active_camera_].SetBoost(3.0f);
                return;
            }

            auto it = key_map.find(ev.GetKeyCode());
            if (it != key_map.end())
            {
                cameras_[active_camera_].CameraMove(it->second, true);
                return;
            }
            break;
        }
        case Forge::EventType::KeyReleased:
        {
            auto ev = static_cast<Forge::KeyReleasedEvent &>(event);

            switch (ev.GetKeyCode())
            {
            case Forge::Key::LeftControl:
                cameras_[active_camera_].SetBoost(0.0f);
                return;
            }

            auto it = key_map.find(ev.GetKeyCode());
            if (it != key_map.end())
            {
                cameras_[active_camera_].CameraMove(it->second, false);
                return;
            }
            break;
        }
        case Forge::EventType::MouseMoved:
        {
            auto ev = static_cast<Forge::MouseMovedEvent &>(event);
            cameras_[active_camera_].ProcessMousePosition(ev.GetX(), ev.GetY());
            return;
        }
        case Forge::EventType::MouseScrolled:
        {
            auto ev = static_cast<Forge::MouseScrolledEvent &>(event);
            cameras_[active_camera_].Zoom(ev.GetYOffset());
            return;
        }
        case Forge::EventType::WindowResize:
        {
            for (auto &camera : cameras_)
                camera.UpdateViewportSize(fctx_->window_width_, fctx_->window_height_);
            return;
        }

        default:
            break;
        }

        for (std::shared_ptr<Forge::Layer> &layer : std::views::reverse(layers_))
        {
            if (!layer->OnEvent(event))
            {
                // Event was consumed by this layer
                return;
            }
        }
    }

    void MultiCameraDemoScene::OnSceneBoot()
    {
        // Main camera — full window, free-fly, receives all input (active_camera_ defaults to 0).
        cameras_.emplace_back(Forge::Viewport(0.0f, 0.0f, 1.0f, 1.0f, fctx_->window_width_, fctx_->window_height_));

        // Picture-in-picture camera — top-right 30%x30% inset. Never made active, never
        // receives input; Scene::Render() still draws it every frame in its own scissored
        // viewport, on top of the main camera's since it's added second. Positioned as a
        // fixed elevated overview looking down over the whole station layout below — a
        // genuinely different view from the main camera's ground-level free-fly, not just a
        // second copy of the same one.
        cameras_.emplace_back(Forge::Viewport(0.68f, 0.68f, 0.30f, 0.30f, fctx_->window_width_, fctx_->window_height_));
        cameras_[1].SetPosition(glm::vec3(0.0f, 10.0f, 2.0f));
        cameras_[1].SetYawPitch(-90.0f, -70.0f);

        auto lightDemoLayer = std::make_shared<Demo::LightDemoLayer>(rmanager_);
        AddLayer(lightDemoLayer);

        // Skydome here instead of Skybox (LightDemoScene already exercises that one) — both
        // techniques get used somewhere, and the two very different camera angles are a good
        // check that the sky looks correct from both without any texture/cubemap involved.
        SetSkydomeBackground();

        AddLight(Forge::PointLight::Create(glm::vec3(-2.5f, 2.0f, -1.0f), glm::vec3(1.0f, 0.85f, 0.6f), 2.0f));
        AddLight(Forge::PointLight::Create(glm::vec3(2.5f, 2.0f, -3.5f), glm::vec3(0.75f, 0.85f, 1.0f), 2.0f));
        AddLight(Forge::PointLight::Create(glm::vec3(-2.5f, 2.0f, -6.0f), glm::vec3(1.0f, 0.6f, 0.65f), 2.0f));
        AddLight(Forge::SpotLight::Create(glm::vec3(2.5f, 3.5f, -8.5f), glm::vec3(0.0f, -1.0f, 0.0f),
                                           glm::vec3(0.75f, 1.0f, 0.85f), 2.5f, 12.5f, 20.0f));
        AddLight(Forge::DirectionalLight::Create(glm::vec3(0.60f, -1.0f, 0.30f), glm::vec3(1.0f, 1.0f, 1.0f), 0.5f));
    }

} // namespace Demo
