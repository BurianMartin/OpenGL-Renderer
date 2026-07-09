#include "Demo/OrthoDemoScene.hpp"
#include "Forge/Camera.hpp"
#include "Forge/Lights.hpp"

namespace Demo
{
    void OrthoDemoScene::OnMouseCapture()
    {
    }

    void OrthoDemoScene::OnUpdate(float delta_time)
    {
    }

    void OrthoDemoScene::OnEvent(Forge::Event &event)
    {
        switch (event.GetEventType())
        {
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

    void OrthoDemoScene::OnSceneBoot()
    {
        // Fixed top-down camera — positioned once via SetPosition/SetYawPitch, never touched
        // again (see class doc). Looking straight down (pitch -90) makes yaw irrelevant to
        // front_, which is exactly the degenerate case SetUp exists to work around.
        cameras_.emplace_back(Forge::Viewport(0.0f, 0.0f, 1.0f, 1.0f, fctx_->window_width_, fctx_->window_height_));
        cameras_[0].SetPosition(glm::vec3(0.0f, 10.0f, -5.0f));
        cameras_[0].SetYawPitch(-90.0f, -90.0f);
        cameras_[0].SetUp(glm::vec3(0.0f, 0.0f, -1.0f)); // world -Z as screen "up" — (0,1,0) degenerates looking straight down
        cameras_[0].SetOrthographic(10.0f);              // half-height fits the floor's full ~18-unit depth (and, x aspect, its ~12-unit width) at 1:1 aspect

        auto lightDemoLayer = std::make_shared<Demo::LightDemoLayer>(rmanager_);
        AddLayer(lightDemoLayer);

        SetBackgroundColor(glm::vec4(0.05f, 0.05f, 0.08f, 1.0f));

        // Same light layout as LightDemoScene/MultiCameraDemoScene — see those for per-station reasoning.
        AddLight(Forge::PointLight::Create(glm::vec3(-2.5f, 2.0f, -1.0f), glm::vec3(1.0f, 0.85f, 0.6f), 2.0f));
        AddLight(Forge::PointLight::Create(glm::vec3(2.5f, 2.0f, -3.5f), glm::vec3(0.75f, 0.85f, 1.0f), 2.0f));
        AddLight(Forge::PointLight::Create(glm::vec3(-2.5f, 2.0f, -6.0f), glm::vec3(1.0f, 0.6f, 0.65f), 2.0f));
        AddLight(Forge::SpotLight::Create(glm::vec3(2.5f, 3.5f, -8.5f), glm::vec3(0.0f, -1.0f, 0.0f),
                                          glm::vec3(0.75f, 1.0f, 0.85f), 2.5f, 12.5f, 20.0f));
        AddLight(Forge::DirectionalLight::Create(glm::vec3(0.60f, -1.0f, 0.30f), glm::vec3(1.0f, 1.0f, 1.0f), 0.5f));
    }

} // namespace Demo
