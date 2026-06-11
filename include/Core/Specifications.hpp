#pragma once

#include "Utils.hpp"
#include "Core/Event.hpp"

#include <vector>
#include <cstdint>
#include <functional>

namespace Core
{
    struct WindowSpecification
    {
        std::string Title = "OpenGL App";
        uint32_t width = 1080;
        uint32_t height = 1080;
        bool isResizable = true;
        bool VSync = true;

        using EventCallbackFn = std::function<void(Event &)>;
        EventCallbackFn EventCallback;
    };

    struct ApplicationSpecification
    {
        std::string appName = "Application";
        WindowSpecification windowSpec;
    };

} // namespace Core
