#include "App/Application.hpp"

void Solitare::Application::Run()
{
    while (!glfwWindowShouldClose(window))
    {
        renderer.RenderScene(scenes_[current_scene_]);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}