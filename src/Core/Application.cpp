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

        // Capture and hide the mouse
        glfwSetInputMode(window_, GLFW_CURSOR, cursor_mode_);

        // glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Mouse lock for camera rotation
        glfwSwapInterval(0); // 1 = vsync on, 0 = uncapped

        glfwMakeContextCurrent(window_);

        EventHandler_ = std::make_shared<Core::EventHandler>(window_, specification_.windowSpec);

        if (!gladLoadGL(glfwGetProcAddress))
        {
            debug_error("Erorr while initializing GLAD");
        }

        glEnable(GL_DEPTH_TEST);

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
            GLfloat delta_time = ComputeDeltaTime();
#ifdef SHOW_FPS
            debug_info("FPS: " << 1.0f / delta_time);
#endif

            scenes_[current_scene_]->Update(delta_time);
            renderer_.RenderScene(scenes_[current_scene_]);
            glfwSwapBuffers(window_);
            glfwPollEvents();
        }
    }

    GLfloat Application::ComputeDeltaTime()
    {
        GLfloat now = glfwGetTime();
        GLfloat dt = now - last_frame_time_;
        last_frame_time_ = now;
        return dt;
    }

    void Application::Destroy()
    {
        for (auto scene : scenes_)
            scene->Destroy();

        scenes_.clear();  // Release all GL resources while context is still alive
        glfwTerminate();
    }

    void Application::AddScene(std::shared_ptr<Core::Scene> scene)
    {
        if (scenes_.empty())
        {
            current_scene_ = 0;
        }
        scenes_.push_back(scene);
    }

    void Application::RaiseEvent(Event &event)
    {
        // TODO: Replace if with a optional Event Handling function in app so App can handle events if needed
        if (event.GetEventType() == Core::EventType::KeyPressed)
        {
            auto ev = static_cast<KeyPressedEvent &>(event);
            switch (ev.GetKeyCode())
            {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window_, true); // Close the app with esc key press
                break;
            case GLFW_KEY_GRAVE_ACCENT: // Basicalyl a semicolon key, changing the cursor mode from captured to free
                if (cursor_mode_ == GLFW_CURSOR_NORMAL)
                    cursor_mode_ = GLFW_CURSOR_DISABLED;
                else
                    cursor_mode_ = GLFW_CURSOR_NORMAL;
                glfwSetInputMode(window_, GLFW_CURSOR, cursor_mode_);
                return;
                break;
            default:
                break;
            }
        }

        if (current_scene_ == -1)
        {
            debug_warn("No active scene to raise event on");
            return;
        }

        scenes_[current_scene_]->HandleEvent(event);
    }

} // namespace Core
