#pragma once
#include "Utils.hpp"

#include "Core/InputEvents.hpp"
#include "Core/Specifications.hpp"

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
        const WindowSpecification spec_;

    public:
        EventHandler() = default;
        EventHandler(GLFWwindow *window, const WindowSpecification spec);
        ~EventHandler() = default;

        void RaiseEvent(Event &e);
    };
} // namespace Core
