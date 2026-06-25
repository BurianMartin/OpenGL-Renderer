#include <glad/gl.h>

#include "App/TestLayer.hpp"
#include "Core/Scene.hpp"

namespace Test
{
    class TestScene : public Core::Scene
    {
    private:
    public:
        TestScene(std::shared_ptr<Core::ResourceManager> rmanager) : Core::Scene(rmanager)
        {
            auto testLayer = std::make_shared<Solitare::TestLayer>(rmanager_);

            AddLayer(testLayer);

            SetBackgroundColor(Solitare::Color_A1::Lime);
        }
        ~TestScene() = default;
    };
} // namespace Test
