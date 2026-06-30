#include "App/TestScene.hpp"
#include "Core/Camera.hpp"

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

    void TestScene::OnEvent(Core::Event &event)
    {
        // Optional Scene event consumption example:

        switch (event.GetEventType())
        {
        case Core::EventType::KeyPressed:
        {
            auto ev = static_cast<Core::KeyPressedEvent &>(event);
            auto it = key_map.find(ev.GetKeyCode());
            if (it != key_map.end())
            {
                cameras_[active_camera_].CameraMove(it->second, true);
                return;
            }
            break;
        }
        case Core::EventType::KeyReleased:
        {
            auto ev = static_cast<Core::KeyReleasedEvent &>(event);
            auto it = key_map.find(ev.GetKeyCode());
            if (it != key_map.end())
            {
                cameras_[active_camera_].CameraMove(it->second, false);
                return;
            }
            break;
        }
        case Core::EventType::MouseMoved:
        {
            auto ev = static_cast<Core::MouseMovedEvent &>(event);
            cameras_[active_camera_].ProcessMousePosition(ev.GetX(), ev.GetY());
            return;
        }
        case Core::EventType::MouseScrolled:
        {
            auto ev = static_cast<Core::MouseScrolledEvent &>(event);
            cameras_[active_camera_].Zoom(ev.GetYOffset());
            return;
        }
        case Core::EventType::WindowResize:
        {
            auto ev = static_cast<Core::WindowResizeEvent &>(event);
            rctx_->aspect_ratio_ = static_cast<float>(ev.GetWidth()) / static_cast<float>(ev.GetHeight());
            cameras_[active_camera_].UpdateAspectRatio(rctx_->aspect_ratio_);
            return;
        }

        default:
            break;
        }

        for (std::shared_ptr<Core::Layer> &layer : std::views::reverse(layers_))
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

        SetBackgroundColor(Core::Color_A1::Lime);
    }

} // namespace Test
