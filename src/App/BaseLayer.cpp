#include "App/BaseLayer.hpp"

Solitare::BaseLayer::BaseLayer()
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

    shaderModels_[solidColorShader].push_back(model);
}

void Solitare::BaseLayer::OnEvent()
{
}
void Solitare::BaseLayer::OnUpdate()
{
}

void Solitare::BaseLayer::Transition()
{
}

void Solitare::BaseLayer::Suspend() const
{
}

void Solitare::BaseLayer::Destroy()
{
}

void Solitare::BaseLayer::OnRender() const
{
    debug_info("Rendering BaseLayer");

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    for (const auto &[shader, models] : shaderModels_)
    {
        shader->Use();
        for (const auto &model : models)
        {
            model->Draw();
            debug_info("Drawing a model");
        }
    }
}