#include "Demo/LightDemoScene.hpp"
#include "Forge/Camera.hpp"
#include "Forge/Lights.hpp"

namespace Demo
{
    void LightDemoScene::OnMouseCapture()
    {
        cameras_[active_camera_].ResetMouseTracking();
    }

    void LightDemoScene::OnUpdate(float delta_time)
    {
        cameras_[active_camera_].Update(delta_time);
    }

    void LightDemoScene::OnEvent(Forge::Event &event)
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

    void LightDemoScene::OnSceneBoot()
    {
        cameras_.emplace_back(Forge::Viewport(0.0f, 0.0f, 1.0f, 1.0f, fctx_->window_width_, fctx_->window_height_));

        auto lightDemoLayer = std::make_shared<Demo::LightDemoLayer>(rmanager_);

        AddLayer(lightDemoLayer);

        // Real sky instead of a flat color — also lets the directional light below read as
        // actual "sunlight" rather than an arbitrary vector.
        SetSkyboxBackground();

        // One light per station, positioned directly above (or, for the spot, aimed straight
        // down at) that station's cube in LightDemoLayer — so each light type's effect
        // dominates its own object instead of five lights blending across a scattered pile.
        // Point-light attenuation (default range ~50 units) means neighboring stations still
        // get some spill; that's realistic, not a bug — each station is *dominated* by its
        // own light, not exclusively lit by it.
        AddLight(Forge::PointLight::Create(glm::vec3(-2.5f, 2.0f, -1.0f), glm::vec3(1.0f, 0.85f, 0.6f), 2.0f));  // Station 1 — warm key, over the Gold cube
        AddLight(Forge::PointLight::Create(glm::vec3(2.5f, 2.0f, -3.5f), glm::vec3(0.75f, 0.85f, 1.0f), 2.0f));  // Station 2 — cool rim, over the Silver cube
        AddLight(Forge::PointLight::Create(glm::vec3(-2.5f, 2.0f, -6.0f), glm::vec3(1.0f, 0.6f, 0.65f), 2.0f));  // Station 3 — warm rim, over the Ruby cube
        AddLight(Forge::SpotLight::Create(glm::vec3(2.5f, 3.5f, -8.5f), glm::vec3(0.0f, -1.0f, 0.0f),
                                           glm::vec3(0.75f, 1.0f, 0.85f), 2.5f, 12.5f, 20.0f)); // Station 4 — spot, aimed straight down at the Emerald cube

        // "Sun" — illuminates every station (and the floor) uniformly regardless of position;
        // the skybox's own sun disc is aimed opposite this direction, so the two agree.
        AddLight(Forge::DirectionalLight::Create(glm::vec3(0.60f, -1.0f, 0.30f), glm::vec3(1.0f, 1.0f, 1.0f), 0.5f));
    }

} // namespace Demo
