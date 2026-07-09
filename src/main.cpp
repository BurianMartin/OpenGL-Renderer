#include "Forge/Engine.hpp"
#include "App/TestLayer.hpp"
#include "App/TestScene.hpp"

int main(void)
{
    Forge::Engine App;

    App.AddScene(std::make_shared<Test::TestScene>());

    App.Run();

    return 0;
}
