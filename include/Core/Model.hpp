#pragma once
#include "Utils.hpp"
#include "Mesh.hpp"

#include <memory>

namespace Core
{
    class Model
    {
        const std::shared_ptr<Mesh> mesh_; // non-owning reference (or shared_ptr)
        glm::vec3 position_;
        glm::vec3 scale_;
        glm::quat rotation_; // prefer quat over Euler to avoid gimbal lock

        // TODO: Add textures and lighting

    public:
        void Draw() const;
        glm::mat4 getModelMatrix() const;

        Model(std::shared_ptr<Mesh> mesh) : mesh_(mesh) {}
        ~Model() = default;
    };
}