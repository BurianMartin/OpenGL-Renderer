#include "App/BaseLayer.hpp"

void Solitare::BaseLayer::OnRender() const
{
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    for (const auto &[shader, models] : shaderModels)
    {
        shader->Use();
        for (const auto &model : models)
        {
            model.Draw();
        }
    }
}