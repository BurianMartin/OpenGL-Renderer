#pragma once
#include "Utils.hpp"
#include "Core/Layer.hpp"
#include "Core/Camera.hpp"
#include "Core/InputEvents.hpp"
#include "Core/ResourceManager.hpp"

#include <ranges>
#include <vector>

namespace Core
{
    enum class Background_Type
    {
        Solid,
        Skybox,
        Skydome,
        None
    };
    class Scene
    {
    private:
        void DrawSolidBackground()
        {
            glClearColor(backgroundColor_.r, backgroundColor_.g, backgroundColor_.b, backgroundColor_.a);
            glClear(GL_COLOR_BUFFER_BIT); // Has to be called to fill the screen with the color
        }

        void DrawSkyboxBackground()
        {
            // TODO: Finish after adding textures to models
            glClear(GL_DEPTH_BUFFER_BIT); // Backround drawn, clear depth buffer to draw everything over it
        }

        void DrawSkydomeBackground()
        {
            // TODO: Finish after adding textures to models
            glClear(GL_DEPTH_BUFFER_BIT); // Backround drawn, clear depth buffer to draw everything over it
        }

        void UpdateRenderContext()
        {
            assert(!cameras_.empty());
            rctx_->camera_position_ = cameras_[active_camera_].GetPosition();
            rctx_->view_ = cameras_[active_camera_].GetViewMatrix();
            rctx_->projection_ = cameras_[active_camera_].GetProjectionMatrix();
        }

    protected:
        glm::vec4 backgroundColor_ = glm::vec4(0);
        Background_Type backgroundType_ = Background_Type::None;
        std::vector<std::shared_ptr<Layer>> layers_;
        std::vector<Camera> cameras_;
        GLint active_camera_ = 0; // index in the cameras vector

        std::shared_ptr<RenderContext> rctx_;
        std::shared_ptr<ResourceManager> rmanager_;

    public:
        Scene() = default;
        virtual ~Scene() = default;

        virtual void OnEvent(Event &event) = 0;

        virtual void OnMouseCapture() = 0;

        virtual void OnUpdate(float delta_time) = 0;

        virtual void OnSceneBoot() = 0;

        void OnLoad(std::shared_ptr<ResourceManager> rmanager, std::shared_ptr<RenderContext> rctx)
        {
            rmanager_ = rmanager;
            rctx_ = rctx;
            OnSceneBoot();
        }

        virtual void Update(float delta_time) final
        {
            OnUpdate(delta_time);
            for (auto layer : layers_)
            {
                layer->OnUpdate();
            }
        }

        virtual void AddLayer(std::shared_ptr<Layer> layer) final
        {
            layers_.push_back(layer);
        }

        virtual void Destroy() final
        {
            for (auto &layer : layers_)
            {
                layer->Destroy();
            }
        }

        virtual void DrawBackground() final
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

        virtual void SetBackgroundColor(glm::vec4 color) final
        {
            backgroundColor_ = color;
            backgroundType_ = Background_Type::Solid;
        }

        virtual void Render() final
        {
            UpdateRenderContext();
            for (const auto &layer : layers_)
            {
                layer->OnRender(rctx_); // Layer ordering matters !!
            }
        }
    };
}