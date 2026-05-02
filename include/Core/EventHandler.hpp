#pragma once
#include "Utils.hpp"

#include "Core/Window.hpp"
#include "Core/InputEvents.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Core
{
    class EventHandler
    {
    private:
        GLFWwindow *window_;
        WindowSpecification spec_;

    public:
        EventHandler() = default;
        EventHandler(GLFWwindow *window, WindowSpecification spec);
        ~EventHandler() = default;

        void RaiseEvent(Event &e);
    };
} // namespace Core
