#pragma once
#include "Utils.hpp"

#include "Core/Scene.hpp"
#include "Core/Renderer.hpp"
#include "Core/EventHandler.hpp"
#include "Core/Specifications.hpp"
#include "Core/ResourceManager.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <functional>

/**
 * @namespace Core
 * @brief Engine-side code: Application, Scene/Layer, rendering, resources, input events, lights/materials.
 *
 * Core has zero compile-time dependency on app-layer types — application-
 * specific code lives in the separate `Test` namespace instead
 * (`include/App/`, `src/App/`). See CLAUDE.md's "Core/App separation" notes.
 */
namespace Core
{
    /**
     * @brief Owns the GLFW window and every top-level engine subsystem; entry point for embedding this engine.
     *
     * Drives the main loop in `Run()`: `Scene::Update` -> `Renderer::RenderScene`
     * -> `glfwSwapBuffers` -> `glfwPollEvents`. Scenes are constructed
     * outside Application (typically in `main.cpp`, so core has zero
     * compile-time dependency on app-layer scene types) and registered via
     * `AddScene`. `RaiseEvent` intercepts ESC (close the window) and the
     * backtick key (toggle mouse capture) before forwarding anything else
     * to the active scene.
     *
     * @warning If `Init()` fails during construction, the constructor logs
     * via `debug_error` (which throws in debug builds) and returns early —
     * in a release build (where `debug_error` is a no-op) this leaves the
     * Application half-constructed rather than preventing its use.
     */
    class Application
    {
    private:
        ApplicationSpecification specification_;

        Renderer renderer_;

        GLFWwindow *window_;
        std::shared_ptr<EventHandler> EventHandler_;
        std::shared_ptr<ResourceManager> rmanager_;

        std::vector<std::shared_ptr<Scene>> scenes_;

        GLint current_scene_ = -1;
        GLfloat last_frame_time_ = 0.0f;

        GLint cursor_mode_ = GLFW_CURSOR_DISABLED;

        /// Initializes GLFW, creates the window, wires up EventHandler, loads GLAD, and enables depth testing + back-face culling. @return False if any step failed.
        bool Init();

        /// @return Seconds elapsed since the previous call (via `glfwGetTime()`), updating the stored last-frame timestamp.
        GLfloat ComputeDeltaTime();

    public:
        /// Constructs and initializes the window/GL context. See the class-level warning about `Init()` failure handling.
        Application(ApplicationSpecification specification = ApplicationSpecification());

        /// Destroys every scene, releases GL resources, and terminates GLFW.
        ~Application();

        /// Runs the main loop until the window is closed. @warning Logs and returns immediately (via `debug_error`) if no scene has been added yet.
        void Run();

        /// Dispatches an event: intercepts ESC/backtick, updates viewport on resize, then forwards to the active scene (suppressing `MouseMoved` while the cursor isn't captured).
        void RaiseEvent(Event &event);

        /// Registers a scene, making it active if it's the first one, and calls its `OnLoad`.
        int AddScene(std::shared_ptr<Scene> scene); // Returns the scene ID in the vector of scenes
    };

} // namespace Core
