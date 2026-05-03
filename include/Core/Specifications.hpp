#pragma once

#include "Utils.hpp"
#include "Core/Event.hpp"

#include <vector>
#include <cstdint>
#include <functional>

namespace Core
{
    // TODO: Add ApplicationSpecification struct that contains WindowSpecification, make it more robust with the app and event system, !URGENT!

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

    struct ApplicationSpecification
    {
        std::string appName = "Application";
        WindowSpecification windowSpec;
    };

} // namespace Core
