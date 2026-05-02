#include "Core/Event_Handler.hpp"

namespace Core
{
    Event_Handler::Event_Handler()
    {
        glfwSetWindowUserPointer(window_, this);

        glfwSetKeyCallback(window_, [](GLFWwindow *win, int key, int scancode, int action, int mods)
                           {
            Event_Handler *event_handler = static_cast<Event_Handler *>(glfwGetWindowUserPointer(win));
           
            switch (action)
            {
            case GLFW_PRESS:{
                KeyPressedEvent event(key, false);
                event_handler->RaiseEvent(event);
                break;}

            case GLFW_RELEASE:
                {
                KeyReleasedEvent event(key);
                event_handler->RaiseEvent(event);
                break;
                }
                
                default:
                break;
            } });

        glfwSetWindowSizeCallback(window_, [](GLFWwindow *win, int width, int height)
                                  {
                                      Event_Handler *event_handler = static_cast<Event_Handler *>(glfwGetWindowUserPointer(win));
                                      // TODO: Add window size event creation here
                                      // WindowSizeEvent event(width, height);
                                      // event_handler->RaiseEvent(event);
                                  });

        glfwSetMouseButtonCallback(window_, [](GLFWwindow *win, int button, int action, int mods)
                                   {
            Event_Handler *event_handler = static_cast<Event_Handler *>(glfwGetWindowUserPointer(win));
            
                    switch (action)
                    {
                    case GLFW_PRESS:{
                        MouseButtonPressedEvent event(button);
                        event_handler->RaiseEvent(event);
                        break;}

                    case GLFW_RELEASE:
                        {
                        MouseButtonReleasedEvent event(button);
                        event_handler->RaiseEvent(event);
                        break;
                        }
                        
                        default:
                        break;
                    } });

        glfwSetWindowCloseCallback(window_, [](GLFWwindow *win)
                                   {
                                       Event_Handler *event_handler = static_cast<Event_Handler *>(glfwGetWindowUserPointer(win));
                                       // TODO: Add window close event creation here
                                       // WindowCloseEvent event;
                                       // event_handler->RaiseEvent(event);
                                   });

        glfwSetCursorPosCallback(window_, [](GLFWwindow *win, double xpos, double ypos)
                                 {
            Event_Handler *event_handler = static_cast<Event_Handler *>(glfwGetWindowUserPointer(win));
            MouseMovedEvent event(xpos, ypos);
            event_handler->RaiseEvent(event); });

        glfwSetScrollCallback(window_, [](GLFWwindow *win, double xoffset, double yoffset)
                              {
            Event_Handler *event_handler = static_cast<Event_Handler *>(glfwGetWindowUserPointer(win));
            MouseScrolledEvent event(xoffset, yoffset);
            event_handler->RaiseEvent(event); });
    }

    void Event_Handler::RaiseEvent(Event &e)
    {
    }

} // namespace Core
