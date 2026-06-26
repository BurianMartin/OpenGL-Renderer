#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>

namespace Core
{
    struct RenderContext // Used to pass context and necessary data to layes on render
    {
        glm::mat4 view_;
        glm::mat4 projection_;
        glm::vec3 camera_position_; // needed for lighting
        float delta_time_;
        float time_ = 0.0f; // for animated shaders

        GLfloat aspect_ratio_;
        // bool multicam_mode_ = false;

        // later:
        // std::vector<LightData> lights;
        // ShadowMap* shadow_map;
        // FogSettings fog;
    };
} // namespace Core