#pragma once

#include "Utils.hpp"

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera
{
public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;

    GLfloat fov;
    GLfloat far;
    GLfloat near;
    GLfloat aspect_ratio;

    GLfloat speed;
    GLfloat sensitivity;

    GLfloat yaw;
    GLfloat pitch;

public:
    Camera();
    ~Camera() = default;
    glm::mat4 GetViewMatrix();
    glm::mat4 GetProjectionMatrix();
};