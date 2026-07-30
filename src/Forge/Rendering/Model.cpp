#include "Forge/Rendering/Model.hpp"
#include "Forge/Rendering/Material.hpp"

#include <limits>

namespace Forge
{
    Model::Model(std::shared_ptr<Mesh> mesh) : mesh_(mesh)
    {
        position_ = glm::vec3(0.0f, 0.0f, 0.0f);
        rotation_ = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f));
        scale_ = glm::vec3(1.0f, 1.0f, 1.0f); // scale of 0 makes it invisible
    }

    void Model::SetPosition(const glm::vec3 &pos) { position_ = pos; }
    void Model::SetScale(const glm::vec3 &scale) { scale_ = scale; }
    void Model::SetScale(float s) { scale_ = glm::vec3(s); }
    void Model::SetRotation(const glm::quat &rot) { rotation_ = rot; }
    void Model::SetRotation(float angle_radians, const glm::vec3 &axis)
    {
        rotation_ = glm::angleAxis(angle_radians, glm::normalize(axis));
    }

    void Model::Draw() const
    {
        mesh_->draw();
    }

    void Model::SetMaterial(std::shared_ptr<Material> material) { material_ = material; }
    std::shared_ptr<Material> Model::GetMaterial() const { return material_; }

    glm::mat4 Model::GetModelMatrix() const
    {
        glm::mat4 m = glm::mat4(1.0f);
        m = glm::translate(m, position_);
        m = m * glm::mat4_cast(rotation_);
        m = glm::scale(m, scale_);
        return m;
    }

    Forge::AABB Model::GetWorldBounds() const
    {
        glm::vec3 localMin = mesh_->GetLowerBounds();
        glm::vec3 localMax = mesh_->GetUpperBounds();
        glm::mat4 modelMatrix = GetModelMatrix();

        glm::vec3 corners[8] = {
            {localMin.x, localMin.y, localMin.z},
            {localMax.x, localMin.y, localMin.z},
            {localMin.x, localMax.y, localMin.z},
            {localMax.x, localMax.y, localMin.z},
            {localMin.x, localMin.y, localMax.z},
            {localMax.x, localMin.y, localMax.z},
            {localMin.x, localMax.y, localMax.z},
            {localMax.x, localMax.y, localMax.z},
        };

        Forge::AABB world;
        world.min = glm::vec3(std::numeric_limits<float>::max());
        world.max = glm::vec3(std::numeric_limits<float>::lowest());

        for (const glm::vec3 &corner : corners)
        {
            glm::vec3 worldCorner = glm::vec3(modelMatrix * glm::vec4(corner, 1.0f));
            world.min = glm::min(world.min, worldCorner);
            world.max = glm::max(world.max, worldCorner);
        }

        return world;
    }
} // namespace Forge
