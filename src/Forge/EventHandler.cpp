#include "Forge/EventHandler.hpp"

namespace Forge
{
    EventHandler::EventHandler(GLFWwindow *window, const WindowSpecification spec)
        : window_(window), spec_(spec)
    {
        glfwSetWindowUserPointer(window_, this);

        glfwSetKeyCallback(window_, [](GLFWwindow *win, int key, int scancode, int action, int mods)
                           {
            EventHandler *handler = static_cast<Forge::EventHandler *>(glfwGetWindowUserPointer(win));
            // Key's values are wired to match GLFW_KEY_* directly (see Keys.hpp) — no translation table needed.
            Key translatedKey = static_cast<Key>(key);

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
            // MouseButton's values are wired to match GLFW_MOUSE_BUTTON_* directly (see Keys.hpp) — no translation table needed.
            MouseButton translatedButton = static_cast<MouseButton>(button);

            // GLFW's mouse-button callback doesn't hand you a position — grab it explicitly,
            // so a click carries the screen point it happened at (needed for click-to-pick).
            double xpos, ypos;
            glfwGetCursorPos(win, &xpos, &ypos);

                    switch (action)
                    {
                    case GLFW_PRESS:{
                        MouseButtonPressedEvent event(translatedButton, xpos, ypos);
                        handler->RaiseEvent(event);
                        break;}

                    case GLFW_RELEASE:
                        {
                        MouseButtonReleasedEvent event(translatedButton, xpos, ypos);
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

        glfwSetWindowFocusCallback(window_, [](GLFWwindow *win, int focused)
                                   {
            EventHandler *handler = static_cast<Forge::EventHandler *>(glfwGetWindowUserPointer(win));
            if (!focused) 
                {
                    WindowLostFocusEvent event;
                    handler->RaiseEvent(event);
                } });
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
