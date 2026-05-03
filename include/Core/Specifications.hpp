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
