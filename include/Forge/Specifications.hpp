#pragma once
#include "Utils.hpp"
#include "Forge/Event.hpp"

#include <vector>
#include <cstdint>
#include <functional>

namespace Forge
{
    /**
     * @brief Window configuration passed to Engine at construction.
     *
     * Everything here is a value the caller sets before the window is
     * created; there's no validation, so e.g. a zero width/height will be
     * forwarded straight to GLFW as-is.
     */
    struct WindowSpecification
    {
        std::string Title = "OpenGL App";
        uint32_t width = 1080;
        uint32_t height = 1080;
        bool isResizable = true;
        bool VSync = true;

        /// Signature for the function that receives every raised Event.
        using EventCallbackFn = std::function<void(Event &)>;

        /// Set by Engine before the window is created; EventHandler
        /// invokes this for every GLFW callback it receives.
        EventCallbackFn EventCallback;
    };

    /**
     * @brief Top-level configuration for an Engine instance.
     */
    struct ApplicationSpecification
    {
        std::string appName = "Engine";
        WindowSpecification windowSpec;
    };

} // namespace Forge
