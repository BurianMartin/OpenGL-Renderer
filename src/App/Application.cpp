#include "Application.hpp"

Application::Application()
{

    Init();

    float vertices[] = {
        0.0f, 0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f};

    Model triangle(vertices, sizeof(vertices), nullptr, 0, GL_TRIANGLES);

    scene.Add_Model(std::move(triangle), std::make_unique<Shader>("/home/buri/OpenGL_Experiments/shaders/vertex.glsl", "/home/buri/OpenGL_Experiments/shaders/time_color.glsl"));
}

Application::~Application() {}

void Application::Init()
{
    if (!glfwInit())
    {
        debug_error("GLFW init error");
        return;
    }

    window = glfwCreateWindow(1080, 1080, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();

        debug_error("GLFW window creation error");

        return;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGL(glfwGetProcAddress))
    {
        debug_error("Erorr while initializing GLAD");
    }

    renderer.Init();
}

void Application::Run()
{

    while (!glfwWindowShouldClose(window))
    {
        renderer.Render_Frame(scene);
        /* Render here */

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        Handle_Events();
    }
    scene.Clear();
    glfwTerminate();
    return;
}

void Application::Handle_Events()
{
}
