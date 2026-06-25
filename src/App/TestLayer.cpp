#include "App/TestLayer.hpp"

namespace Solitare
{
    TestLayer::TestLayer(std::shared_ptr<Core::ResourceManager> resourceManager)
    {
        auto testMesh = resourceManager->LoadMesh("models/star.obj");

        if (!testMesh)
            debug_error("Failed to create TestMesh");

        auto model = std::make_shared<Core::Model>(testMesh);

        // The OBJ is in raw Tinkercad units: bounding box center ≈ (-11.09, 1.0, 5.0),
        // width ≈ 36 units. Scale to 0.05 first, then translate to bring the scaled
        // center to the desired world position.;
        // Centering offset = -scale * OBJ_center = -0.05 * (-11.09, 1.0, 5.0) = (0.55, -0.05, -0.25)
        model->SetScale(0.05f);
        model->SetPosition(glm::vec3(0.55f, -0.05f, -0.25f)); // star centered at world (0, 0, 0)

        auto solidColorShader = resourceManager->LoadShader("shaders/vertex.glsl", "shaders/solid_color.glsl", "Solid");
        if (!solidColorShader)
            debug_error("Failed to create Solid shader");

        shaders_.push_back(solidColorShader);
        solidColorShader->Use();
        solidColorShader->SetVec4("triangle_color", Color_A1::Magenta);

        shaderModels_[solidColorShader].push_back(model);

        srand(time(NULL));
    }

    bool TestLayer::OnEvent(Core::Event &event)
    {
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
                    auto shader_find = std::ranges::find(shaders_, "Solid", &Core::Shader::Tag_); // Get shader by name/tag
                    if (shader_find == shaders_.end())
                    {
                        debug_info("Failed to find Solid shader");
                        return true;
                    }

                    auto shader = *shader_find;
                    glUseProgram(shader->ID);
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
                return true;
                break;
            }
            break;
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

    void TestLayer::OnRender(std::shared_ptr<Core::RenderContext> ctx) const
    {
        for (const auto &[shader, models] : shaderModels_)
        {
            shader->Use();
            shader->SetMat4("uView", ctx->view_);
            shader->SetMat4("uProjection", ctx->projection_);

            for (const auto &model : models)
            {
                shader->SetMat4("uModel", model->getModelMatrix());
                model->Draw();
            }
        }
    }
} // namespace Solitare
