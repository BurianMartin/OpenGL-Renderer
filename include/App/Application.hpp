#pragma once

#include <memory>
#include <GLFW/glfw3.h>

#include "Utils.hpp"

class Application
{
private:
    GLFWwindow *window;

public:
    Application();
    ~Application();

    void Init();
    void Run();
};