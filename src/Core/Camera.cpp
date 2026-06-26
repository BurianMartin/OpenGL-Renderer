#include "Core/Camera.hpp"

namespace Core
{

    Camera::Camera(GLfloat aspect_ratio) : aspect_ratio_(aspect_ratio)
    {
        position_ = glm::vec3(0.0f, 0.0f, 3.0f); // 3 units back from origin
        front_ = glm::vec3(0.0f, 0.0f, -1.0f);   // looking into the screen
        up_ = glm::vec3(0.0f, 1.0f, 0.0f);       // y is up

        yaw_ = -90.0f;
        pitch_ = 0.0f;

        fov_ = 45.0f;
        speed_ = 1.0f;
        sensitivity_ = 0.1f;
        near_plane_ = 0.1f;
        far_plane_ = 100.0f;
    }

    glm::mat4 Camera::GetViewMatrix() const
    {
        return glm::lookAt(position_, position_ + front_, up_);
    }

    glm::mat4 Camera::GetProjectionMatrix() const
    {
        return glm::perspective(glm::radians(fov_), aspect_ratio_, near_plane_, far_plane_);
    }

    void Camera::MoveForward(GLfloat delta_time)
    {
        position_ += front_ * speed_ * delta_time;
    }

    void Camera::MoveBackward(GLfloat delta_time)
    {
        position_ -= front_ * speed_ * delta_time;
    }

    void Camera::MoveDown(GLfloat delta_time)
    {
        position_ -= up_ * speed_ * delta_time;
    }

    void Camera::MoveUp(GLfloat delta_time)
    {
        position_ += up_ * speed_ * delta_time;
    }

    void Camera::ResetMouseTracking()
    {
        first_mouse_ = true;
    }

    void Camera::UpdateAspectRatio(float aspect_ratio)
    {
        aspect_ratio_ = aspect_ratio;
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
        position_ -= glm::normalize(glm::cross(front_, up_)) * speed_ * delta_time;
    }

    void Camera::MoveRight(GLfloat delta_time)
    {
        position_ += glm::normalize(glm::cross(front_, up_)) * speed_ * delta_time;
    }

    void Camera::Rotate(GLfloat x_offset, GLfloat y_offset)
    {
        yaw_ += x_offset * sensitivity_;
        pitch_ += y_offset * sensitivity_;

        // Limit pitch to avoid flipping
        if (pitch_ > 89.0f)
            pitch_ = 89.0f;
        if (pitch_ < -89.0f)
            pitch_ = -89.0f;

        UpdateFront();
    }

    void Camera::Zoom(GLfloat y_offset)
    {
        fov_ -= y_offset;
        if (fov_ < 1.0f)
            fov_ = 1.0f;
        if (fov_ > 45.0f)
            fov_ = 45.0f;
    }

    glm::vec3 Camera::GetPosition() const
    {
        return position_;
    }

    void Camera::Update(GLfloat delta_time)
    {
        if (move_forward_)
            MoveForward(delta_time);
        if (move_backward_)
            MoveBackward(delta_time);
        if (move_up_)
            MoveUp(delta_time);
        if (move_down_)
            MoveDown(delta_time);
        if (move_left_)
            MoveLeft(delta_time);
        if (move_right_)
            MoveRight(delta_time);

        // debug_info("Camera coord: " << position.x << ", " << position.y << ", " << position.z);
    }

    void Camera::CameraMove(CamMove direction, bool state)
    {
        switch (direction)
        {
        case CamMove::UP:
            move_up_ = state;
            break;

        case CamMove::DOWN:
            move_down_ = state;
            break;

        case CamMove::LEFT:
            move_left_ = state;
            break;

        case CamMove::RIGHT:
            move_right_ = state;
            break;

        case CamMove::FORWARD:
            move_forward_ = state;
            break;

        case CamMove::BACKWARD:
            move_backward_ = state;
            break;

        default:
            break;
        }
    }

    void Camera::UpdateFront()
    {
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
        direction.y = sin(glm::radians(pitch_));
        direction.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
        front_ = glm::normalize(direction);
    }
}
