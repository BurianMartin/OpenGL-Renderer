#include "App/TestLayer.hpp"

namespace Solitare
{
    TestLayer::TestLayer()
    {
        /*std::vector<GLfloat> vertices = {
            -0.5f, 0.5f, 0.0f,  // Bottom-left
            -0.5f, -0.5f, 0.0f, // Top-left
            0.5f, 0.5f, 0.0f,   // Bottom-right
            -0.5f, -0.5f, 0.0f, // Top-left
            0.5f, 0.5f, 0.0f,   // Bottom-right
            0.5f, -0.5f, 0.0f}; // Top-right*/

        std::vector<GLfloat> vertices = {
            -0.1682f, 0.2356f, 0.0f,
            0.2398f, 0.0343f, 0.0f,
            -0.1682f, -0.1672f, 0.0f,
            -0.5514f, 0.3601f, 0.0f,
            -0.2337f, 0.6859f, 0.0f,
            -0.7883f, 0.0343f, 0.0f,
            -1.0000f, 0.4370f, 0.0f,
            -0.5514f, -0.2916f, 0.0f,
            -1.0000f, -0.3685f, 0.0f,
            -0.2337f, -0.6173f, 0.0f,

            -0.1682f, -0.1672f, 0.3426f,
            0.2398f, 0.0343f, 0.3426f,
            -0.1682f, 0.2356f, 0.3426f,
            -0.2337f, 0.6859f, 0.3426f,
            -0.5514f, 0.3601f, 0.3426f,
            -1.0000f, 0.4370f, 0.3426f,
            -0.7883f, 0.0343f, 0.3426f,
            -1.0000f, -0.3685f, 0.3426f,
            -0.5514f, -0.2916f, 0.3426f,
            -0.2337f, -0.6173f, 0.3426f};

        std::vector<GLuint> indices = {
            0, 1, 2,
            3, 4, 0,
            5, 6, 3,
            7, 8, 5,
            2, 9, 7,
            3, 0, 2,
            7, 5, 3,
            3, 2, 7,

            10, 11, 12,
            12, 13, 14,
            14, 15, 16,
            16, 17, 18,
            18, 19, 10,
            10, 12, 14,
            14, 16, 18,
            18, 10, 14,

            1, 0, 11,
            0, 12, 11,
            0, 4, 12,
            4, 13, 12,
            4, 3, 13,
            3, 14, 13,
            3, 6, 14,
            6, 15, 14,
            6, 5, 15,
            5, 16, 15,
            5, 8, 16,
            8, 17, 16,
            8, 7, 17,
            7, 18, 17,
            7, 9, 18,
            9, 19, 18,
            9, 2, 19,
            2, 10, 19,
            2, 1, 10,
            1, 11, 10};

        auto mesh = std::make_shared<Core::Mesh>("TestMesh", vertices, indices);
        auto model = std::make_shared<Core::Model>(mesh);

        auto model2 = std::make_shared<Core::Model>(mesh);

        auto solidColorShader = std::make_shared<Core::Shader>(
            "shaders/vertex.glsl",
            "shaders/solid_color.glsl",
            "Solid");

        shaders_.push_back(solidColorShader);
        glUseProgram(solidColorShader->ID);

        solidColorShader->SetVec4("triangle_color", Color_A1::Magenta);

        shaderModels_[solidColorShader].push_back(model);
        shaderModels_[solidColorShader].push_back(model2);
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
                    auto shader = *std::ranges::find(shaders_, "Solid", &Core::Shader::Tag_); // Get shader by name/tag
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
                break;
            }
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
