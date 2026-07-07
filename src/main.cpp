#include "Core/Application.hpp"
#include "App/TestLayer.hpp"
#include "App/TestScene.hpp"

int main(void)
{
    Core::Application App;

    App.AddScene(std::make_shared<Test::TestScene>());

    App.Run();

    return 0;
}
