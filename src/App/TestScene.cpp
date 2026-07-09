#include "App/TestScene.hpp"
#include "Forge/Camera.hpp"
#include "Forge/Lights.hpp"

namespace Test
{
    void TestScene::OnMouseCapture()
    {
        cameras_[active_camera_].ResetMouseTracking();
    }

    void TestScene::OnUpdate(float delta_time)
    {
        cameras_[active_camera_].Update(delta_time);
    }

    void TestScene::OnEvent(Forge::Event &event)
    {
        // Optional Scene event consumption example:

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
            auto ev = static_cast<Forge::WindowResizeEvent &>(event);
            rctx_->aspect_ratio_ = static_cast<float>(ev.GetWidth()) / static_cast<float>(ev.GetHeight());
            cameras_[active_camera_].UpdateAspectRatio(rctx_->aspect_ratio_);
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

    void TestScene::OnSceneBoot()
    {
        cameras_.emplace_back(rctx_->aspect_ratio_);

        auto testLayer = std::make_shared<Test::TestLayer>(rmanager_);

        AddLayer(testLayer);

        SetBackgroundColor(Forge::Color_A1::Black);

        // Warm key light, plus two colorful rim lights either side of the scene.
        AddLight(Forge::PointLight::Create(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(1.0f, 0.95f, 0.85f), 1.0f));
        AddLight(Forge::PointLight::Create(glm::vec3(-3.0f, 1.0f, -1.0f), glm::vec3(1.0f, 0.95f, 0.85f), 0.8f));
        AddLight(Forge::PointLight::Create(glm::vec3(3.0f, 1.0f, -3.0f), glm::vec3(1.0f, 0.95f, 0.85f), 0.8f));
        AddLight(Forge::DirectionalLight::Create(glm::vec3(0.60f, -1.0f, 0.30f), glm::vec3(1.0f, 1.0f, 1.0f), 0.4f));
    }

} // namespace Test
