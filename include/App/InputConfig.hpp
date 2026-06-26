#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <unordered_map>

#include "Utils.hpp"

#include "Core/Camera.hpp"

inline const std::unordered_map<int, Core::CamMove> key_map = {
    {GLFW_KEY_W, Core::CamMove::FORWARD},
    {GLFW_KEY_S, Core::CamMove::BACKWARD},
    {GLFW_KEY_A, Core::CamMove::LEFT},
    {GLFW_KEY_D, Core::CamMove::RIGHT},
    {GLFW_KEY_SPACE, Core::CamMove::UP},
    {GLFW_KEY_LEFT_SHIFT, Core::CamMove::DOWN}};