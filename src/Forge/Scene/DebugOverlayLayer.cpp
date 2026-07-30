#include "Forge/Scene/DebugOverlayLayer.hpp"
#include "Forge/Rendering/Material.hpp"

namespace Forge
{
    namespace
    {
        /// A 1x1x1 wireframe cube centered on the origin (corners at +-0.5), so that
        /// SetPosition(center)/SetScale(size) maps it exactly onto any target AABB with no
        /// rotation involved — see DebugOverlayLayer::OnUpdate(). 12 edges, GL_LINES (each
        /// consecutive pair of indices is one independent segment, not a connected loop).
        std::shared_ptr<Mesh> BuildUnitWireBoxMesh()
        {
            std::vector<Vertex> vertices = {
                {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // 0
                {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},  // 1
                {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},   // 2
                {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},  // 3
                {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},  // 4
                {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},   // 5
                {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},    // 6
                {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},   // 7
            };
            std::vector<unsigned int> indices = {
                0,
                1,
                1,
                2,
                2,
                3,
                3,
                0, // near face
                4,
                5,
                5,
                6,
                6,
                7,
                7,
                4, // far face
                0,
                4,
                1,
                5,
                2,
                6,
                3,
                7, // connecting edges
            };
            return Mesh::Create("DebugAABBBox", vertices, indices, GL_LINES);
        }
    }

    DebugOverlayLayer::DebugOverlayLayer(std::string name, std::shared_ptr<ResourceManager> resourceManager,
                                         std::vector<std::shared_ptr<Layer>> watchedLayers)
        : Layer(std::move(name))
    {
        auto shader = resourceManager->LoadShader("shaders/vertex.glsl", "shaders/debug_overlay_fragment.glsl", "DebugOverlay");
        if (!shader)
            debug_error("Failed to load debug overlay shader");

        auto material = Material::Create(shader, "DebugOverlayBox");
        material->SetColor(glm::vec4(0.1f, 1.0f, 0.2f, 1.0f)); // bright green — reads clearly against every station's material
        materials_.push_back(material);

        auto boxMesh = BuildUnitWireBoxMesh();
        if (!boxMesh)
            debug_error("Failed to build the debug overlay's wireframe box mesh");

        for (const auto &layer : watchedLayers)
        {
            for (const auto &model : layer->GetAllModels())
            {
                auto box = std::make_shared<Model>(boxMesh);
                materialModels_[material].push_back(box);
                tracked_.emplace_back(model, box);
            }
        }
    }

    bool DebugOverlayLayer::OnEvent(Forge::Event &e, std::shared_ptr<Forge::FrameContext> ctx)
    {
        return true;
    }

    void DebugOverlayLayer::OnUpdate()
    {
        for (auto &[source, box] : tracked_)
        {
            Forge::AABB bounds = source->GetWorldBounds();
            box->SetPosition((bounds.min + bounds.max) * 0.5f);
            box->SetScale(bounds.max - bounds.min);
        }
    }

    void DebugOverlayLayer::OnRender(std::shared_ptr<Forge::FrameContext> ctx) const
    {
    }
} // namespace Forge
