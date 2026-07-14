#include "Forge/Scene.hpp"
#include "Scene.hpp"

namespace Forge
{
    Scene::~Scene()
    {
        if (skydomeVAO_ != 0)
            glDeleteVertexArrays(1, &skydomeVAO_);
    }

    void Scene::DrawSolidBackground()
    {
        glClearColor(backgroundColor_.r, backgroundColor_.g, backgroundColor_.b, backgroundColor_.a);
        glClear(GL_COLOR_BUFFER_BIT); // Has to be called to fill the screen with the color
    }

    void Scene::DrawSkyboxBackground()
    {
        if (!skyboxShader_)
        {
            skyboxShader_ = rmanager_->LoadShader("shaders/skybox_vertex.glsl", "shaders/skybox_fragment.glsl", "Skybox");

            // Unit cube, position-only, 36 non-indexed floats (12 triangles). Winding
            // direction doesn't matter here — face culling is disabled for this draw
            // below, and the shader never reads a normal anyway.
            static const std::vector<GLfloat> kSkyboxVertices = {
                -1.0f, 1.0f, -1.0f,
                -1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, 1.0f, -1.0f,
                -1.0f, 1.0f, -1.0f,

                -1.0f, -1.0f, 1.0f,
                -1.0f, -1.0f, -1.0f,
                -1.0f, 1.0f, -1.0f,
                -1.0f, 1.0f, -1.0f,
                -1.0f, 1.0f, 1.0f,
                -1.0f, -1.0f, 1.0f,

                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, 1.0f,
                1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,

                -1.0f, -1.0f, 1.0f,
                -1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 1.0f,
                -1.0f, -1.0f, 1.0f,

                -1.0f, 1.0f, -1.0f,
                1.0f, 1.0f, -1.0f,
                1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, 1.0f,
                -1.0f, 1.0f, 1.0f,
                -1.0f, 1.0f, -1.0f,

                -1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f, 1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f, 1.0f,
                1.0f, -1.0f, 1.0f};

            skyboxMesh_ = Mesh::Create("Skybox", kSkyboxVertices, std::vector<GLuint>{});
        }

        if (!skyboxShader_ || !skyboxMesh_)
        {
            debug_warn("Skybox resources unavailable — falling back to whatever the depth buffer already shows");
            return;
        }

        glDepthFunc(GL_LEQUAL); // the vertex shader pins depth to 1.0 (the far plane), matching the buffer's own clear value — LESS would reject the tie
        glDepthMask(GL_FALSE);  // never occludes anything
        glDisable(GL_CULL_FACE);

        skyboxShader_->Use();
        skyboxShader_->SetMat4("uView", glm::mat4(glm::mat3(fctx_->view_))); // rotation only — strip translation so the cube always surrounds the camera
        skyboxShader_->SetMat4("uProjection", fctx_->projection_);
        skyboxMesh_->draw();

        glEnable(GL_CULL_FACE);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
    }

    void Scene::DrawSkydomeBackground()
    {
        if (!skydomeShader_)
        {
            skydomeShader_ = rmanager_->LoadShader("shaders/skydome_vertex.glsl", "shaders/skydome_fragment.glsl", "Skydome");
            glGenVertexArrays(1, &skydomeVAO_);
        }

        if (!skydomeShader_)
        {
            debug_warn("Skydome shader unavailable — falling back to whatever the depth buffer already shows");
            return;
        }

        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);

        skydomeShader_->Use();
        glm::mat4 invViewProj = glm::inverse(fctx_->projection_ * fctx_->view_);
        skydomeShader_->SetMat4("uInvViewProj", invViewProj);

        glBindVertexArray(skydomeVAO_);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
    }

    void Scene::SetSkyboxBackground()
    {
        backgroundType_ = Background_Type::Skybox;
    }

    void Scene::SetSkydomeBackground()
    {
        backgroundType_ = Background_Type::Skydome;
    }

    void Scene::UpdateFrameContext(GLint camera_index)
    {
        if (cameras_.empty())
        {
            debug_error("No active camera found");
            return;
        }
        fctx_->camera_position_ = cameras_[camera_index].GetPosition();
        fctx_->view_ = cameras_[camera_index].GetViewMatrix();
        fctx_->projection_ = cameras_[camera_index].GetProjectionMatrix();
    }

    void Scene::LoadLights()
    {
        fctx_->BindLightBuffer();

        glm::vec4 countHeader(static_cast<float>(lights_.size()), 0.0f, 0.0f, 0.0f);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::vec4), &countHeader);

        std::vector<GPULight> gpuLights;
        gpuLights.reserve(lights_.size());
        for (const auto &light : lights_)
            gpuLights.push_back(light->ToGPULight());

        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::vec4),
                        gpuLights.size() * sizeof(GPULight), gpuLights.data());
    }

    void Scene::AddLight(std::shared_ptr<Light> light)
    {
        if (static_cast<GLint>(lights_.size()) >= fctx_->MAX_LIGHTS)
        {
            debug_warn("Scene::AddLight: at MAX_LIGHTS (" << fctx_->MAX_LIGHTS << "), dropping light");
            return;
        }
        lights_.push_back(light);
    }

    void Scene::OnLoad(std::shared_ptr<ResourceManager> rmanager, std::shared_ptr<FrameContext> rctx)
    {
        rmanager_ = rmanager;
        fctx_ = rctx;
        OnSceneBoot();
    }

    void Scene::Update(float delta_time)
    {
        OnUpdate(delta_time);
        for (auto layer : layers_)
        {
            layer->Update(delta_time);
        }
    }

    void Scene::AddLayer(std::shared_ptr<Layer> layer)
    {
        layers_.push_back(layer);
    }

    void Scene::Destroy()
    {
        for (auto &layer : layers_)
        {
            layer->Destroy();
        }
    }

    void Scene::ApplyViewport(GLint camera_index)
    {
        cameras_[camera_index].ApplyViewport();
    }

    void Scene::ResetMouse()
    {
        cameras_[active_camera_].ResetMouseTracking();
    }

    void Scene::DrawBackground()
    {
        switch (backgroundType_)
        {
        case Background_Type::Solid:
            DrawSolidBackground();
            break;

        case Background_Type::Skybox:
            DrawSkyboxBackground();
            break;

        case Background_Type::Skydome:
            DrawSkydomeBackground();
            break;

        case Background_Type::None:
            break; // intentional — caller owns the color buffer

        default:
            break;
        }
    }

    void Scene::SetBackgroundColor(glm::vec4 color)
    {
        backgroundColor_ = color;
        backgroundType_ = Background_Type::Solid;
    }

    void Scene::Render()
    {
        LoadLights();

        glEnable(GL_SCISSOR_TEST); // without this, glScissor's rect is set but never enforced — each camera's clear would hit the whole depth buffer
        for (size_t i = 0; i < cameras_.size(); ++i)
        {
            ApplyViewport(i);             // sets glViewport + glScissor from cameras_[i]'s own Viewport
            UpdateFrameContext(i);        // refreshes view_/projection_/camera_position_ from cameras_[i]
            glClear(GL_DEPTH_BUFFER_BIT); // scissored to this camera's rect
            DrawBackground();
            for (const auto &layer : layers_)
                layer->Render(fctx_);
        }
        glDisable(GL_SCISSOR_TEST);
    }
} // namespace Forge
