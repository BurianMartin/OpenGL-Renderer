#include "Core/Application.hpp"

namespace Core
{
    Application::Application(ApplicationSpecification specification) : specification_(specification)
    {
        Init();

        auto testScene = std::make_shared<Core::Scene>((float)specification_.windowSpec.width / (float)specification_.windowSpec.height);
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

        window_ = glfwCreateWindow(specification_.windowSpec.width, specification_.windowSpec.height, specification_.windowSpec.Title.c_str(), NULL, NULL);
        if (!window_)
        {
            glfwTerminate();

            debug_error("GLFW window creation error");

            return;
        }

        // glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Mouse lock for camera rotation
        glfwSwapInterval(1); // 1 = vsync on, 0 = uncapped

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
            GLfloat current_frame_time = glfwGetTime();
            GLfloat delta_time = current_frame_time - last_frame_time_;
            last_frame_time_ = current_frame_time;
            debug_info("delta_time: " << delta_time << " FPS: " << 1.0f / delta_time);

            scenes_[current_scene_]->Update(delta_time);
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
        // TODO: Replace if with a optional Event Handling function in app so App can handle events if needed
        if (event.GetEventType() == Core::EventType::WindowClose)
        {
            // I think this is redundant now, maybe edit to close on keys if needed
            glfwSetWindowShouldClose(window_, true);
        }

        if (current_scene_ == -1)
        {
            debug_warn("No active scene to raise event on");
        }

        scenes_[current_scene_]->HandleEvent(event);
    }

} // namespace Core
