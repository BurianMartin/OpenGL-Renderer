#include "Core/EventHandler.hpp"

namespace Core
{
    EventHandler::EventHandler(GLFWwindow *window, const WindowSpecification spec)
        : window_(window), spec_(spec)
    {
        glfwSetWindowUserPointer(window_, this);

        glfwSetKeyCallback(window_, [](GLFWwindow *win, int key, int scancode, int action, int mods)
                           {
            EventHandler *EventHandler = static_cast<Core::EventHandler *>(glfwGetWindowUserPointer(win));
           
            switch (action)
            {
            case GLFW_PRESS:{
                KeyPressedEvent event(key, false);
                EventHandler->RaiseEvent(event);
                break;}

            case GLFW_RELEASE:
            {
                KeyReleasedEvent event(key);
                EventHandler->RaiseEvent(event);
                break;
            }
            case GLFW_REPEAT:
            {
                KeyPressedEvent event(key, true);
                EventHandler->RaiseEvent(event);
                break;
            }

                default:
                break;
            } });

        glfwSetWindowSizeCallback(window_, [](GLFWwindow *win, int width, int height)
                                  {
                                      EventHandler *EventHandler = static_cast<Core::EventHandler *>(glfwGetWindowUserPointer(win));
                                      // TODO: Add window size event creation here
                                      // WindowSizeEvent event(width, height);
                                      // EventHandler->RaiseEvent(event);
                                  });

        glfwSetMouseButtonCallback(window_, [](GLFWwindow *win, int button, int action, int mods)
                                   {
            EventHandler *EventHandler = static_cast<Core::EventHandler *>(glfwGetWindowUserPointer(win));
            
                    switch (action)
                    {
                    case GLFW_PRESS:{
                        MouseButtonPressedEvent event(button);
                        EventHandler->RaiseEvent(event);
                        break;}

                    case GLFW_RELEASE:
                        {
                        MouseButtonReleasedEvent event(button);
                        EventHandler->RaiseEvent(event);
                        break;
                        }
                        
                        default:
                        break;
                    } });

        glfwSetWindowCloseCallback(window_, [](GLFWwindow *win)
                                   {
                                       EventHandler *EventHandler = static_cast<Core::EventHandler *>(glfwGetWindowUserPointer(win));
                                       // TODO: Add window close event creation here
                                       // WindowCloseEvent event;
                                       // EventHandler->RaiseEvent(event);
                                   });

        glfwSetCursorPosCallback(window_, [](GLFWwindow *win, double xpos, double ypos)
                                 {
            EventHandler *EventHandler = static_cast<Core::EventHandler *>(glfwGetWindowUserPointer(win));
            MouseMovedEvent event(xpos, ypos);
            EventHandler->RaiseEvent(event); });

        glfwSetScrollCallback(window_, [](GLFWwindow *win, double xoffset, double yoffset)
                              {
            EventHandler *EventHandler = static_cast<Core::EventHandler *>(glfwGetWindowUserPointer(win));
            MouseScrolledEvent event(xoffset, yoffset);
            EventHandler->RaiseEvent(event); });
    }

    void EventHandler::RaiseEvent(Event &e)
    {
        debug_info(e.ToString());
        if (spec_.EventCallback)
        {
            spec_.EventCallback(e);
        }
    }
} // namespace Core
