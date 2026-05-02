#include "Core/Application.hpp"
#include "Application.hpp"

namespace Core
{
    Application::Application(WindowSpecification specification) : specification_(specification)
    {
        Init();

        auto testScene = std::make_shared<Core::Scene>();
        auto testLayer = std::make_shared<Solitare::TestLayer>();

        testScene->AddLayer(testLayer);

        testScene->SetBackgroundColor(Solitare::Color_A1::Lime);

        AddScene(testScene);
    }

    Application::~Application()
    {
    }

    void Application::Init()
    {
        if (!glfwInit())
        {
            debug_error("GLFW init error");
            return;
        }

        if (specification_.Title.empty())
        {
            specification_.Title = "OpenGL App";
        }
        specification_.EventCallback = [this](Event &event)
        { RaiseEvent(event); };

        window = glfwCreateWindow(1080, 1080, specification_.Title.c_str(), NULL, NULL);
        if (!window)
        {
            glfwTerminate();

            debug_error("GLFW window creation error");

            return;
        }

        glfwMakeContextCurrent(window);

        EventHandler_ = std::make_shared<Core::EventHandler>(window, specification_);

        if (!gladLoadGL(glfwGetProcAddress))
        {
            debug_error("Erorr while initializing GLAD");
        }

        debug_info("App init complete");
    }

    void Application::Run()
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
    void Application::Destroy()
    {
        for (auto scene : scenes_)
        {
            scene->Destroy();
        }
    }
    void Application::AddScene(std::shared_ptr<Core::Scene> scene)
    {
        current_scene_ = 0;
        scenes_.push_back(scene);
    }

    void Application::RaiseEvent(Event &event)
    {
        if (current_scene_ == -1)
        {
            debug_warn("No active scene to raise event on");
        }

        scenes_[current_scene_]->HandleEvent(event);
    }

} // namespace Solitare
