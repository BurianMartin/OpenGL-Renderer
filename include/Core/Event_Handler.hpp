#include "Utils.hpp"
#include "Core/InputEvents.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Core
{
    class Event_Handler
    {
    private:
        GLFWwindow *window_;

    public:
        Event_Handler();
        ~Event_Handler() = default;

        void RaiseEvent(Event &e);
    };
} // namespace Core
