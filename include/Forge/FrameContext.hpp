#pragma once

#include "Forge/Light.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>

namespace Forge
{
    /**
     * @brief Per-frame data handed to every Layer::OnRender call.
     *
     * Owned by Renderer, refreshed once per frame by
     * Scene::UpdateFrameContext() from the scene's active Camera, and
     * passed down as a `shared_ptr` so Layers/Materials can read from it
     * without the Scene needing to expose its Camera list directly.
     * Material::Bind is the main consumer — it uploads `view_`,
     * `projection_`, `camera_position_`, and `time_` as shader uniforms
     * every time a material is bound.
     */
    struct FrameContext // Used to pass context and necessary data to layes on render
    {
        /// Current active camera's view matrix, refreshed once per frame by Scene::UpdateFrameContext().
        glm::mat4 view_;
        /// Current active camera's projection matrix, refreshed once per frame by Scene::UpdateFrameContext().
        glm::mat4 projection_;
        glm::vec3 camera_position_; // needed for lighting
        /// Seconds elapsed since the previous frame.
        GLfloat delta_time_;
        GLfloat time_ = 0.0f; // for animated shaders
        GLint window_width_;
        GLint window_height_;

        /// GL buffer object ID for the lights UBO; 0 until CreateLightBuffer() has been called.
        GLuint lightBufferId_ = 0;

        /// Fixed capacity of the lights UBO — must match the array size declared in the lighting shader.
        const GLint MAX_LIGHTS = 32;
        // bool multicam_mode_ = false;

        // later:
        // std::vector<LightData> lights;
        // ShadowMap* shadow_map;
        // FogSettings fog;

        /// Allocates the lights UBO (sized for a header vec4 plus `MAX_LIGHTS` GPULight entries) and binds it to uniform binding point 0. Call once during scene setup, before any light data is uploaded.
        void CreateLightBuffer()
        {
            glGenBuffers(1, &lightBufferId_);
            glBindBuffer(GL_UNIFORM_BUFFER, lightBufferId_);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec4) + MAX_LIGHTS * sizeof(GPULight), nullptr, GL_DYNAMIC_DRAW);

            GLint lightBindPoint = 0;
            glBindBufferBase(GL_UNIFORM_BUFFER, lightBindPoint, lightBufferId_);
        }

        /// Binds the lights UBO as the current `GL_UNIFORM_BUFFER`, ahead of a `glBufferSubData` upload.
        void BindLightBuffer()
        {
            glBindBuffer(GL_UNIFORM_BUFFER, lightBufferId_);
        }
    };
} // namespace Forge