#pragma once

#include "Utils.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Forge
{
    /// Discriminant for every concrete Event subclass; see the
    /// `EVENT_CLASS_TYPE` macro for how it's wired to `GetEventType()`.
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

    /**
     * @brief Generates the boilerplate RTTI-style overrides every concrete
     * Event subclass needs.
     *
     * Invoke inside the class body with the matching `EventType` enumerator
     * name, e.g. `EVENT_CLASS_TYPE(KeyPressed)`. Defines `GetStaticType()`
     * (for `Is<T>`-style checks), `GetEventType()`, and `GetName()`.
     */
#define EVENT_CLASS_TYPE(type)                                                  \
    static EventType GetStaticType() { return EventType::type; }                \
    virtual EventType GetEventType() const override { return GetStaticType(); } \
    virtual const char *GetName() const override { return #type; }

    /**
     * @brief Abstract base for every input/window event raised by
     * EventHandler and dispatched through Engine/Scene/Layer.
     *
     * Concrete subclasses live in InputEvents.hpp and are built with
     * `EVENT_CLASS_TYPE`.
     */
    class Event
    {
    public:
        Event() = default;
        virtual ~Event() = default;

        /// @return The concrete type of this event, for switch-based dispatch.
        virtual EventType GetEventType() const = 0;

        /// @return The event's class name as a string (e.g. "KeyPressed").
        virtual const char *GetName() const = 0;

        /// @return A human-readable description; defaults to GetName(),
        /// subclasses override to include their data (e.g. key code).
        virtual std::string ToString() const { return GetName(); }
    };
} // namespace Forge
