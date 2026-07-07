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

    /**
     * @brief A drawable instance: a shared Mesh plus its own transform and material.
     *
     * Multiple Models can share one Mesh (position/scale/rotation are
     * per-Model, geometry is not duplicated). @note The Material reference
     * here is a convenience for queries only — `Layer::materialModels_` is
     * the authority that actually decides which material a model renders
     * with; setting one here does not automatically add the model to that
     * material's bucket.
     */
    class Model
    {
        const std::shared_ptr<Mesh> mesh_;
        glm::vec3 position_;
        glm::vec3 scale_;
        glm::quat rotation_;

        std::shared_ptr<Material> material_;

    public:
        /// Binds the underlying Mesh and issues its draw call.
        void Draw() const;

        /// @return The composed translation * rotation * scale matrix for this instance, for the `uModel` uniform.
        glm::mat4 GetModelMatrix() const;

        void SetPosition(const glm::vec3 &pos);
        void SetScale(const glm::vec3 &scale);

        /// Equivalent to `SetScale(glm::vec3(uniform_scale))`.
        void SetScale(float uniform_scale);
        void SetRotation(const glm::quat &rot);

        /// Equivalent to `SetRotation(glm::angleAxis(angle_radians, axis))` — `axis` is normalized internally, so callers don't need to pre-normalize it.
        void SetRotation(float angle_radians, const glm::vec3 &axis);

        /// Sets the convenience material reference (see class note — does not affect actual render dispatch).
        void SetMaterial(std::shared_ptr<Material> material);
        std::shared_ptr<Material> GetMaterial() const;

        /// @param mesh Geometry this instance draws; may be shared with other Models.
        Model(std::shared_ptr<Mesh> mesh);
        ~Model() = default;
    };
}
