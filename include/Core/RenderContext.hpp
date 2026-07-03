#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>

namespace Core
{
    /**
     * @brief Per-frame data handed to every Layer::OnRender call.
     *
     * Owned by Renderer, refreshed once per frame by
     * Scene::UpdateRenderContext() from the scene's active Camera, and
     * passed down as a `shared_ptr` so Layers/Materials can read from it
     * without the Scene needing to expose its Camera list directly.
     * Material::Bind is the main consumer — it uploads `view_`,
     * `projection_`, `camera_position_`, and `time_` as shader uniforms
     * every time a material is bound.
     */
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