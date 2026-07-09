#include "Forge/EventHandler.hpp"

namespace Forge
{
    namespace
    {
        /// Translates a GLFW key code into the engine's own Key — the only place in Forge that should ever do this.
        Key TranslateGLFWKey(int key)
        {
            switch (key)
            {
            case GLFW_KEY_A: return Key::A;
            case GLFW_KEY_B: return Key::B;
            case GLFW_KEY_C: return Key::C;
            case GLFW_KEY_D: return Key::D;
            case GLFW_KEY_E: return Key::E;
            case GLFW_KEY_F: return Key::F;
            case GLFW_KEY_G: return Key::G;
            case GLFW_KEY_H: return Key::H;
            case GLFW_KEY_I: return Key::I;
            case GLFW_KEY_J: return Key::J;
            case GLFW_KEY_K: return Key::K;
            case GLFW_KEY_L: return Key::L;
            case GLFW_KEY_M: return Key::M;
            case GLFW_KEY_N: return Key::N;
            case GLFW_KEY_O: return Key::O;
            case GLFW_KEY_P: return Key::P;
            case GLFW_KEY_Q: return Key::Q;
            case GLFW_KEY_R: return Key::R;
            case GLFW_KEY_S: return Key::S;
            case GLFW_KEY_T: return Key::T;
            case GLFW_KEY_U: return Key::U;
            case GLFW_KEY_V: return Key::V;
            case GLFW_KEY_W: return Key::W;
            case GLFW_KEY_X: return Key::X;
            case GLFW_KEY_Y: return Key::Y;
            case GLFW_KEY_Z: return Key::Z;

            case GLFW_KEY_0: return Key::Num0;
            case GLFW_KEY_1: return Key::Num1;
            case GLFW_KEY_2: return Key::Num2;
            case GLFW_KEY_3: return Key::Num3;
            case GLFW_KEY_4: return Key::Num4;
            case GLFW_KEY_5: return Key::Num5;
            case GLFW_KEY_6: return Key::Num6;
            case GLFW_KEY_7: return Key::Num7;
            case GLFW_KEY_8: return Key::Num8;
            case GLFW_KEY_9: return Key::Num9;

            case GLFW_KEY_F1: return Key::F1;
            case GLFW_KEY_F2: return Key::F2;
            case GLFW_KEY_F3: return Key::F3;
            case GLFW_KEY_F4: return Key::F4;
            case GLFW_KEY_F5: return Key::F5;
            case GLFW_KEY_F6: return Key::F6;
            case GLFW_KEY_F7: return Key::F7;
            case GLFW_KEY_F8: return Key::F8;
            case GLFW_KEY_F9: return Key::F9;
            case GLFW_KEY_F10: return Key::F10;
            case GLFW_KEY_F11: return Key::F11;
            case GLFW_KEY_F12: return Key::F12;

            case GLFW_KEY_SPACE: return Key::Space;
            case GLFW_KEY_TAB: return Key::Tab;
            case GLFW_KEY_ENTER: return Key::Enter;
            case GLFW_KEY_ESCAPE: return Key::Escape;
            case GLFW_KEY_BACKSPACE: return Key::Backspace;
            case GLFW_KEY_DELETE: return Key::Delete;
            case GLFW_KEY_INSERT: return Key::Insert;
            case GLFW_KEY_HOME: return Key::Home;
            case GLFW_KEY_END: return Key::End;
            case GLFW_KEY_PAGE_UP: return Key::PageUp;
            case GLFW_KEY_PAGE_DOWN: return Key::PageDown;
            case GLFW_KEY_CAPS_LOCK: return Key::CapsLock;

            case GLFW_KEY_UP: return Key::Up;
            case GLFW_KEY_DOWN: return Key::Down;
            case GLFW_KEY_LEFT: return Key::Left;
            case GLFW_KEY_RIGHT: return Key::Right;

            case GLFW_KEY_LEFT_SHIFT: return Key::LeftShift;
            case GLFW_KEY_RIGHT_SHIFT: return Key::RightShift;
            case GLFW_KEY_LEFT_CONTROL: return Key::LeftControl;
            case GLFW_KEY_RIGHT_CONTROL: return Key::RightControl;
            case GLFW_KEY_LEFT_ALT: return Key::LeftAlt;
            case GLFW_KEY_RIGHT_ALT: return Key::RightAlt;
            case GLFW_KEY_LEFT_SUPER: return Key::LeftSuper;
            case GLFW_KEY_RIGHT_SUPER: return Key::RightSuper;

            case GLFW_KEY_GRAVE_ACCENT: return Key::GraveAccent;
            case GLFW_KEY_MINUS: return Key::Minus;
            case GLFW_KEY_EQUAL: return Key::Equal;
            case GLFW_KEY_LEFT_BRACKET: return Key::LeftBracket;
            case GLFW_KEY_RIGHT_BRACKET: return Key::RightBracket;
            case GLFW_KEY_BACKSLASH: return Key::Backslash;
            case GLFW_KEY_SEMICOLON: return Key::Semicolon;
            case GLFW_KEY_APOSTROPHE: return Key::Apostrophe;
            case GLFW_KEY_COMMA: return Key::Comma;
            case GLFW_KEY_PERIOD: return Key::Period;
            case GLFW_KEY_SLASH: return Key::Slash;

            default: return Key::Unknown;
            }
        }

