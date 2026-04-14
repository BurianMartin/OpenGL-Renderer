#include "Camera.hpp"

Camera::Camera() : position(0.0f, 0.0f, 3.0f), front(0.0f, 0.0f, -1.0f), up(0.0f, 1.0f, 0.0f), fov(45.0f), near(0.1f), far(100.0f), aspect_ratio(16.0f / 9.0f), speed(2.5f)
{
}

glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::GetProjectionMatrix()
{
    return glm::perspective(fov, aspect_ratio, near, far);
}
