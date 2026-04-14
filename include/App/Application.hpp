#pragma once

#include <memory>

#include "Utils.hpp"
#include "Scene.hpp"
#include "Renderer.hpp"

#define NEXT_SCENE 0b00000001
#define PREV_SCENE 0b00000010

class Application
{
private:
    GLFWwindow *window;
    Renderer renderer;
    std::vector<Scene> scenes;

    GLuint current_scene;

    uint8_t flags;

    void Init();
    void Handle_Internal_Events();
    void Clear_Flags();

public:
    Application();
    ~Application();

    void Run();

    void Next_Scene();
    void Prev_Scene();

    enum class Direction
    {
        Left = 0,
        Right,
        Down,
        Up,
        Forward,
        Backward
    };

    void Move_Cam(Direction dir);

    static void Handle_Key_Events(GLFWwindow *window, int key, int scancode, int action, int mods);
};