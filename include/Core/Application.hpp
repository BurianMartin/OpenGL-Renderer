#pragma once
#include "Utils.hpp"
#include "Scene.hpp"
#include "Renderer.hpp"
#include "App/TestLayer.hpp"
#include "Colors.hpp"
#include "Core/InputEvents.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <functional>

namespace Core
{

    // TODO: Moved the Application class from App to Core, make it more generic, integrate with events.

    struct WindowSpecification
    {
        std::string Title;
        uint32_t width;
        uint32_t height;
        bool isResizable = true;
        bool VSync = true;

        using EventCallbackFn = std::function<void(Event &)>;
        EventCallbackFn EventCallback;
    };

    class Application
    {
    private:
        Renderer renderer;
        GLFWwindow *window;

        std::vector<std::shared_ptr<Scene>> scenes_;

        GLint current_scene_ = 0;

        void Init();

    public:
        Application();
        ~Application();

        void Run();

        void Destroy();

        void AddScene(std::shared_ptr<Scene> scene); // Returns the scene ID in the vector of scenes
    };

} // namespace Core
