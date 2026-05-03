#pragma once
#include "Utils.hpp"
#include "Colors.hpp"

#include "Core/Scene.hpp"
#include "Core/Renderer.hpp"
#include "Core/EventHandler.hpp"
#include "Core/Specifications.hpp"

#include "App/TestLayer.hpp"

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

    class Application
    {
    private:
        Renderer renderer_;
        GLFWwindow *window_;
        std::shared_ptr<Core::EventHandler> EventHandler_;

        ApplicationSpecification specification_;

        std::vector<std::shared_ptr<Scene>> scenes_;

        GLint current_scene_ = -1;

        void Init();

    public:
        Application(ApplicationSpecification specification = ApplicationSpecification());
        ~Application() = default;

        void Run();

        void Destroy();

        void AddScene(std::shared_ptr<Scene> scene); // Returns the scene ID in the vector of scenes

        void RaiseEvent(Event &event);
    };

} // namespace Core
