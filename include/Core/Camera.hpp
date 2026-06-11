#pragma once

#include "Utils.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

namespace Core
{

    enum CamMove
    {
        UP,
        DOWN,
        LEFT,
        RIGHT,
        FORWARD,
        BACKWARD
    };

    class Camera
    {
    public:
        Camera(GLfloat aspect_ratio);

        glm::mat4 GetViewMatrix();
        glm::mat4 GetProjectionMatrix();

        // Mouse and scroll
        void ProcessMousePosition(float xpos, float ypos);
        void Rotate(GLfloat x_offset, GLfloat y_offset);
        void Zoom(GLfloat y_offset);

        glm::vec3 GetPosition() const;

        void Update(GLfloat delta_time);

        void CameraMove(CamMove direction, bool state);

    private:
        bool move_up = false;
        bool move_down = false;
        bool move_left = false;
        bool move_right = false;
        bool move_forward = false;
        bool move_backward = false;

        float last_x_ = 0.0f;
        float last_y_ = 0.0f;
        bool first_mouse_ = true;

        glm::vec3 position;
        glm::vec3 front;
        glm::vec3 up;

        GLfloat yaw;
        GLfloat pitch;
        GLfloat fov;

        GLfloat speed;
        GLfloat sensitivity;

        GLfloat aspect_ratio;
        GLfloat near_plane;
        GLfloat far_plane;

        void UpdateFront(); // rebuilds front vector from yaw and pitch

        void MoveForward(GLfloat delta_time);
        void MoveBackward(GLfloat delta_time);
        void MoveLeft(GLfloat delta_time);
        void MoveRight(GLfloat delta_time);

        void MoveDown(GLfloat delta_time);
        void MoveUp(GLfloat delta_time);
    };
}