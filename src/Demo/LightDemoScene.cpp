#include "Demo/LightDemoScene.hpp"
#include "Forge/Rendering/Camera.hpp"
#include "Forge/Lighting/Lights.hpp"
#include "Forge/Scene/DebugOverlayLayer.hpp"
#include "Forge/Scene/UILayer.hpp"
#include "Forge/Scene/DragLayer.hpp"
#include "Forge/Rendering/Panel.hpp"
#include "LightDemoScene.hpp"

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
            case Forge::Key::F3:
            {
                GLint layerIndex = GetLayerByName("Overlay");
                if (layerIndex == -1)
                {
                    debug_warn("Layer 'Overlay' not found");
                    return;
                }
                auto layer = layers_[layerIndex];

                layer->SetShow(!layer->IsShown());
                return;
            }
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
            // Camera-look only makes sense while the cursor is captured (FPS-style) --
            // Engine now forwards every MouseMoved regardless of cursor mode (a Normal-mode
            // UI, e.g. drag-and-drop, needs them too), so this scene has to make that call
            // itself via FrameContext::cursor_mode_ instead of relying on Engine to filter.
            if (fctx_->cursor_mode_ != Forge::CursorMode::Captured)
                break;
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
            if (!layer->OnEvent(event, fctx_))
            {
                // Event was consumed by this layer
                return;
            }
        }
    }

    void LightDemoScene::OnSceneBoot()
    {
        cameras_.emplace_back(Forge::Viewport(0.0f, 0.0f, 1.0f, 1.0f, fctx_->window_width_, fctx_->window_height_));

        auto lightDemoLayer = std::make_shared<Demo::LightDemoLayer>("LightDemoLayer", rmanager_);

        AddLayer(lightDemoLayer);

        // F3-toggled wireframe-AABB debug overlay (Forge::DebugOverlayLayer) — off by default,
        // named "Overlay" to match OnEvent's GetLayerByName("Overlay") lookup above.
        auto debugOverlay = std::make_shared<Forge::DebugOverlayLayer>(
            "Overlay", rmanager_, std::vector<std::shared_ptr<Forge::Layer>>{lightDemoLayer});
        debugOverlay->Hide();
        AddLayer(debugOverlay);

        // Always-on screen-space UI — draws over the 3D scene above it and gets first look
        // at clicks among the non-drag layers (Scene::OnEvent dispatches in reverse
        // registration order); the DragLayer below is registered after this one so a card
        // drag still takes priority over it.
        auto uiLayer = std::make_shared<Forge::UILayer>("UI");
        uiLayer->AddButton(lightDemoLayer->GetRandomizeButton());

        // Panel/DragLayer demo: two draggable cards, one drop zone. Exercises the new
        // screen-space quad primitive (Panel) and generic drag-and-drop (DragLayer) the
        // same way "Randomize Gold" above exercises Button/ClickableRegion — drag either
        // card onto the yellow-bordered zone to see it accepted (stays), drop it anywhere
        // else to see it snap back.
        constexpr float kZoneX = 300.0f, kZoneY = 120.0f, kZoneW = 140.0f, kZoneH = 140.0f;
        auto dropZoneVisual = Forge::Panel::Create(rmanager_, kZoneX, kZoneY, kZoneW, kZoneH,
                                                     glm::vec4(0.15f, 0.15f, 0.15f, 0.5f), glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), 3.0f);
        auto cardA = Forge::Panel::Create(rmanager_, 20.0f, 120.0f, 80.0f, 110.0f,
                                            glm::vec4(0.8f, 0.2f, 0.2f, 1.0f), glm::vec4(1.0f), 3.0f);
        auto cardB = Forge::Panel::Create(rmanager_, 110.0f, 120.0f, 80.0f, 110.0f,
                                            glm::vec4(0.2f, 0.3f, 0.8f, 1.0f), glm::vec4(1.0f), 3.0f);
        if (dropZoneVisual)
            uiLayer->AddPanel(dropZoneVisual);

        AddLayer(uiLayer);

        // Registered after uiLayer so cards draw on top of the drop-zone panel and get
        // first look at clicks (Scene::OnEvent dispatches in reverse registration order) —
        // same reasoning the comment above gives for uiLayer itself.
        if (dropZoneVisual && cardA && cardB)
        {
            auto dragLayer = std::make_shared<Forge::DragLayer>("Drag");
            dragLayer->AddDraggable("CardA", cardA);
            dragLayer->AddDraggable("CardB", cardB);
            dragLayer->AddDropZone("PlayZone", kZoneX, kZoneY, kZoneW, kZoneH);
            dragLayer->SetOnDrop([](const std::string &id, const std::string &zoneId)
                                  {
                                      bool accepted = zoneId == "PlayZone";
                                      debug_info(id << (accepted ? " dropped on PlayZone -- accepted"
                                                                  : " dropped outside PlayZone -- rejected, snapping back"));
                                      return accepted;
                                  });
            // Brightens the zone's border while a card is being dragged over it, back to
            // plain yellow otherwise -- proves OnHover independently of OnDrop.
            dragLayer->SetOnHover([dropZoneVisual](const std::string &, const std::string &zoneId)
                                   { dropZoneVisual->SetBorderColor(zoneId == "PlayZone" ? glm::vec4(0.3f, 1.0f, 0.3f, 1.0f)
                                                                                          : glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)); });
            AddLayer(dragLayer);
        }

        // Real sky instead of a flat color — also lets the directional light below read as
        // actual "sunlight" rather than an arbitrary vector.
        SetSkyboxBackground();

        // One light per station, positioned directly above (or, for the spot, aimed straight
        // down at) that station's cube in LightDemoLayer — so each light type's effect
        // dominates its own object instead of five lights blending across a scattered pile.
        // Point-light attenuation (default range ~50 units) means neighboring stations still
        // get some spill; that's realistic, not a bug — each station is *dominated* by its
        // own light, not exclusively lit by it.
        AddLight(Forge::PointLight::Create(glm::vec3(-2.5f, 2.0f, -1.0f), glm::vec3(1.0f, 0.85f, 0.6f), 2.0f)); // Station 1 — warm key, over the Gold cube
        AddLight(Forge::PointLight::Create(glm::vec3(2.5f, 2.0f, -3.5f), glm::vec3(0.75f, 0.85f, 1.0f), 2.0f)); // Station 2 — cool rim, over the Silver cube
        AddLight(Forge::PointLight::Create(glm::vec3(-2.5f, 2.0f, -6.0f), glm::vec3(1.0f, 0.6f, 0.65f), 2.0f)); // Station 3 — warm rim, over the Ruby cube
        AddLight(Forge::SpotLight::Create(glm::vec3(2.5f, 3.5f, -8.5f), glm::vec3(0.0f, -1.0f, 0.0f),
                                          glm::vec3(0.75f, 1.0f, 0.85f), 2.5f, 12.5f, 20.0f)); // Station 4 — spot, aimed straight down at the Emerald cube

        // "Sun" — illuminates every station (and the floor) uniformly regardless of position;
        // the skybox's own sun disc is aimed opposite this direction, so the two agree.
        AddLight(Forge::DirectionalLight::Create(glm::vec3(0.60f, -1.0f, 0.30f), glm::vec3(1.0f, 1.0f, 1.0f), 0.5f));
    }

    void LightDemoScene::OnResume(std::shared_ptr<Forge::FrameContext> fctx)
    {
    }

} // namespace Demo
