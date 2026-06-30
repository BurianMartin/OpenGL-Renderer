#include "App/TestLayer.hpp"

namespace Test
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

        auto material = Core::Material::Create(solidColorShader, "Solid");

        materialModels_[material].push_back(model);
        materials_.push_back(material);

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
                    auto material_find = std::ranges::find_if(materials_, [](const std::shared_ptr<Core::Material> &m)
                                                              { return m->GetTag() == "Solid"; });
                    if (material_find == materials_.end())
                    {
                        debug_info("Failed to find Solid material");
                        return true;
                    }

                    auto material = *material_find;
                    material->GetShader()->Use();
                    material->GetShader()->SetVec4("triangle_color", Core::Color_A1::RandomColor());
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

    void TestLayer::Destroy()
    {
    }

    void TestLayer::OnRender(std::shared_ptr<Core::RenderContext> ctx) const
    {
        for (const auto &[material, models] : materialModels_)
        {
            material->Bind(ctx);

            for (const auto &model : models)
            {
                material->GetShader()->SetMat4("uModel", model->getModelMatrix());
                model->Draw();
            }
        }
    }
} // namespace Test
