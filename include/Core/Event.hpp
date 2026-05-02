#pragma once

#include "Utils.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Core
{
    enum class EventType
    {
        None = 0,
        WindowClose,
        WindowResize,
        KeyPressed,
        KeyReleased,
        MouseButtonPressed,
        MouseButtonReleased,
        MouseMoved,
        MouseScrolled
    };

#define EVENT_CLASS_TYPE(type)                                                  \
    static EventType GetStaticType() { return EventType::type; }                \
    virtual EventType GetEventType() const override { return GetStaticType(); } \
    virtual const char *GetName() const override { return #type; }

    class Event
    {
    public:
        bool handled_ = false;

        Event() = default;
        virtual ~Event() = default;

        virtual EventType GetEventType() const = 0;
        virtual const char *GetName() const = 0;
        virtual std::string ToString() const { return GetName(); }
    };
} // namespace Core
