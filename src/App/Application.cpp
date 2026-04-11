#include "Application.hpp"

Application::Application()
{
    debug_info("App created");
    debug_warn("App created");
    debug_error("App created");
}

Application::~Application() {}

void Application::Init()
{
    if (!glfwInit())
    {
        debug_error("GLFW init error");
        return;
    }

    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();

        debug_error("GLFW window creation error");

        return;
    }
}

void Application::Run()
{
    glfwMakeContextCurrent(window);
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return;
}