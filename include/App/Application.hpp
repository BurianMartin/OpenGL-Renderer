#pragma once
#include "Utils.hpp"
#include "Scene.hpp"
#include "Renderer.hpp"
#include "App/TestLayer.hpp"
#include "Colors.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

namespace Solitare
{
    class Application
    {
    private:
        Core::Renderer renderer;
        GLFWwindow *window;

        std::vector<std::shared_ptr<Core::Scene>> scenes_;

        GLint current_scene_ = 0;

        void Init();

    public:
        Application();
        ~Application();

        void Run();

        void Destroy();

        void AddScene(std::shared_ptr<Core::Scene> scene); // Returns the scene ID in the vector of scenes
    };

} // namespace Solitare
