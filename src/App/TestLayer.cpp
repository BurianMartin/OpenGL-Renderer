#include "App/TestLayer.hpp"
#include "TestLayer.hpp"

Solitare::TestLayer::TestLayer()
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

void Solitare::TestLayer::OnEvent()
{
}

void Solitare::TestLayer::OnUpdate()
{
}

void Solitare::TestLayer::Transition()
{
}

void Solitare::TestLayer::Suspend() const
{
}

void Solitare::TestLayer::Destroy()
{
}

void Solitare::TestLayer::OnRender() const
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