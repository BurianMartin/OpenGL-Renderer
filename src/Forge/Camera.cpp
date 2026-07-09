#include "Forge/Camera.hpp"
#include "Camera.hpp"

namespace Forge
{

    Camera::Camera(Viewport viewport) : viewport_(viewport)
    {
        position_ = glm::vec3(0.0f, 0.0f, 3.0f); // 3 units back from origin
        front_ = glm::vec3(0.0f, 0.0f, -1.0f);   // looking into the screen
        up_ = glm::vec3(0.0f, 1.0f, 0.0f);       // y is up

        yaw_ = -90.0f;
        pitch_ = 0.0f;

        fov_ = 45.0f;
        speed_ = 1.0f;
        boost_ = 0.0f;
        sensitivity_ = 0.1f;
        near_plane_ = 0.1f;
        far_plane_ = 100.0f;
        is_orthographic_ = false;
        ortho_half_height_ = 5.0f;
    }

    void Camera::SetOrthographic(GLfloat half_height)
    {
        is_orthographic_ = true;
        ortho_half_height_ = half_height;
    }

    void Camera::SetUp(const glm::vec3 &up)
    {
        up_ = glm::normalize(up);
    }

    glm::mat4 Camera::GetViewMatrix() const
    {
        return glm::lookAt(position_, position_ + front_, up_);
    }

    glm::mat4 Camera::GetProjectionMatrix() const
    {
        if (is_orthographic_)
        {
            GLfloat halfWidth = ortho_half_height_ * viewport_.GetAspectRatio();
            return glm::ortho(-halfWidth, halfWidth, -ortho_half_height_, ortho_half_height_, near_plane_, far_plane_);
        }
        return glm::perspective(glm::radians(fov_), viewport_.GetAspectRatio(), near_plane_, far_plane_);
    }

    void Camera::MoveForward(GLfloat delta_time)
    {
        position_ += front_ * (speed_ + boost_) * delta_time;
    }

    void Camera::MoveBackward(GLfloat delta_time)
    {
        position_ -= front_ * (speed_ + boost_) * delta_time;
    }

    void Camera::MoveDown(GLfloat delta_time)
    {
        position_ -= up_ * (speed_ + boost_) * delta_time;
    }

    void Camera::MoveUp(GLfloat delta_time)
    {
        position_ += up_ * (speed_ + boost_) * delta_time;
    }

    void Camera::UpdateViewportPosition(GLfloat x, GLfloat y)
    {
        viewport_.SetViewportPos(x, y);
    }

    void Camera::UpdateViewport(Viewport viewport)
    {
        viewport_ = viewport;
    }

    void Camera::UpdateViewport(GLfloat x, GLfloat y, GLfloat width, GLfloat height)
    {
        viewport_.SetViewportPos(x, y);
        viewport_.SetSize(width, height);
    }

    void Camera::ResetMouseTracking()
    {
        first_mouse_ = true;
    }

    void Camera::UpdateViewportSize(GLint width, GLint height)
    {
        viewport_.SetWindowSize(width, height);
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

    void Camera::ApplyViewport()
    {
        viewport_.Apply();
    }

    glm::vec3 Camera::GetPosition() const
    {
        return position_;
    }

    GLfloat Camera::GetFOV() const
    {
        return fov_;
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

    void Camera::SetBoost(GLfloat boost)
    {
        boost_ = boost;
    }

    void Camera::SetPosition(const glm::vec3 &position)
    {
        position_ = position;
    }

    void Camera::SetYawPitch(GLfloat yaw, GLfloat pitch)
    {
        yaw_ = yaw;
        pitch_ = pitch;
        UpdateFront();
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