        /// Translates a GLFW mouse button code into the engine's own MouseButton.
        MouseButton TranslateGLFWMouseButton(int button)
        {
            switch (button)
            {
            case GLFW_MOUSE_BUTTON_LEFT: return MouseButton::Left;
            case GLFW_MOUSE_BUTTON_RIGHT: return MouseButton::Right;
            case GLFW_MOUSE_BUTTON_MIDDLE: return MouseButton::Middle;
            case GLFW_MOUSE_BUTTON_4: return MouseButton::Button4;
            case GLFW_MOUSE_BUTTON_5: return MouseButton::Button5;
            default: return MouseButton::Unknown;
            }
        }
    } // namespace

    EventHandler::EventHandler(GLFWwindow *window, const WindowSpecification spec)
        : window_(window), spec_(spec)
    {
        glfwSetWindowUserPointer(window_, this);

        glfwSetKeyCallback(window_, [](GLFWwindow *win, int key, int scancode, int action, int mods)
                           {
            EventHandler *handler = static_cast<Forge::EventHandler *>(glfwGetWindowUserPointer(win));
            Key translatedKey = TranslateGLFWKey(key);

            switch (action)
            {
            case GLFW_PRESS:{
                KeyPressedEvent event(translatedKey, false);
                handler->RaiseEvent(event);
                break;}

            case GLFW_RELEASE:
            {
                KeyReleasedEvent event(translatedKey);
                handler->RaiseEvent(event);
                break;
            }
            case GLFW_REPEAT:
            {
                KeyPressedEvent event(translatedKey, true);
                handler->RaiseEvent(event);
                break;
            }

                default:
                break;
            } });

        glfwSetWindowSizeCallback(window_, [](GLFWwindow *win, int width, int height)
                                  {
                                      EventHandler *handler = static_cast<Forge::EventHandler *>(glfwGetWindowUserPointer(win));
                                      WindowResizeEvent event(width, height);
                                      handler->RaiseEvent(event); });

        glfwSetMouseButtonCallback(window_, [](GLFWwindow *win, int button, int action, int mods)
                                   {
            EventHandler *handler = static_cast<Forge::EventHandler *>(glfwGetWindowUserPointer(win));
            MouseButton translatedButton = TranslateGLFWMouseButton(button);

                    switch (action)
                    {
                    case GLFW_PRESS:{
                        MouseButtonPressedEvent event(translatedButton);
                        handler->RaiseEvent(event);
                        break;}

                    case GLFW_RELEASE:
                        {
                        MouseButtonReleasedEvent event(translatedButton);
                        handler->RaiseEvent(event);
                        break;
                        }

                        default:
                        break;
                    } });

        glfwSetWindowCloseCallback(window_, [](GLFWwindow *win)
                                   {
                                       EventHandler *handler = static_cast<Forge::EventHandler *>(glfwGetWindowUserPointer(win));
 
                                       WindowCloseEvent event;
                                       handler->RaiseEvent(event); });

        glfwSetCursorPosCallback(window_, [](GLFWwindow *win, double xpos, double ypos)
                                 {
            EventHandler *handler = static_cast<Forge::EventHandler *>(glfwGetWindowUserPointer(win));
            MouseMovedEvent event(xpos, ypos);
            handler->RaiseEvent(event); });

        glfwSetScrollCallback(window_, [](GLFWwindow *win, double xoffset, double yoffset)
                              {
            EventHandler *handler = static_cast<Forge::EventHandler *>(glfwGetWindowUserPointer(win));
            MouseScrolledEvent event(xoffset, yoffset);
            handler->RaiseEvent(event); });
    }

    void EventHandler::RaiseEvent(Event &e)
    {
        debug_event(e.ToString());
        if (spec_.EventCallback)
        {
            spec_.EventCallback(e);
        }
    }
} // namespace Forge
