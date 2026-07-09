#pragma once
#include "Utils.hpp"

#include "Forge/InputEvents.hpp"
#include "Forge/Specifications.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Forge
{
    /**
     * @brief Wires GLFW's C callbacks to the engine's typed Event system.
     *
     * Registers `glfwSet*Callback` handlers on construction; each callback
     * constructs the matching Forge::Event subclass and forwards it to
     * `spec_.EventCallback` (ultimately `Engine::RaiseEvent`).
     */
    class EventHandler
    {
    private:
        GLFWwindow *window_;
        const WindowSpecification spec_;

    public:
        /**
         * @brief Registers GLFW callbacks on the given window.
         * @param window GLFW window to attach callbacks to; must outlive this EventHandler.
         * @param spec Window spec whose `EventCallback` receives every raised event.
         */
        EventHandler(GLFWwindow *window, const WindowSpecification spec);
        ~EventHandler() = default;

        /// Common exit point every registered GLFW callback funnels its constructed
        /// event through: logs it (debug builds, via `debug_event`) then forwards it
        /// to `spec_.EventCallback`. Public so callers can also inject a synthetic event.
        void RaiseEvent(Event &e);
    };
} // namespace Forge
