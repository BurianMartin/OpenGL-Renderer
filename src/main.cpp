#include "Core/Application.hpp"

int main(void)
{
    Core::Application App;

    App.Run();

    App.Destroy();

    return 0;
}