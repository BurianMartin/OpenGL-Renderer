#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

namespace Forge
{
    /**
     * @brief A world-space line: an origin point plus a normalized direction.
     *
     * Produced by UnprojectScreenPoint()/Camera::ScreenPointToRay() from a screen-space
     * click, and consumed by IntersectRayAABB()/IntersectRayPlane() to find what (if
     * anything) it hits. Kept as its own type specifically because it's created in one
     * place and tested against shapes in another — bundling origin+direction avoids the
     * two ever getting swapped or mismatched across that handoff.
     */
    struct Ray
    {
        glm::vec3 origin{0.0f};
        glm::vec3 direction{0.0f, 0.0f, -1.0f};
    };

    /**
     * @brief Axis-aligned bounding box: opposite corners `min`/`max`.
     *
     * Its own type for the same reason as Ray: it's produced in one place
     * (Model::GetWorldBounds()) and consumed in another (IntersectRayAABB(), by way of
     * Layer::GetClickedObj()) — bundling the two corners means there's only one value to
     * pass along that handoff, not two that could get swapped at a call site.
     */
    struct AABB
    {
        glm::vec3 min{0.0f};
        glm::vec3 max{0.0f};
    };

    /**
     * @brief Ray/axis-aligned-box intersection via the slab method.
     *
     * Treats the box as three pairs of parallel planes (one pair per axis) and finds the
     * range of `t` (the ray's `origin + t * direction` parameter) for which the ray is
     * between each pair, then intersects the three ranges — if what's left is non-empty,
     * the ray passes through the box. `tMin` starts at 0 rather than `-infinity` so a box
     * behind the ray's origin never counts as a hit.
     * @param box The box, in the same space as `ray` (world space, typically — see Model::GetWorldBounds()).
     * @param outT Set to the ray parameter of the nearest intersection on a hit; untouched otherwise.
     * @return True if the ray intersects the box at `t >= 0`.
     */
    inline bool IntersectRayAABB(const Ray &ray, const AABB &box, float &outT)
    {
        float tMin = 0.0f;
        float tMax = std::numeric_limits<float>::max();

        for (int axis = 0; axis < 3; ++axis)
        {
            float origin = ray.origin[axis];
            float dir = ray.direction[axis];
            float minPlane = box.min[axis];
            float maxPlane = box.max[axis];

            if (std::abs(dir) < 1e-8f)
            {
                // Ray runs parallel to this axis's slab — only still a hit if the origin
                // already lies within it, since it will never cross either plane.
                if (origin < minPlane || origin > maxPlane)
                    return false;
                continue;
            }

            float invDir = 1.0f / dir;
            float t1 = (minPlane - origin) * invDir;
            float t2 = (maxPlane - origin) * invDir;
            if (t1 > t2)
                std::swap(t1, t2);

            tMin = std::max(tMin, t1);
            tMax = std::min(tMax, t2);
            if (tMin > tMax)
                return false;
        }

        outT = tMin;
        return true;
    }

    /**
     * @brief Ray/plane intersection, given any point on the plane plus its normal.
     * @param outT Set to the ray parameter of the intersection on a hit; untouched otherwise.
     * @return True if the ray hits the plane at `t >= 0`. False if the ray is (near-)parallel
     * to the plane (no single intersection point) or the plane is behind the ray's origin.
     */
    inline bool IntersectRayPlane(const Ray &ray, const glm::vec3 &planePoint, const glm::vec3 &planeNormal, float &outT)
    {
        float denom = glm::dot(planeNormal, ray.direction);
        if (std::abs(denom) < 1e-8f)
            return false;

        outT = glm::dot(planePoint - ray.origin, planeNormal) / denom;
        return outT >= 0.0f;
    }

    /**
     * @brief Unprojects a screen-space pixel into a world-space Ray.
     *
     * Reverses the normal render transform (world -> view -> projection -> NDC) via
     * `inverse(projection * view)`, applied at both the near and far plane in NDC space
     * to get two world-space points — the ray runs from the near point towards the far
     * one. `screenX`/`screenY` use the same convention as MouseMovedEvent/GLFW (pixels,
     * origin top-left, y grows downward); `viewportX/Y/Width/Height` is the on-screen
     * pixel rect the point should be interpreted against (a Camera's own Viewport, or an
     * approximation like the full window when no per-camera rect is available).
     * @param outRay Set to the resulting world-space ray on success; untouched otherwise.
     * @return False if `screenX`/`screenY` falls outside the given viewport rect, or the rect is empty.
     */
    inline bool UnprojectScreenPoint(GLfloat screenX, GLfloat screenY,
                                     GLint viewportX, GLint viewportY, GLint viewportWidth, GLint viewportHeight,
                                     const glm::mat4 &view, const glm::mat4 &projection,
                                     Ray &outRay)
    {
        if (viewportWidth <= 0 || viewportHeight <= 0)
            return false;

        float localX = screenX - static_cast<float>(viewportX);
        float localY = screenY - static_cast<float>(viewportY);
        if (localX < 0.0f || localX > static_cast<float>(viewportWidth) ||
            localY < 0.0f || localY > static_cast<float>(viewportHeight))
            return false;

        // Screen space (origin top-left, y down) -> NDC ([-1,1], y up).
        float ndcX = (2.0f * localX / static_cast<float>(viewportWidth)) - 1.0f;
        float ndcY = 1.0f - (2.0f * localY / static_cast<float>(viewportHeight));

        glm::mat4 invViewProj = glm::inverse(projection * view);

        glm::vec4 nearPoint = invViewProj * glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
        glm::vec4 farPoint = invViewProj * glm::vec4(ndcX, ndcY, 1.0f, 1.0f);
        // A forward projection's perspective divide happens for you on the GPU; going
        // backward through the inverse matrix doesn't do that automatically — w has to be
        // divided out by hand before nearPoint/farPoint.xyz mean anything.
        nearPoint /= nearPoint.w;
        farPoint /= farPoint.w;

        outRay.origin = glm::vec3(nearPoint);
        outRay.direction = glm::normalize(glm::vec3(farPoint - nearPoint));
        return true;
    }
} // namespace Forge
