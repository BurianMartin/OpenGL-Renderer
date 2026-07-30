#pragma once
#include "Forge/Rendering/Model.hpp"

#include <memory>
#include <vector>

namespace Forge
{
    /**
     * @brief Groups several Models that represent one logical object, so they move as a unit.
     *
     * Splitting an OBJ's `usemtl` groups into separate Models (see `Mesh::ParseObjFileGroups`/
     * `Mesh::CreateGroups`) means one visual object is really N independent Models, each with
     * its own Mesh/Material/transform. Nothing keeps those N transforms in sync on its own —
     * Prop exists purely to close that gap: it forwards `SetPosition`/`SetScale`/`SetRotation`
     * to every part it owns, so the whole object can be moved as a single rigid body instead of
     * the caller re-deriving "call this on every part" by hand.
     *
     * @note Deliberately not a rendering concept. Every part still has to be registered into
     * a `Layer`'s `materialModels_`/`materials_` for it to actually render — Prop doesn't
     * change draw dispatch or how `Layer::Render()` works, it only keeps transforms in sync.
     */
    class Prop
    {
        std::vector<std::shared_ptr<Model>> parts_;

    public:
        Prop() = default;

        /// Adds a part to this Prop. The caller is still responsible for registering `model`
        /// into a Layer's `materialModels_`/`materials_` — Prop only tracks it for the setters below.
        void AddPart(std::shared_ptr<Model> model)
        {
            parts_.push_back(std::move(model));
        }

        /// @return Every part owned by this Prop (e.g. to register each into a Layer).
        const std::vector<std::shared_ptr<Model>> &GetParts() const { return parts_; }

        /// Applies the same position to every part.
        void SetPosition(const glm::vec3 &pos)
        {
            for (auto &part : parts_)
                part->SetPosition(pos);
        }

        /// Applies the same scale to every part.
        void SetScale(const glm::vec3 &scale)
        {
            for (auto &part : parts_)
                part->SetScale(scale);
        }

        /// Equivalent to `SetScale(glm::vec3(uniform_scale))`.
        void SetScale(float uniform_scale)
        {
            SetScale(glm::vec3(uniform_scale));
        }

        /// Applies the same rotation to every part.
        void SetRotation(const glm::quat &rot)
        {
            for (auto &part : parts_)
                part->SetRotation(rot);
        }

        /// Equivalent to `SetRotation(glm::angleAxis(angle_radians, axis))`.
        void SetRotation(float angle_radians, const glm::vec3 &axis)
        {
            for (auto &part : parts_)
                part->SetRotation(angle_radians, axis);
        }
    };
} // namespace Forge
