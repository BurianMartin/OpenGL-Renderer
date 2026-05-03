#include "App/TestLayer.hpp"

namespace Solitare
{
    TestLayer::TestLayer()
    {
        float vertices[] = {
            0.0f, 0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f};

        std::vector<Core::Vertex> vertexData;

        for (int i = 0; i < 9; i += 3)
        {
            Core::Vertex v;
            v.position = {vertices[i], vertices[i + 1], vertices[i + 2]};

            v.normal = {0.0f, 0.0f, 1.0f};
            v.texCoords = {0.0f, 0.0f};

            vertexData.push_back(v);
        }

        auto mesh = std::make_shared<Core::Mesh>(vertexData, std::vector<unsigned int>{});
        auto model = std::make_shared<Core::Model>(mesh);

        auto solidColorShader = std::make_shared<Core::Shader>(
            "shaders/vertex.glsl",
            "shaders/solid_color.glsl",
            "Solid");

        shaders_.push_back(solidColorShader);
        glUseProgram(solidColorShader->ID);

        solidColorShader->SetVec4("triangle_color", Color_A1::Magenta);

        shaderModels_[solidColorShader].push_back(model);
    }

    bool TestLayer::OnEvent(Core::Event &event)
    {

        // TODO: Implement the logic for event consumption
        switch (event.GetEventType())
        {
        case Core::EventType::KeyPressed:
        {
            Core::KeyPressedEvent ev = static_cast<Core::KeyPressedEvent &>(event);

            switch (ev.GetKeyCode())
            {
            case GLFW_KEY_TAB:
            {
                if (!ev.IsRepeat())
                {
                    debug_info("Change triangle color");
                    auto shader = *std::ranges::find(shaders_, "Solid", &Core::Shader::Tag_); // Get shader by name/tag
                    shader->SetVec4("triangle_color", Color_A1::RandomColor());
                    return true;
                }
                else
                {
                    debug_info("Tab key repeat detected");
                    return true;
                }
            }
            default:
                break;
            }
        }

        case Core::EventType::MouseButtonPressed:
            // Handle mouse button pressed event like:

            return true;
            // OnMouseButtonPressed(event);
            break;

            // ... More event types, maybe even handle mouse movement here ?

        default:
            break;
        }

        return true;
    }

    void TestLayer::OnUpdate()
    {
    }

    void TestLayer::Transition()
    {
    }

    void TestLayer::Suspend() const
    {
    }

    void TestLayer::Destroy()
    {
    }

    void TestLayer::OnRender() const
    {
        for (const auto &[shader, models] : shaderModels_)
        {
            shader->Use();
            for (const auto &model : models)
            {
                model->Draw();
            }
        }
    }
} // namespace Solitare
