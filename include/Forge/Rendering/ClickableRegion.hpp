#pragma once
#include "Utils.hpp"
#include "Forge/Core/InputEvents.hpp"

#include <functional>
#include <vector>

namespace Forge
{
    /**
     * @brief A screen-space rectangular hit-region with an optional click callback.
     *
     * Deliberately not a rendering concept — same spirit as `Prop` not being one either — a
     * `ClickableRegion` doesn't know how to draw itself; it only knows whether a screen
     * point falls inside its rect and what to do if a left-click lands there. Pair it with
     * whatever visual actually represents it (a `Text` label, a `Model`, nothing at all for
     * an invisible hit-zone) — see `Button` below for the common "label + region" case.
     *
     * Coordinates are screen-space pixels, origin top-left, y down — the same convention
     * `MouseButtonEvent::GetX()/GetY()` and `Forge::Text` already use, so a region's rect
     * can be compared directly against a click's raw coordinates with no unprojection or
     * camera involved at all (unlike `Layer::GetClickedObj()`'s 3D ray/AABB picking, a UI
     * region is flat, always-on-screen, and never needs to account for a camera moving).
     */
    class ClickableRegion
    {
    public:
        ClickableRegion(float x, float y, float width, float height)
            : x_(x), y_(y), width_(width), height_(height) {}

        /// @return True if (screenX, screenY) falls within this region's rect (inclusive of its edges).
        bool Contains(float screenX, float screenY) const
        {
            return screenX >= x_ && screenX <= x_ + width_ && screenY >= y_ && screenY <= y_ + height_;
        }

        /// Repositions/resizes this region — e.g. to follow a moved/relabeled Button.
        void SetRect(float x, float y, float width, float height)
        {
            x_ = x;
            y_ = y;
            width_ = width;
            height_ = height;
        }

        /// Replaces the callback invoked by a successful HandleClick(). Pass `nullptr` to clear it.
        void SetOnClick(std::function<void()> callback) { onClick_ = std::move(callback); }

        /**
         * @brief Tests a left-click against this region and invokes its callback on a hit.
         * @return True if `ev` was a left-click inside this region (whether or not a
         * callback was actually set) — the opposite polarity from `Layer::OnEvent`'s
         * `false`-means-consumed convention; a caller wrapping this in an `OnEvent`
         * override needs to invert it.
         */
        bool HandleClick(const MouseButtonPressedEvent &ev) const
        {
            if (ev.GetMouseButton() != MouseButton::Left)
                return false;
            if (!Contains(static_cast<float>(ev.GetX()), static_cast<float>(ev.GetY())))
                return false;
            if (onClick_)
                onClick_();
            return true;
        }

        float GetX() const { return x_; }
        float GetY() const { return y_; }
        float GetWidth() const { return width_; }
        float GetHeight() const { return height_; }

    private:
        float x_, y_, width_, height_;
        std::function<void()> onClick_;
    };

    /**
     * @brief Picks whichever of several overlapping regions is "on top."
     *
     * A free function rather than a method since it operates over a whole collection —
     * `DragLayer` uses this to decide which draggable a click should pick up, and which
     * drop zone a release lands on, when more than one candidate's rect contains the point.
     * "On top" means last in `regions` (vector order doubles as z-order, matching how
     * `Scene::OnEvent`/`UILayer` already treat later-registered/-drawn things as being
     * visually in front) — no separate z-index field to keep in sync.
     * @return The index of the topmost region containing (x, y), or -1 if none do.
     */
    inline int FindTopmostContaining(const std::vector<ClickableRegion> &regions, float x, float y)
    {
        for (int i = static_cast<int>(regions.size()) - 1; i >= 0; --i)
            if (regions[i].Contains(x, y))
                return i;
        return -1;
    }
} // namespace Forge
