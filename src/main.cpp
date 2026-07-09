#include "Forge/Engine.hpp"
#include "Demo/LightDemoLayer.hpp"
#include "Demo/LightDemoScene.hpp"
#include "Demo/MultiCameraDemoScene.hpp"
#include "Demo/OrthoDemoScene.hpp"

int main(void)
{
    Forge::Engine engine;

    // The first scene added boots as active; press Tab at runtime to cycle to the next one
    // (Engine::RaiseEvent -> SetScene((current_scene_ + 1) % scenes_.size())).
    engine.AddScene(std::make_shared<Demo::LightDemoScene>());
    engine.AddScene(std::make_shared<Demo::MultiCameraDemoScene>());
    engine.AddScene(std::make_shared<Demo::OrthoDemoScene>());

    engine.Run();

    return 0;
}
