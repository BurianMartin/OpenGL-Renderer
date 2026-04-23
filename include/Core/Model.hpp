#pragma once
#include "Utils.hpp"
#include "Mesh.hpp"

#include <memory>

namespace Core
{
    class Model
    {
        const std::shared_ptr<Mesh> mesh; // non-owning reference (or shared_ptr)
        glm::vec3 position;
        glm::vec3 scale;
        glm::quat rotation; // prefer quat over Euler to avoid gimbal lock

    public:
        void Draw() const;
        glm::mat4 getModelMatrix() const;
    };
}