#include "Core/Application.hpp"

namespace Core
{
    Application::Application(ApplicationSpecification specification) : specification_(specification)
    {
        Init();

        auto testScene = std::make_shared<Core::Scene>();
        auto testLayer = std::make_shared<Solitare::TestLayer>();

        testScene->AddLayer(testLayer);

        testScene->SetBackgroundColor(Solitare::Color_A1::Lime);

        AddScene(testScene);
    }

    void Application::Init()
    {
        if (!glfwInit())
        {
            debug_error("GLFW init error");
            return;
        }

        if (specification_.windowSpec.Title.empty())
        {
            specification_.windowSpec.Title = "OpenGL App";
        }
        specification_.windowSpec.EventCallback = [this](Event &event)
        { RaiseEvent(event); };

        window_ = glfwCreateWindow(1080, 1080, specification_.windowSpec.Title.c_str(), NULL, NULL);
        if (!window_)
        {
            glfwTerminate();

            debug_error("GLFW window creation error");

            return;
        }

        glfwMakeContextCurrent(window_);

        EventHandler_ = std::make_shared<Core::EventHandler>(window_, specification_.windowSpec);

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

        while (!glfwWindowShouldClose(window_))
        {
            renderer_.RenderScene(scenes_[current_scene_]);
            glfwSwapBuffers(window_);
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
        // TODO: Replace switch with a optional Event Handling function in app so App can handle events if needed
        switch (event.GetEventType())
        {
        case EventType::KeyPressed:
        {
            KeyPressedEvent ev = static_cast<KeyPressedEvent &>(event);
            if (ev.GetKeyCode() == GLFW_KEY_ESCAPE)
            {
                glfwSetWindowShouldClose(window_, true);
                return; // Consume event
            }
            break;
        }
        default:
            break;
        }

        if (current_scene_ == -1)
        {
            debug_warn("No active scene to raise event on");
        }

        scenes_[current_scene_]->HandleEvent(event);
    }

} // namespace Core
