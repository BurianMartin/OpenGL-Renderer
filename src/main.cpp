#include "Forge/Core/Engine.hpp"
#include "Demo/LightDemoLayer.hpp"
#include "Demo/LightDemoScene.hpp"
#include "Demo/MultiCameraDemoScene.hpp"
#include "Demo/OrthoDemoScene.hpp"

int main(void)
{
    Forge::Engine engine;

    // Set up event handler for the whole engine to run on event raise
    auto eventHandler = [&engine](Forge::Event &event) -> bool
    {
        switch (event.GetEventType())
        {
        case Forge::EventType::KeyPressed:
        {
            auto ev = static_cast<Forge::KeyPressedEvent &>(event);
            switch (ev.GetKeyCode())
            {
            case Forge::Key::Escape:
                engine.CloseWindow();
                return false;

            case Forge::Key::Tab:

                engine.NextScene();
                return false;

            case Forge::Key::GraveAccent: // Basically a semicolon key, changing the cursor mode from captured to free
                if (engine.GetCursorMode() == Forge::CursorMode::Normal)
                {
                    engine.SetCursorMode(Forge::CursorMode::Captured);
                }
                else
                {
                    engine.SetCursorMode(Forge::CursorMode::Normal);
                }
                return false;

            default:
                break;
            }
            break;
        }
        // WindowResize's AdjustViewport() call is now fixed, always-on Engine bookkeeping
        // (Engine::RaiseEvent), not app policy — nothing to do here; just fall through to
        // `default:` and forward the event to the active scene so it can update its
        // camera(s)' aspect ratio.

        default:
            return true;
            break;
        }
        return true;
    };

    engine.SetEventHandler(eventHandler);

    // The first scene added boots as active; press Tab at runtime to cycle to the next one
    // (Engine::RaiseEvent -> SetScene((current_scene_ + 1) % scenes_.size())).
    engine.AddScene(std::make_shared<Demo::LightDemoScene>());
    engine.AddScene(std::make_shared<Demo::MultiCameraDemoScene>());
    engine.AddScene(std::make_shared<Demo::OrthoDemoScene>());

    engine.Run();

    return 0;
}
