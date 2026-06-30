#pragma once
#include "Utils.hpp"
#include "Mesh.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>

namespace Core
{
    class Material;

    class Model
    {
        const std::shared_ptr<Mesh> mesh_;
        glm::vec3 position_;
        glm::vec3 scale_;
        glm::quat rotation_;

        std::shared_ptr<Material> material_;

    public:
        void Draw() const;
        glm::mat4 getModelMatrix() const;

        void SetPosition(const glm::vec3 &pos);
        void SetScale(const glm::vec3 &scale);
        void SetScale(float uniform_scale);
        void SetRotation(const glm::quat &rot);
        void SetRotation(float angle_radians, const glm::vec3 &axis);

        void SetMaterial(std::shared_ptr<Material> material);
        std::shared_ptr<Material> GetMaterial() const;

        Model(std::shared_ptr<Mesh> mesh);
        ~Model() = default;
    };
}
