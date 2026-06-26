#include "Core/Application.hpp"
#include "App/TestLayer.hpp"
#include "App/TestScene.hpp"

int main(void)
{
    Core::Application App;

    auto testScene = std::make_shared<Test::TestScene>();
    App.AddScene(testScene);

    App.Run();

    return 0;
}
