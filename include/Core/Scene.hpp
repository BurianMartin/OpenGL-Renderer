#pragma once
#include "Utils.hpp"
#include "Core/Layer.hpp"
#include "Core/Camera.hpp"
#include "Core/InputEvents.hpp"
#include "Core/ResourceManager.hpp"

#include <ranges>
#include <vector>
#include <unordered_map>

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
    protected:
        glm::vec4 backgroundColor_ = glm::vec4(0);
        Background_Type backgroundType_ = Background_Type::None;
        std::vector<std::shared_ptr<Layer>> layers_;
        std::vector<Camera> cameras_;
        GLint active_camera_ = 0; // index in the cameras vector

        std::shared_ptr<RenderContext> rctx_;
        std::shared_ptr<ResourceManager> rmanager_;

        void DrawSolidBackground();
        void DrawSkyboxBackground();
        void DrawSkydomeBackground();

    public:
        Scene(std::shared_ptr<ResourceManager> rmanager);
        ~Scene() = default;

        void OnLoad(std::shared_ptr<RenderContext> rctx);

        void OnMouseCapture();

        void Update(GLfloat delta_time);

        void AddLayer(std::shared_ptr<Layer> layer);

        const std::vector<std::shared_ptr<Layer>> &GetLayers() const;

        void Destroy();

        void DrawBackground();

        void SetBackgroundColor(glm::vec4 color);

        void HandleEvent(Event &event);

        void UpdateRenderContext();
    };
}