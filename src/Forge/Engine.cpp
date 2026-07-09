#include "Forge/Engine.hpp"
#include "Engine.hpp"

namespace Forge
{
    Engine::Engine(ApplicationSpecification specification)
        : specification_(specification), renderer_(specification_.windowSpec.width, specification_.windowSpec.height),
          rmanager_(std::make_shared<Forge::ResourceManager>())
    {
        if (!Init())
        {
            debug_error("Failed to initialize application"); // still throws+logs in debug; no-op in release
            throw std::runtime_error("Engine failed to initialize");
        }
    }

    bool Engine::Init()
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

        EventHandler_ = std::make_shared<Forge::EventHandler>(window_, specification_.windowSpec);

        if (!gladLoadGL(glfwGetProcAddress))
        {
            debug_error("Erorr while initializing GLAD");
            return false;
        }

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        renderer_.GetFrameContext()->CreateLightBuffer();

        debug_info("App init complete");
        return true;
    }

    void Engine::Run()
    {

        if (scenes_.empty())
        {
            debug_error("No scenes to render");
            return;
        }

        last_frame_time_ = glfwGetTime();
#ifdef SHOW_FPS
        int fps_frame_count = 0;
        GLfloat fps_accum = 0.0f;
#endif
        while (!glfwWindowShouldClose(window_))
        {
            GLfloat delta_time = ComputeDeltaTime();
#ifdef SHOW_FPS
            fps_accum += delta_time;
            fps_frame_count++;
            if (fps_accum >= 1.0f)
            {
                debug_info("FPS: " << fps_frame_count / fps_accum);
                fps_frame_count = 0;
                fps_accum = 0.0f;
            }
#endif
            if (current_scene_ != next_scene_)
            {
                current_scene_ = next_scene_;
                scenes_[current_scene_]->ResetMouse();
            }
            scenes_[current_scene_]->Update(delta_time);
            renderer_.RenderScene(scenes_[current_scene_], delta_time);
            glfwSwapBuffers(window_);
            glfwPollEvents();
        }
    }

    GLfloat Engine::ComputeDeltaTime()
    {
        GLfloat now = glfwGetTime();
        GLfloat dt = now - last_frame_time_;
        last_frame_time_ = now;
        return dt;
    }

    Engine::~Engine()
    {
        if (!window_)
            return;

        for (auto scene : scenes_)
            scene->Destroy();

        scenes_.clear(); // Release all GL resources while context is still alive
        glfwTerminate(); // Must run before EventHandler_ is released — prevents callbacks firing on a dead pointer
        window_ = nullptr;
    }

    int Engine::AddScene(std::shared_ptr<Forge::Scene> scene)
    {
        if (scenes_.empty())
        {
            next_scene_ = 0;
        }
        scenes_.push_back(scene);
        scene->OnLoad(rmanager_, renderer_.GetFrameContext());
        return scenes_.size() - 1;
    }

    void Engine::SetScene(GLint index)
    {
        if (index >= 0 && index < static_cast<GLint>(scenes_.size()))
        {
            debug_info("Switching to scene " << index);
            next_scene_ = index;
        }
    }

    void Engine::RaiseEvent(Event &event)
    {

        switch (event.GetEventType())
        {
        case Forge::EventType::KeyPressed:
        {
            auto ev = static_cast<KeyPressedEvent &>(event);
            switch (ev.GetKeyCode())
            {
            case Forge::Key::Escape:
                glfwSetWindowShouldClose(window_, true); // Close the app with esc key press
                return;
                break;

            case Forge::Key::Tab:
                SetScene((current_scene_ + 1) % scenes_.size());
                break;

            case Forge::Key::GraveAccent: // Basicalyl a semicolon key, changing the cursor mode from captured to free
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
        case Forge::EventType::WindowClose:
        {
            glfwSetWindowShouldClose(window_, true);
            return;
            break;
        }
        case Forge::EventType::WindowResize:
        {
            auto ev = static_cast<WindowResizeEvent &>(event);
            glViewport(0, 0, ev.GetWidth(), ev.GetHeight());
            auto fctx = renderer_.GetFrameContext();
            fctx->window_width_ = ev.GetWidth();
            fctx->window_height_ = ev.GetHeight();
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
        if (event.GetEventType() == Forge::EventType::MouseMoved && cursor_mode_ != GLFW_CURSOR_DISABLED)
            return;

        scenes_[current_scene_]->OnEvent(event);
    }

} // namespace Forge
