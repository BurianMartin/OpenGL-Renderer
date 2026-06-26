#pragma once

#include "Utils.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <unordered_map>

enum class CamMove
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    FORWARD,
    BACKWARD
};

namespace Core
{

    inline const std::unordered_map<int, CamMove> key_map = {
        {GLFW_KEY_W, CamMove::FORWARD},
        {GLFW_KEY_S, CamMove::BACKWARD},
        {GLFW_KEY_A, CamMove::LEFT},
        {GLFW_KEY_D, CamMove::RIGHT},
        {GLFW_KEY_SPACE, CamMove::UP},
        {GLFW_KEY_LEFT_SHIFT, CamMove::DOWN}};

    class Camera
    {
    public:
        Camera(GLfloat aspect_ratio);

        glm::mat4 GetViewMatrix() const;
        glm::mat4 GetProjectionMatrix() const;

        // Mouse and scroll
        void ProcessMousePosition(float xpos, float ypos);
        void Rotate(GLfloat x_offset, GLfloat y_offset);
        void Zoom(GLfloat y_offset);

        glm::vec3 GetPosition() const;

        void Update(GLfloat delta_time);

        void CameraMove(CamMove direction, bool state);

        void UpdateAspectRatio(float aspect_ratio);

        void ResetMouseTracking();

    private:
        bool move_up_ = false;
        bool move_down_ = false;
        bool move_left_ = false;
        bool move_right_ = false;
        bool move_forward_ = false;
        bool move_backward_ = false;

        float last_x_ = 0.0f;
        float last_y_ = 0.0f;
        bool first_mouse_ = true;

        glm::vec3 position_;
        glm::vec3 front_;
        glm::vec3 up_;

        GLfloat yaw_;
        GLfloat pitch_;
        GLfloat fov_;

        GLfloat speed_;
        GLfloat sensitivity_;

        GLfloat aspect_ratio_;
        GLfloat near_plane_;
        GLfloat far_plane_;

        void UpdateFront(); // rebuilds front vector from yaw and pitch

        void MoveForward(GLfloat delta_time);
        void MoveBackward(GLfloat delta_time);
        void MoveLeft(GLfloat delta_time);
        void MoveRight(GLfloat delta_time);

        void MoveDown(GLfloat delta_time);
        void MoveUp(GLfloat delta_time);
    };
}