#include "Forge/Core/Engine.hpp"

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

        specification_.windowSpec.isResizable ? glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE) : glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window_ = glfwCreateWindow(specification_.windowSpec.width, specification_.windowSpec.height, specification_.windowSpec.Title.c_str(), NULL, NULL);
        if (!window_)
        {
            glfwTerminate();

            debug_error("GLFW window creation error");

            return false;
        }

        // Capture and hide the mouse
        glfwSetInputMode(window_, GLFW_CURSOR, static_cast<int>(cursor_mode_));

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
                scenes_[current_scene_]->Suspend();
                current_scene_ = next_scene_;
                scenes_[current_scene_]->Resume();
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
            current_scene_ = 0;
        }
        scenes_.push_back(scene);
        scene->OnLoad(rmanager_, renderer_.GetFrameContext());
        return scenes_.size() - 1;
    }

    void Engine::SetScene(GLint index)
    {
        if (index >= 0 && index < static_cast<GLint>(scenes_.size()) && index != current_scene_)
        {
            debug_info("Switching to scene " << index);
            next_scene_ = index;
        }
    }

    bool Engine::HandleEvent(Event &event)
    {
        if (event_handler_)
        {
            return event_handler_(event);
        }
        return false;
    }

    bool Engine::IsPressed(Forge::Key key)
    {
        return key_states_[static_cast<size_t>(key)] == KeyState::Pressed || key_states_[static_cast<size_t>(key)] == KeyState::Repeat;
    }

    bool Engine::IsRepeat(Forge::Key key)
    {
        return key_states_[static_cast<size_t>(key)] == KeyState::Repeat;
    }

    void Engine::RaiseEvent(Event &event)
    {
        if (event.GetEventType() == Forge::EventType::WindowLostFocus)
        {
            key_states_.fill(KeyState::Released);
        }
        else if (event.GetEventType() == Forge::EventType::WindowResize)
        {
            // Fixed, always-on engine bookkeeping — same tier as WindowLostFocus above, not
            // app policy. Previously this only happened because main.cpp's registered
            // SetEventHandler callback called AdjustViewport() itself; a consumer app that
            // didn't replicate that got a silently stale viewport/FrameContext on resize.
            auto ev = static_cast<Forge::WindowResizeEvent &>(event);
            AdjustViewport(ev.GetWidth(), ev.GetHeight());
        }
        else if (event.GetEventType() == Forge::EventType::KeyPressed)
        {
            auto ev = static_cast<Forge::KeyPressedEvent &>(event);
            if (ev.GetKeyCode() == Forge::Key::Unknown)
            {
                return;
            }
            else if (ev.IsRepeat())
            {
                key_states_[static_cast<size_t>(ev.GetKeyCode())] = KeyState::Repeat;
            }
            else
            {
                key_states_[static_cast<size_t>(ev.GetKeyCode())] = KeyState::Pressed;
            }
        }
        else if (event.GetEventType() == Forge::EventType::KeyReleased)
        {
            auto ev = static_cast<Forge::KeyReleasedEvent &>(event);
            if (ev.GetKeyCode() == Forge::Key::Unknown)
            {
                return;
            }
            key_states_[static_cast<size_t>(ev.GetKeyCode())] = KeyState::Released;
        }

        if (!HandleEvent(event))
        {
            return;
        }

        if (current_scene_ == -1)
        {
            debug_warn("No active scene to raise event on");
            return;
        }

        scenes_[current_scene_]->OnEvent(event);
    }

    void Engine::AdjustViewport(GLint width, GLint height)
    {
        glViewport(0, 0, width, height);
        auto fctx = renderer_.GetFrameContext();
        fctx->window_width_ = width;
        fctx->window_height_ = height;
    }

    void Engine::CloseWindow()
    {
        glfwSetWindowShouldClose(window_, true);
    }

    void Engine::NextScene()
    {
        SetScene((current_scene_ + 1) % scenes_.size());
    }
    void Engine::PrevScene()
    {
        SetScene((current_scene_ - 1 + scenes_.size()) % scenes_.size());
    }

    void Engine::SetCursorMode(Forge::CursorMode mode)
    {
        cursor_mode_ = mode;
        renderer_.GetFrameContext()->cursor_mode_ = mode;
        if (current_scene_ != -1)
        {
            scenes_[current_scene_]->OnMouseCapture();
        }

        glfwSetInputMode(window_, GLFW_CURSOR, static_cast<int>(cursor_mode_));
    }

    Forge::CursorMode Engine::GetCursorMode() const
    {
        return cursor_mode_;
    }

    void Engine::SetEventHandler(std::function<bool(Event &)> handler)
    {
        event_handler_ = handler;
    }

} // namespace Forge
