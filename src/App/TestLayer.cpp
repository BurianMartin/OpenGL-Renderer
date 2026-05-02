#include "App/TestLayer.hpp"
#include "TestLayer.hpp"

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
            "shaders/solid_color.glsl");

        glUseProgram(solidColorShader->ID);

        solidColorShader->SetVec4("triangle_color", Color_A1::Magenta);

        shaderModels_[solidColorShader].push_back(model);
    }

    void TestLayer::OnEvent(/*const Core::Event& e*/)
    {
        /*
        TODO: Implement the logic for the event system

        switch(e.GetType())
        {
            case Core::EventType::KeyPressed:
                // Handle key pressed event like:
                OnKeyPressed(event);
                break;

            case Core::EventType::MouseButtonPressed:
                // Handle mouse button pressed event like:
                OnMouseButtonPressed(event);
                break;

            ... More event types, maybe even handle mouse movement here ?

            default:
                break;
        }
        */
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
