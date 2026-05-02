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

    bool TestLayer::OnEvent(Core::Event &e)
    {

        // TODO: Implement the logic for event consumption
        switch (e.GetEventType())
        {
        case Core::EventType::KeyPressed:
        {
            // TEST CODE:
            // Handle key pressed event like:
            debug_info("Change triangle color");
            auto it = std::ranges::find(shaders_, "Solid", &Core::Shader::Tag_);
            auto shader = *it;
            shader->SetVec4("triangle_color", Color_A1::Red);
            return true;
            // OnKeyPressed(event);
            break;
            // END TEST CODE, TODO: REMOVE LATER
        }

        case Core::EventType::MouseButtonPressed:
            // Handle mouse button pressed event like:

            debug_info("Mouse Button Event consumed by TestLayer");
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
