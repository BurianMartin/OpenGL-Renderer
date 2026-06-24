#include "Core/Camera.hpp"

namespace Core
{

    Camera::Camera(GLfloat aspect_ratio) : aspect_ratio(aspect_ratio)
    {
        position = glm::vec3(0.0f, 0.0f, 3.0f); // 3 units back from origin
        front = glm::vec3(0.0f, 0.0f, -1.0f);   // looking into the screen
        up = glm::vec3(0.0f, 1.0f, 0.0f);       // y is up

        yaw = -90.0f;
        pitch = 0.0f;

        fov = 45.0f;
        speed = 1.0f;
        sensitivity = 0.1f;
        near_plane = 0.1f;
        far_plane = 100.0f;
    }

    glm::mat4 Camera::GetViewMatrix() const
    {
        return glm::lookAt(position, position + front, up);
    }

    glm::mat4 Camera::GetProjectionMatrix() const
    {
        return glm::perspective(glm::radians(fov), aspect_ratio, near_plane, far_plane);
    }

    void Camera::MoveForward(GLfloat delta_time)
    {
        position += front * speed * delta_time;
    }

    void Camera::MoveBackward(GLfloat delta_time)
    {
        position -= front * speed * delta_time;
    }

    void Camera::MoveDown(GLfloat delta_time)
    {
        position -= up * speed * delta_time;
    }

    void Camera::MoveUp(GLfloat delta_time)
    {
        position += up * speed * delta_time;
    }

    void Camera::ProcessMousePosition(float xpos, float ypos)
    {
        if (first_mouse_)
        {
            last_x_ = xpos;
            last_y_ = ypos;
            first_mouse_ = false;
            return;
        }

        float x_offset = xpos - last_x_;
        float y_offset = last_y_ - ypos;

        last_x_ = xpos;
        last_y_ = ypos;

        Rotate(x_offset, y_offset);
    }

    void Camera::MoveLeft(GLfloat delta_time)
    {
        position -= glm::normalize(glm::cross(front, up)) * speed * delta_time;
    }

    void Camera::MoveRight(GLfloat delta_time)
    {
        position += glm::normalize(glm::cross(front, up)) * speed * delta_time;
    }

    void Camera::Rotate(GLfloat x_offset, GLfloat y_offset)
    {
        yaw += x_offset * sensitivity;
        pitch += y_offset * sensitivity;

        // Limit pitch to avoid flipping
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        UpdateFront();
    }

    void Camera::Zoom(GLfloat y_offset)
    {
        fov -= y_offset;
        if (fov < 1.0f)
            fov = 1.0f;
        if (fov > 45.0f)
            fov = 45.0f;
    }

    glm::vec3 Camera::GetPosition() const
    {
        return position;
    }

    void Camera::Update(GLfloat delta_time)
    {
        if (move_forward)
            MoveForward(delta_time);
        if (move_backward)
            MoveBackward(delta_time);
        if (move_up)
            MoveUp(delta_time);
        if (move_down)
            MoveDown(delta_time);
        if (move_left)
            MoveLeft(delta_time);
        if (move_right)
            MoveRight(delta_time);

        // debug_info("Camera coord: " << position.x << ", " << position.y << ", " << position.z);
    }

    void Camera::CameraMove(CamMove direction, bool state)
    {
        switch (direction)
        {
        case CamMove::UP:
            move_up = state;
            break;

        case CamMove::DOWN:
            move_down = state;
            break;

        case CamMove::LEFT:
            move_left = state;
            break;

        case CamMove::RIGHT:
            move_right = state;
            break;

        case CamMove::FORWARD:
            move_forward = state;
            break;

        case CamMove::BACKWARD:
            move_backward = state;
            break;

        default:
            break;
        }
    }

    void Camera::UpdateFront()
    {
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(direction);
    }
}
