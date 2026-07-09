#include "Forge/Scene.hpp"

namespace Forge
{
    void Scene::DrawSolidBackground()
    {
        glClearColor(backgroundColor_.r, backgroundColor_.g, backgroundColor_.b, backgroundColor_.a);
        glClear(GL_COLOR_BUFFER_BIT); // Has to be called to fill the screen with the color
    }

    void Scene::DrawSkyboxBackground()
    {
        // TODO: Finish after adding textures to models
        glClear(GL_DEPTH_BUFFER_BIT); // Backround drawn, clear depth buffer to draw everything over it
    }

    void Scene::DrawSkydomeBackground()
    {
        // TODO: Finish after adding textures to models
        glClear(GL_DEPTH_BUFFER_BIT); // Backround drawn, clear depth buffer to draw everything over it
    }

    void Scene::UpdateFrameContext()
    {
        if (cameras_.empty())
        {
            debug_error("No active camera found");
            return;
        }
        rctx_->camera_position_ = cameras_[active_camera_].GetPosition();
        rctx_->view_ = cameras_[active_camera_].GetViewMatrix();
        rctx_->projection_ = cameras_[active_camera_].GetProjectionMatrix();
    }

    void Scene::LoadLights()
    {
        rctx_->BindLightBuffer();

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
        lights_.push_back(light);
    }

    void Scene::OnLoad(std::shared_ptr<ResourceManager> rmanager, std::shared_ptr<FrameContext> rctx)
    {
        rmanager_ = rmanager;
        rctx_ = rctx;
        OnSceneBoot();
    }

    void Scene::Update(float delta_time)
    {
        OnUpdate(delta_time);
        for (auto layer : layers_)
        {
            layer->OnUpdate();
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
        UpdateFrameContext();
        LoadLights();
        for (const auto &layer : layers_)
        {
            layer->Render(rctx_); // Layer ordering matters !!
        }
    }
} // namespace Forge
