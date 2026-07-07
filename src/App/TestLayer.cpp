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

        auto GoldCube1 = std::make_shared<Core::Model>(testMesh);
        auto GoldCube2 = std::make_shared<Core::Model>(testMesh);

        // The OBJ is in raw Tinkercad units: bounding box center ≈ (-11.09, 1.0, 5.0),
        // width ≈ 36 units. Scale to 0.05 first, then translate to bring the scaled
        // center to the desired world position.;
        // Centering offset = -scale * OBJ_center = -0.05 * (-11.09, 1.0, 5.0) = (0.55, -0.05, -0.25)
        GoldCube1->SetScale(0.025f);
        GoldCube1->SetPosition(glm::vec3(-1.0f, -0.05f, 1.25f)); // cube centered at world (0, 0, 0)

        GoldCube2->SetScale(0.025f);
        GoldCube2->SetPosition(glm::vec3(1.0f, -0.05f, 0.25f)); // sun centered at world (0, 0, 0)

        // A few more cubes scattered around, each with a different preset material,
        // so there's an actual scene to test the multi-light shader loop against.
        auto silverCube = std::make_shared<Core::Model>(testMesh);
        auto rubyCube = std::make_shared<Core::Model>(testMesh);
        auto emeraldCube = std::make_shared<Core::Model>(testMesh);

        silverCube->SetScale(0.025f);
        silverCube->SetPosition(glm::vec3(-2.2f, 0.05f, -1.0f));

        rubyCube->SetScale(0.025f);
        rubyCube->SetPosition(glm::vec3(2.2f, -0.05f, -1.6f));

        emeraldCube->SetScale(0.025f);
        emeraldCube->SetPosition(glm::vec3(0.3f, 0.9f, -2.2f));

        auto solidColorShader = resourceManager->LoadShader("shaders/vertex.glsl", "shaders/fragment.glsl", "Uber");
        if (!solidColorShader)
            debug_error("Failed to create Solid shader");

        /*
        auto material1 = Core::Material::Create(solidColorShader, "Changing");
        auto material2 = Core::Material::Create(solidColorShader, "Solid");
        */

        auto crateMesh = resourceManager->LoadMesh("models/crate.obj");
        auto crateMat = Core::Material::Create(solidColorShader, "Crate");
        crateMat->SetTexture(resourceManager->LoadTexture("textures/crate_diffuse.png"));
        crateMat->SetSpecularTexture(resourceManager->LoadTexture("textures/crate_specular.png"));

        auto crateModel = std::make_shared<Core::Model>(crateMesh);
        materialModels_[crateMat].push_back(crateModel);
        materials_.push_back(crateMat);

        crateModel->SetScale(0.8);
        crateModel->SetPosition(glm::vec3(2.2f, 0.05f, -1.0f));

        auto material1 = Core::Material::Gold(solidColorShader);
        auto material2 = Core::Material::Gold(solidColorShader);
        auto material3 = Core::Material::Silver(solidColorShader);
        auto material4 = Core::Material::Ruby(solidColorShader);
        auto material5 = Core::Material::Emerald(solidColorShader);

        materialModels_[material2].push_back(GoldCube1);
        materialModels_[material1].push_back(GoldCube2);
        materialModels_[material3].push_back(silverCube);
        materialModels_[material4].push_back(rubyCube);
        materialModels_[material5].push_back(emeraldCube);
        materials_.push_back(material1);
        materials_.push_back(material2);
        materials_.push_back(material3);
        materials_.push_back(material4);
        materials_.push_back(material5);

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
                materials_[1]->SetColor(Core::Color_A1::RandomColor());
                return true;
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
                glm::mat4 modelMatrix = model->GetModelMatrix();
                glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(modelMatrix)));

                material->GetShader()->SetMat4("uModel", modelMatrix);
                material->GetShader()->SetMat3("uNormalMatrix", normalMatrix);

                model->Draw();
            }
        }
    }
} // namespace Test
