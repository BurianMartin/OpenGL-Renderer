#include "Demo/LightDemoLayer.hpp"
#include "Forge/Material.hpp"
#include "Forge/Light.hpp"

#include <glm/gtc/constants.hpp>
namespace Demo
{
    namespace
    {
        /// A large flat quad in the XZ plane, normal pointing up — floor/ground for the demo.
        /// Built directly from Vertex data (skipping the OBJ path entirely) since it's just
        /// four corners with a known normal, the same "hardcoded vertex array" pattern
        /// Scene::DrawSkyboxBackground uses for its own unit cube.
        std::shared_ptr<Forge::Mesh> BuildFloorMesh()
        {
            std::vector<Forge::Vertex> vertices = {
                {{-6.0f, -1.2f, 4.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
                {{6.0f, -1.2f, 4.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
                {{6.0f, -1.2f, -14.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
                {{-6.0f, -1.2f, -14.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            };
            std::vector<unsigned int> indices = {0, 1, 2, 0, 2, 3};
            return Forge::Mesh::Create("Floor", vertices, indices);
        }
    }

    LightDemoLayer::LightDemoLayer(std::shared_ptr<Forge::ResourceManager> resourceManager)
    {
        auto solidColorShader = resourceManager->LoadShader("shaders/vertex.glsl", "shaders/fragment.glsl", "Uber");
        if (!solidColorShader)
            debug_error("Failed to create Solid shader");

        // --- Floor — grounds the scene and gives the directional light + point-light falloff
        // something to actually read against, instead of objects floating in a void. ---
        auto floorMesh = BuildFloorMesh();
        auto floorMat = Forge::Material::Create(solidColorShader, "Floor",
                                                glm::vec3(0.05f, 0.05f, 0.06f), glm::vec3(0.35f, 0.35f, 0.38f),
                                                glm::vec3(0.15f, 0.15f, 0.15f), 16.0f);
        auto floorModel = std::make_shared<Forge::Model>(floorMesh);
        materialModels_[floorMat].push_back(floorModel);
        materials_.push_back(floorMat);

        // --- Four "light stations", one cube each, spaced so each one's dedicated light
        // (added in LightDemoScene::OnSceneBoot) dominates it — see that file for why these
        // positions/lights are paired up. ---
        auto cubeMesh = resourceManager->LoadMesh("models/cube.obj");
        if (!cubeMesh)
            debug_error("Failed to load models/cube.obj");

        auto goldCube = std::make_shared<Forge::Model>(cubeMesh);
        goldCube->SetScale(0.025f);
        goldCube->SetPosition(glm::vec3(-2.5f, -0.05f, -1.0f)); // Station 1 — point light (warm key)

        auto silverCube = std::make_shared<Forge::Model>(cubeMesh);
        silverCube->SetScale(0.025f);
        silverCube->SetPosition(glm::vec3(2.5f, 0.05f, -3.5f)); // Station 2 — point light (cool rim)

        auto rubyCube = std::make_shared<Forge::Model>(cubeMesh);
        rubyCube->SetScale(0.025f);
        glm::vec3 rubyCubePos(-2.5f, -0.05f, -6.0f); // Station 3 — point light (warm rim)
        rubyCube->SetPosition(rubyCubePos);

        auto emeraldCube = std::make_shared<Forge::Model>(cubeMesh);
        emeraldCube->SetScale(0.025f);
        glm::vec3 emeraldCubePos(2.5f, -0.05f, -8.5f); // Station 4 — spot light, aimed straight down
        emeraldCube->SetPosition(emeraldCubePos);

        // --- Fifth station: the textured crate — demonstrates diffuse+specular texture
        // mapping specifically, as opposed to the flat Blinn-Phong presets above. ---
        auto crateMesh = resourceManager->LoadMesh("models/crate.obj");
        auto crateMat = Forge::Material::Create(solidColorShader, "Crate");
        crateMat->SetTexture(resourceManager->LoadTexture("textures/crate_diffuse.png"));
        crateMat->SetSpecularTexture(resourceManager->LoadTexture("textures/crate_specular.png"));

        auto crateModel = std::make_shared<Forge::Model>(crateMesh);
        crateModel->SetScale(0.8f);
        crateModel->SetPosition(glm::vec3(0.0f, 0.05f, -11.0f)); // Station 5 — lit only by the directional "sun" + ambient spill

        auto goldMat = Forge::Material::Gold(solidColorShader);
        auto silverMat = Forge::Material::Silver(solidColorShader);
        auto rubyMat = Forge::Material::Ruby(solidColorShader);
        auto emeraldMat = Forge::Material::Emerald(solidColorShader);

        materialModels_[goldMat].push_back(goldCube);
        materialModels_[silverMat].push_back(silverCube);
        materialModels_[rubyMat].push_back(rubyCube);
        materialModels_[emeraldMat].push_back(emeraldCube);
        // goldMat pushed right after floorMat so materials_[1] (the Q-handler's target below)
        // stays a flat-colored preset — SetColor() has zero visible effect on crateMat, since
        // fragment.glsl always prefers a bound texture over uBaseColor when uHasTexture is true.
        materials_.push_back(goldMat);
        materials_.push_back(silverMat);
        materials_.push_back(rubyMat);
        materials_.push_back(emeraldMat);

        materialModels_[crateMat].push_back(crateModel);
        materials_.push_back(crateMat);

        // --- Tween demo: the Ruby cube spins 360° forever. SetSpin builds the rotation directly
        // from a continuously growing angle instead of slerping between two fixed quaternions —
        // see SetSpin's comment for why a full-turn spin needs that. ---
        auto rubySpin = std::make_unique<Forge::Vec3Tween>(rubyCube, rubyCubePos, rubyCubePos, 3.0f, Forge::Vec3Tween::Ease::Linear);
        rubySpin->SetSpin(glm::vec3(0.0f, 1.0f, 0.0f), 2.0f * glm::pi<float>());
        rubySpin->SetRepeat(Forge::Vec3Tween::Repeat::Loop);
        AddTween(std::move(rubySpin));

        // --- Tween demo: the Emerald cube bobs up and down forever — PingPong reverses
        // direction each cycle (forth, back, forth, back...) instead of resetting like Loop.
        // Default (EaseOutQuad) easing here on purpose: it decelerates into both the top and
        // bottom of the bob, giving it a soft "hang and fall" feel rather than Ruby's constant-speed spin. ---
        auto emeraldBob = std::make_unique<Forge::Vec3Tween>(
            emeraldCube, emeraldCubePos, emeraldCubePos + glm::vec3(0.0f, 0.4f, 0.0f), 1.0f);
        emeraldBob->SetRepeat(Forge::Vec3Tween::Repeat::PingPong);
        AddTween(std::move(emeraldBob));

        srand(time(NULL));
    }

    bool LightDemoLayer::OnEvent(Forge::Event &event)
    {
        switch (event.GetEventType())
        {
        case Forge::EventType::KeyPressed:
        {
            Forge::KeyPressedEvent ev = static_cast<Forge::KeyPressedEvent &>(event);
            switch (ev.GetKeyCode())
            {
            case Forge::Key::Q:
            {
                materials_[1]->SetColor(Forge::Color_A1::RandomColor());
                return true;
            }
            default:
                return true;
                break;
            }
            break;
        }

        case Forge::EventType::MouseButtonPressed:
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

    void LightDemoLayer::OnUpdate()
    {
    }

    void LightDemoLayer::Destroy()
    {
    }

    void LightDemoLayer::OnRender(std::shared_ptr<Forge::FrameContext> ctx) const
    {
    }
} // namespace Demo
