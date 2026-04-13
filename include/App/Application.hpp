#pragma once

#include <memory>

#include "Utils.hpp"
#include "Scene.hpp"
#include "Renderer.hpp"

class Application
{
private:
    GLFWwindow *window;
    Renderer renderer;
    Scene scene;

    void Init();

public:
    Application();
    ~Application();

    void Run();

    void Handle_Events();
};