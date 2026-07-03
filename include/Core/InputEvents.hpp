#pragma once

#include "Utils.hpp"
#include "Core/Event.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <format>

namespace Core
{
    /// Common base for KeyPressedEvent/KeyReleasedEvent — carries the GLFW key code.
    class KeyEvent : public Event
    {
    protected:
        KeyEvent(int keycode) : keycode_(keycode) {}
        GLint keycode_;

    public:
        /// @return The GLFW key code (e.g. `GLFW_KEY_W`) this event refers to.
        inline GLint GetKeyCode() const { return keycode_; }
    };

    /// Raised on key-down (and, per GLFW, on OS key-repeat while held).
    class KeyPressedEvent : public KeyEvent
    {
    private:
        bool IsRepeat_;

    public:
        KeyPressedEvent(int keycode, bool isRepeat) : KeyEvent(keycode), IsRepeat_(isRepeat) {}

        /// @return True if this is an OS-generated repeat while the key is held, not the initial press.
        inline bool IsRepeat() const { return IsRepeat_; }

        std::string ToString() const { return std::format("KeyPressedEvent: {} (repeat: {})", keycode_, IsRepeat_); }

        EVENT_CLASS_TYPE(KeyPressed);
    };

    /// Raised on key-up.
    class KeyReleasedEvent : public KeyEvent
    {
    public:
        KeyReleasedEvent(int keycode) : KeyEvent(keycode) {}

        std::string ToString() const { return std::format("KeyReleasedEvent: {}", keycode_); }

        EVENT_CLASS_TYPE(KeyReleased);
    };

    // ------------------- Mouse Events -------------------

    /// Raised on every cursor position update; coordinates are in screen space (pixels from top-left).
    class MouseMovedEvent : public Event
    {
    private:
        double XPos_, YPos_;

    public:
        MouseMovedEvent(double xpos, double ypos) : XPos_(xpos), YPos_(ypos) {}

        inline double GetX() const { return XPos_; }
        inline double GetY() const { return YPos_; }

        std::string ToString() const { return std::format("MouseMovedEvent: {}, {}", XPos_, YPos_); }

        EVENT_CLASS_TYPE(MouseMoved);
    };

    /// Raised on scroll-wheel/trackpad scroll input; consumed by Camera::Zoom for FOV control.
    class MouseScrolledEvent : public Event
    {
    private:
        double XOffset_, YOffset_;

    public:
        MouseScrolledEvent(double xoffset, double yoffset) : XOffset_(xoffset), YOffset_(yoffset) {}

        inline double GetXOffset() const { return XOffset_; }
        inline double GetYOffset() const { return YOffset_; }

        std::string ToString() const { return std::format("MouseScrolledEvent: {}, {}", XOffset_, YOffset_); }

        EVENT_CLASS_TYPE(MouseScrolled)
    };

    /// Common base for MouseButtonPressedEvent/MouseButtonReleasedEvent — carries the GLFW button code.
    class MouseButtonEvent : public Event
    {
    protected:
        GLint Button_;
        MouseButtonEvent(GLint button) : Button_(button) {}

    public:
        /// @return The GLFW mouse button code (e.g. `GLFW_MOUSE_BUTTON_LEFT`).
        inline int GetMouseButton() const { return Button_; }
    };

    /// Raised on mouse-button-down.
    class MouseButtonPressedEvent : public MouseButtonEvent
    {
    public:
        MouseButtonPressedEvent(GLint button) : MouseButtonEvent(button) {}
        ~MouseButtonPressedEvent() = default;

        std::string ToString() const { return std::format("MouseButtonPressedEvent: {}", Button_); }

        EVENT_CLASS_TYPE(MouseButtonPressed);
    };

    /// Raised on mouse-button-up.
    class MouseButtonReleasedEvent : public MouseButtonEvent
    {
    public:
        MouseButtonReleasedEvent(GLint button) : MouseButtonEvent(button) {}
        ~MouseButtonReleasedEvent() = default;

        std::string ToString() const { return std::format("MouseButtonReleasedEvent: {}", Button_); }

        EVENT_CLASS_TYPE(MouseButtonReleased);
    };

    /// Raised once when the user requests the window be closed (e.g. clicking the X). Application handles this by stopping the main loop.
    class WindowCloseEvent : public Event
    {
    private:
        /* data */
    public:
        WindowCloseEvent() = default;
        ~WindowCloseEvent() = default;

        std::string ToString() const { return "WindowCloseEvent"; }

        EVENT_CLASS_TYPE(WindowClose);
    };

    /// Raised whenever the window's framebuffer size changes; consumers should update aspect-ratio-dependent state (e.g. Camera's projection matrix).
    class WindowResizeEvent : public Event
    {
    private:
        int Width_, Height_;

    public:
        WindowResizeEvent(int width, int height) : Width_(width), Height_(height) {}

        inline int
        GetWidth() const
        {
            return Width_;
        }
        inline int GetHeight() const { return Height_; }

        std::string ToString() const { return std::format("WindowResizeEvent: {}, {}", Width_, Height_); }

        EVENT_CLASS_TYPE(WindowResize);
    };

} // namespace Core
