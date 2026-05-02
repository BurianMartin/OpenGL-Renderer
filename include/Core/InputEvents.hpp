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
    class KeyEvent : public Event
    {
    protected:
        KeyEvent(int keycode) : keycode_(keycode) {}
        GLint keycode_;

    public:
        inline GLint GetKeyCode() const { return keycode_; }
    };

    class KeyPressedEvent : public KeyEvent
    {
    private:
        bool IsRepeat_;

    public:
        KeyPressedEvent(int keycode, bool isRepeat) : KeyEvent(keycode), IsRepeat_(isRepeat) {}

        inline bool IsRepeat() const { return IsRepeat_; }

        std::string ToString() const { return std::format("KeyPressedEvent: {} (repeat: {})", keycode_, IsRepeat_); }

        EVENT_CLASS_TYPE(KeyPressed);
    };

    class KeyReleasedEvent : public KeyEvent
    {
    public:
        KeyReleasedEvent(int keycode) : KeyEvent(keycode) {}

        std::string ToString() const { return std::format("KeyReleasedEvent: {}", keycode_); }

        EVENT_CLASS_TYPE(KeyReleased);
    };

    // ------------------- Mouse Events -------------------

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

    class MouseButtonEvent : public Event
    {
    protected:
        GLint Button_;
        MouseButtonEvent(GLint button) : Button_(button) {}

    public:
        inline int GetMouseButton() const { return Button_; }
    };

    class MouseButtonPressedEvent : public MouseButtonEvent
    {
    public:
        MouseButtonPressedEvent(GLint button) : MouseButtonEvent(button) {}
        ~MouseButtonPressedEvent() = default;

        std::string ToString() const { return std::format("MouseButtonPressedEvent: {}", Button_); }

        EVENT_CLASS_TYPE(MouseButtonPressed);
    };

    class MouseButtonReleasedEvent : public MouseButtonEvent
    {
    public:
        MouseButtonReleasedEvent(GLint button) : MouseButtonEvent(button) {}
        ~MouseButtonReleasedEvent() = default;

        std::string ToString() const { return std::format("MouseButtonReleasedEvent: {}", Button_); }

        EVENT_CLASS_TYPE(MouseButtonReleased);
    };

} // namespace Core
