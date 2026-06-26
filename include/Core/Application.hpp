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

namespace Core
{
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

        bool Init();
        GLfloat ComputeDeltaTime();

    public:
        Application(ApplicationSpecification specification = ApplicationSpecification());
        ~Application();

        void Run();

        void RaiseEvent(Event &event);

        int AddScene(std::shared_ptr<Scene> scene); // Returns the scene ID in the vector of scenes
    };

} // namespace Core
