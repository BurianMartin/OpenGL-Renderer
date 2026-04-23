#include "App/Application.hpp"

Solitare::Application::Application()
{
    Init();

    auto testScene = std::make_shared<Core::Scene>();

    auto testLayer = std::make_shared<Solitare::BaseLayer>();

    testScene->AddLayer(testLayer);

    AddScene(testScene);
}

Solitare::Application::~Application()
{
}

void Solitare::Application::Init()
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

    debug_info("App init complete");
}

void Solitare::Application::Run()
{
    if (scenes_.empty())
    {
        debug_error("No scenes to render");
        return;
    }

    while (!glfwWindowShouldClose(window))
    {
        renderer.RenderScene(scenes_[current_scene_]);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
void Solitare::Application::Destroy()
{
    for (auto scene : scenes_)
    {
        scene->Destroy();
    }
}
void Solitare::Application::AddScene(std::shared_ptr<Core::Scene> scene)
{
    scenes_.push_back(scene);
}
