#include "Core/Application.hpp"

namespace Core
{
    Application::Application(ApplicationSpecification specification)
        : specification_(specification), renderer_((float)specification_.windowSpec.width / (float)specification_.windowSpec.height),
          rmanager_(std::make_shared<Core::ResourceManager>())
    {

        if (!Init())
        {
            debug_error("Failed to initialize application");
            return;
        }
    }

    bool Application::Init()
    {
        if (!glfwInit())
        {
            debug_error("GLFW init error");
            return false;
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

            return false;
        }

        // Capture and hide the mouse
        glfwSetInputMode(window_, GLFW_CURSOR, cursor_mode_);

        glfwMakeContextCurrent(window_);

        specification_.windowSpec.VSync ? glfwSwapInterval(1) : glfwSwapInterval(0); // 1 = vsync on, 0 = uncapped

        EventHandler_ = std::make_shared<Core::EventHandler>(window_, specification_.windowSpec);

        if (!gladLoadGL(glfwGetProcAddress))
        {
            debug_error("Erorr while initializing GLAD");
            return false;
        }

        glEnable(GL_DEPTH_TEST);

        debug_info("App init complete");
        return true;
    }

    void Application::Run()
    {
        if (scenes_.empty())
        {
            debug_error("No scenes to render");
            return;
        }

        last_frame_time_ = glfwGetTime();
        while (!glfwWindowShouldClose(window_))
        {
            GLfloat delta_time = ComputeDeltaTime();
#ifdef SHOW_FPS
            debug_info("FPS: " << 1.0f / delta_time);
#endif

            scenes_[current_scene_]->Update(delta_time);
            renderer_.RenderScene(scenes_[current_scene_], delta_time);
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

    Application::~Application()
    {
        if (!window_)
            return;

        for (auto scene : scenes_)
            scene->Destroy();

        scenes_.clear(); // Release all GL resources while context is still alive
        glfwTerminate(); // Must run before EventHandler_ is released — prevents callbacks firing on a dead pointer
        window_ = nullptr;
    }

    int Application::AddScene(std::shared_ptr<Core::Scene> scene)
    {
        if (scenes_.empty())
        {
            current_scene_ = 0;
        }
        scenes_.push_back(scene);
        scene->OnLoad(rmanager_, renderer_.GetRenderContext());
        return scenes_.size() - 1;
    }

    void Application::RaiseEvent(Event &event)
    {

        switch (event.GetEventType())
        {
        case Core::EventType::KeyPressed:
        {
            auto ev = static_cast<KeyPressedEvent &>(event);
            switch (ev.GetKeyCode())
            {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window_, true); // Close the app with esc key press
                return;
                break;
            case GLFW_KEY_GRAVE_ACCENT: // Basicalyl a semicolon key, changing the cursor mode from captured to free
                if (cursor_mode_ == GLFW_CURSOR_NORMAL)
                {
                    cursor_mode_ = GLFW_CURSOR_DISABLED;

                    if (current_scene_ != -1)
                    {
                        scenes_[current_scene_]->OnMouseCapture();
                    }
                }
                else
                {
                    cursor_mode_ = GLFW_CURSOR_NORMAL;
                }
                glfwSetInputMode(window_, GLFW_CURSOR, cursor_mode_);

                break;
            default:
                break;
            }
            break;
        }
        case Core::EventType::WindowClose:
        {
            glfwSetWindowShouldClose(window_, true);
            return;
            break;
        }
        case Core::EventType::WindowResize:
        {
            auto ev = static_cast<WindowResizeEvent &>(event);
            glViewport(0, 0, ev.GetWidth(), ev.GetHeight());
            break; // forward to scene so it can update aspect ratio
        }

        default:
            break;
        }

        if (current_scene_ == -1)
        {
            debug_warn("No active scene to raise event on");
            return;
        }

        // Only rotate camera when cursor is captured
        if (event.GetEventType() == Core::EventType::MouseMoved && cursor_mode_ != GLFW_CURSOR_DISABLED)
            return;

        scenes_[current_scene_]->OnEvent(event);
    }

} // namespace Core
