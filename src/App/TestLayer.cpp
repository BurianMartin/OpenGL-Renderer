#include "App/TestLayer.hpp"
#include "Core/Material.hpp"
#include "Core/Light.hpp"
namespace Test
{
    TestLayer::TestLayer(std::shared_ptr<Core::ResourceManager> resourceManager)
    {
        auto testMesh = resourceManager->LoadMesh("models/cube.obj");

        if (!testMesh)
            debug_error("Failed to create TestMesh");

        auto model = std::make_shared<Core::Model>(testMesh);
        auto sun = std::make_shared<Core::Model>(testMesh);

        // The OBJ is in raw Tinkercad units: bounding box center ≈ (-11.09, 1.0, 5.0),
        // width ≈ 36 units. Scale to 0.05 first, then translate to bring the scaled
        // center to the desired world position.;
        // Centering offset = -scale * OBJ_center = -0.05 * (-11.09, 1.0, 5.0) = (0.55, -0.05, -0.25)
        model->SetScale(0.05f);
        model->SetPosition(glm::vec3(-1.0f, -0.05f, -0.25f)); // cube centered at world (0, 0, 0)

        sun->SetScale(0.05f);
        sun->SetPosition(glm::vec3(1.0f, -0.05f, -0.25f)); // sun centered at world (0, 0, 0)

        auto solidColorShader = resourceManager->LoadShader("shaders/vertex.glsl", "shaders/fragment.glsl", "Uber");
        if (!solidColorShader)
            debug_error("Failed to create Solid shader");

        /*
        auto material1 = Core::Material::Create(solidColorShader, "Changing");
        auto material2 = Core::Material::Create(solidColorShader, "Solid");
        */

        auto material1 = Core::Material::Gold(solidColorShader);
        auto material2 = Core::Material::Gold(solidColorShader);

        materialModels_[material2].push_back(model);
        materialModels_[material1].push_back(sun);
        materials_.push_back(material1);
        materials_.push_back(material2);

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
                    debug_info("Color change now!");
                    auto material_find = std::ranges::find_if(materials_, [](const std::shared_ptr<Core::Material> &m)
                                                              { return m->GetTag() == "Changing"; });
                    if (material_find == materials_.end())
                    {
                        debug_info("Failed to find Changing material");
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
