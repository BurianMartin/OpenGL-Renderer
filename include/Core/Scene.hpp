#pragma once
#include "Utils.hpp"
#include "Core/Layer.hpp"
#include "Core/Camera.hpp"
#include "Core/InputEvents.hpp"

#include <unordered_map>

#include <vector>
#include <ranges>

namespace Core
{

    static const std::unordered_map<int, CamMove> key_map = {
        {GLFW_KEY_W, CamMove::FORWARD},
        {GLFW_KEY_S, CamMove::BACKWARD},
        {GLFW_KEY_A, CamMove::LEFT},
        {GLFW_KEY_D, CamMove::RIGHT},
        {GLFW_KEY_SPACE, CamMove::UP},
        {GLFW_KEY_LEFT_SHIFT, CamMove::DOWN}};

    enum class Background_Type
    {
        Solid,
        Skybox,
        Skydome,
        None
    };

    class Scene
    {
        bool mouse_captured_ = false; // WSL mouse lock issue fix (Temporary, TODO: Remove fast, Fr, get this out)
    private:
        glm::vec4 backgroundColor_ = glm::vec4(0);
        Background_Type backgroundType_ = Background_Type::None;
        std::vector<std::shared_ptr<Layer>> layers_;
        std::vector<Camera> cameras_;
        GLint active_camera_ = 0; // index in the cameras vector

        void DrawSolidBackground();
        void DrawSkyboxBackground();
        void DrawSkydomeBackground();

    public:
        Scene(float aspect_ratio);
        ~Scene() = default;

        void Update(GLfloat delta_time);

        void AddLayer(std::shared_ptr<Layer> layer);

        const std::vector<std::shared_ptr<Layer>> &GetLayers() const;

        void Destroy();

        void DrawBackground();

        void SetBackgroundColor(glm::vec4 color);

        void HandleEvent(Event &event);

        void FillRenderContext(Core::RenderContext &ctx);
    };
}