#pragma once
#include "Utils.hpp"
#include "Core/Event.hpp"

#include <vector>
#include <cstdint>
#include <functional>

namespace Core
{
    /**
     * @brief Window configuration passed to Application at construction.
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
        bool VSync = false;

        /// Signature for the function that receives every raised Event.
        using EventCallbackFn = std::function<void(Event &)>;

        /// Set by Application before the window is created; EventHandler
        /// invokes this for every GLFW callback it receives.
        EventCallbackFn EventCallback;
    };

    /**
     * @brief Top-level configuration for an Application instance.
     */
    struct ApplicationSpecification
    {
        std::string appName = "Application";
        WindowSpecification windowSpec;
    };

} // namespace Core
