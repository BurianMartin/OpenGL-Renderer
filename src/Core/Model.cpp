#include "Core/Model.hpp"
#include "Model.hpp"

namespace Core
{
    void Model::Draw() const
    {
        mesh_->bind();
        mesh_->draw();
    }

    glm::mat4 Model::getModelMatrix() const
    {
        glm::mat4 m = glm::mat4(1.0f);
        m = glm::translate(m, position_);
        m = glm::rotate(m, rotation_.x, glm::vec3(1, 0, 0));
        m = glm::rotate(m, rotation_.y, glm::vec3(0, 1, 0));
        m = glm::rotate(m, rotation_.z, glm::vec3(0, 0, 1));
        m = glm::scale(m, scale_);
        return m;
    }
} // namespace Core
