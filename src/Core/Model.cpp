#include "Core/Model.hpp"

namespace Core
{
    Model::Model(std::shared_ptr<Mesh> mesh) : mesh_(mesh)
    {
        position_ = glm::vec3(0.0f, 0.0f, 0.0f);
        rotation_ = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f));
        scale_ = glm::vec3(1.0f, 1.0f, 1.0f); // scale of 0 makes it invisible
    }

    void Model::Draw() const
    {
        mesh_->draw();
    }

    glm::mat4 Model::getModelMatrix() const
    {
        glm::mat4 m = glm::mat4(1.0f);
        m = glm::translate(m, position_);
        m = m * glm::mat4_cast(rotation_);
        m = glm::scale(m, scale_);
        return m;
    }
} // namespace Core
