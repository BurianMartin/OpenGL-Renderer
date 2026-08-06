#pragma once
#include "Forge/Scene/Layer.hpp"
#include "Forge/Rendering/Panel.hpp"
#include "Forge/Rendering/ClickableRegion.hpp"

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Forge
{
    /**
     * @brief Click-and-drag for screen-space `Panel`s onto named drop zones — Forge's
     * generic drag-and-drop primitive, the counterpart to `UILayer`'s click-only `Button`s.
     *
     * Deliberately game-agnostic, same spirit as `ClickableRegion`/`Button`: `DragLayer`
     * knows nothing about cards, hands, or battlefields — just "these rects can be picked
     * up and moved by the cursor, those rects are places they can be dropped." A consumer
     * (e.g. `Demo`/a game's own client code) registers a `Panel` per draggable item, some
     * named drop-zone rects, and an `OnDrop` callback that decides whether a given
     * (item id, zone id) drop is legal — DragLayer only handles the input/movement
     * mechanics, not what a drop *means*.
     *
     * Each registered draggable owns its `Panel` — `DragLayer::OnRender` draws every one of
     * them, in registration order, except the item currently being dragged (if any) is
     * always drawn last so it renders on top of every other draggable mid-drag. Pair a
     * `Text` (or anything else) with a draggable's on-screen position via `OnDragMove`'s
     * live x/y callback if a caller wants a label to ride along. `OnHover` reports which
     * zone (if any) the cursor is currently over mid-drag, for e.g. highlighting a legal
     * drop target before the player actually releases — purely advisory, independent of
     * what `OnDrop` ultimately decides.
     *
     * Same `CursorMode::Captured` guard as `UILayer::OnEvent` — dragging only makes sense
     * against a real on-screen cursor position, which `Captured` mode doesn't have (see
     * `UILayer`'s own comment on this).
     */
    class DragLayer : public Layer
    {
    public:
        DragLayer(std::string name) : Layer(std::move(name)) {}

        /// Registers (or replaces) a draggable item. `panel`'s current rect becomes its
        /// "home" — where it snaps back to if a drag over it is later rejected.
        void AddDraggable(const std::string &id, std::shared_ptr<Panel> panel)
        {
            RemoveDraggable(id);
            draggables_.push_back({id, std::move(panel), 0.0f, 0.0f});
            auto &entry = draggables_.back();
            entry.homeX = entry.panel->GetX();
            entry.homeY = entry.panel->GetY();
        }

        void RemoveDraggable(const std::string &id)
        {
            std::erase_if(draggables_, [&](const Entry &e)
                           { return e.id == id; });
        }

        /// Removes every registered draggable — e.g. before rebuilding a hand's worth of
        /// card Panels wholesale after the game state they represent changed. Safe to call
        /// mid-drag (clears draggingIndex_ too); does nothing to drop zones, see ClearDropZones.
        void ClearDraggables()
        {
            draggables_.clear();
            draggingIndex_ = -1;
        }

        std::shared_ptr<Panel> GetDraggablePanel(const std::string &id) const
        {
            for (const auto &entry : draggables_)
                if (entry.id == id)
                    return entry.panel;
            return nullptr;
        }

        /// Moves a draggable not currently mid-drag and resets its home to the new position
        /// — e.g. re-laying out a hand after a card leaves it.
        void SetDraggablePosition(const std::string &id, float x, float y)
        {
            for (auto &entry : draggables_)
                if (entry.id == id)
                {
                    entry.panel->SetPosition(x, y);
                    entry.homeX = x;
                    entry.homeY = y;
                    return;
                }
        }

        void AddDropZone(const std::string &id, float x, float y, float width, float height)
        {
            RemoveDropZone(id);
            zones_.push_back({id, ClickableRegion(x, y, width, height)});
        }

        void RemoveDropZone(const std::string &id)
        {
            std::erase_if(zones_, [&](const Zone &z)
                           { return z.id == id; });
        }

        void ClearDropZones() { zones_.clear(); }

        /// Fired with an item's live top-left position every time it moves — during a drag,
        /// and once more with the home position if a drop is rejected and it snaps back.
        void SetOnDragMove(std::function<void(const std::string &id, float x, float y)> callback)
        {
            onDragMove_ = std::move(callback);
        }

        /// Fired once when a drag ends. `zoneId` is empty if released over no zone at all.
        /// Return true to accept the drop — the item's Panel stays wherever it now is, and
        /// that becomes its new home; return false (or leave no callback set at all) to
        /// reject it — DragLayer snaps the Panel back to its home position itself.
        void SetOnDrop(std::function<bool(const std::string &id, const std::string &zoneId)> callback)
        {
            onDrop_ = std::move(callback);
        }

        /// Fired whenever the zone under the cursor changes during a drag — at pickup (with
        /// whatever zone, if any, the cursor already happened to be over), on every move
        /// that crosses into/out of a zone, and once more with an empty zoneId when the
        /// drag ends (accepted or not) so a caller can always clear its own highlight
        /// rather than infer "drag over" from silence. Purely advisory — has no bearing on
        /// what OnDrop actually resolves to, e.g. a highlighted zone can still reject the
        /// drop.
        void SetOnHover(std::function<void(const std::string &id, const std::string &zoneId)> callback)
        {
            onHover_ = std::move(callback);
        }

        bool OnEvent(Event &e, std::shared_ptr<FrameContext> ctx) override
        {
            // See UILayer::OnEvent's own comment — a Captured cursor has no real on-screen
            // position to drag against.
            if (ctx->cursor_mode_ == CursorMode::Captured)
                return true;

            if (e.GetEventType() == EventType::MouseButtonPressed)
            {
                auto &ev = static_cast<MouseButtonPressedEvent &>(e);
                if (ev.GetMouseButton() != MouseButton::Left || draggingIndex_ != -1)
                    return true;

                float px = static_cast<float>(ev.GetX());
                float py = static_cast<float>(ev.GetY());

                std::vector<ClickableRegion> rects;
                rects.reserve(draggables_.size());
                for (const auto &entry : draggables_)
                    rects.emplace_back(entry.panel->GetX(), entry.panel->GetY(), entry.panel->GetWidth(),
                                        entry.panel->GetHeight());
                int i = FindTopmostContaining(rects, px, py);
                if (i < 0)
                    return true;

                // Copy (not reference) everything needed out before reordering draggables_
                // below — std::rotate move-assigns *values between slots* in place, it
                // doesn't relocate storage, so even a reference to a field (not just the
                // whole Entry) would end up reflecting whatever rotate move-assigned into
                // that slot afterward, not the entry we actually picked.
                std::string id = draggables_[static_cast<std::size_t>(i)].id;
                grabDX_ = px - draggables_[static_cast<std::size_t>(i)].panel->GetX();
                grabDY_ = py - draggables_[static_cast<std::size_t>(i)].panel->GetY();
                // Move it to the back so it draws (and hit-tests) on top for the rest of this drag.
                if (i != static_cast<int>(draggables_.size()) - 1)
                    std::rotate(draggables_.begin() + i, draggables_.begin() + i + 1, draggables_.end());
                draggingIndex_ = static_cast<int>(draggables_.size()) - 1;

                currentHoverZone_ = ZoneUnder(px, py);
                if (onHover_ && !currentHoverZone_.empty())
                    onHover_(id, currentHoverZone_);
                return false;
            }

            if (e.GetEventType() == EventType::MouseMoved && draggingIndex_ != -1)
            {
                auto &ev = static_cast<MouseMovedEvent &>(e);
                auto &entry = draggables_[draggingIndex_];
                float px = static_cast<float>(ev.GetX());
                float py = static_cast<float>(ev.GetY());
                entry.panel->SetPosition(px - grabDX_, py - grabDY_);
                if (onDragMove_)
                    onDragMove_(entry.id, entry.panel->GetX(), entry.panel->GetY());

                std::string zoneId = ZoneUnder(px, py);
                if (zoneId != currentHoverZone_)
                {
                    currentHoverZone_ = zoneId;
                    if (onHover_)
                        onHover_(entry.id, currentHoverZone_);
                }
                return true;
            }

            if (e.GetEventType() == EventType::MouseButtonReleased && draggingIndex_ != -1)
            {
                auto &ev = static_cast<MouseButtonReleasedEvent &>(e);
                if (ev.GetMouseButton() != MouseButton::Left)
                    return true;

                // Copy everything needed out by value BEFORE calling the caller's OnDrop --
                // that callback is exactly where a real consumer (e.g. an actual game)
                // mutates its own state and typically reacts by fully clearing and
                // rebuilding this DragLayer's draggables_ (a card that got played leaves
                // the hand). That reentrant rebuild would invalidate any reference/index
                // held into draggables_ across the call — `panel` is a shared_ptr
                // specifically so the Panel itself stays alive even if its owning Entry
                // is destroyed mid-callback.
                std::string id = draggables_[draggingIndex_].id;
                std::shared_ptr<Panel> panel = draggables_[draggingIndex_].panel;
                float homeX = draggables_[draggingIndex_].homeX;
                float homeY = draggables_[draggingIndex_].homeY;
                draggingIndex_ = -1;

                std::string zoneId = ZoneUnder(static_cast<float>(ev.GetX()), static_cast<float>(ev.GetY()));
                bool accepted = onDrop_ && onDrop_(id, zoneId);

                // Only touch draggables_ again if this id is still actually in it — the
                // callback may have already removed (or entirely replaced) it.
                for (auto &entry : draggables_)
                    if (entry.id == id)
                    {
                        if (accepted)
                        {
                            entry.homeX = entry.panel->GetX();
                            entry.homeY = entry.panel->GetY();
                        }
                        else
                        {
                            entry.panel->SetPosition(homeX, homeY);
                            if (onDragMove_)
                                onDragMove_(id, homeX, homeY);
                        }
                        break;
                    }

                if (onHover_ && !currentHoverZone_.empty())
                    onHover_(id, "");
                currentHoverZone_.clear();
                return false;
            }

            return true;
        }

        void OnUpdate(std::shared_ptr<FrameContext>) override {}

        void OnRender(std::shared_ptr<FrameContext> ctx) const override
        {
            for (const auto &entry : draggables_)
                if (entry.panel)
                    entry.panel->Draw(ctx->window_width_, ctx->window_height_);
        }

    private:
        struct Entry
        {
            std::string id;
            std::shared_ptr<Panel> panel;
            float homeX, homeY;
        };
        struct Zone
        {
            std::string id;
            ClickableRegion rect;
        };

        /// @return the id of the topmost zone containing (x, y), or "" if none do.
        std::string ZoneUnder(float x, float y) const
        {
            std::vector<ClickableRegion> zoneRects;
            zoneRects.reserve(zones_.size());
            for (const auto &zone : zones_)
                zoneRects.push_back(zone.rect);
            int idx = FindTopmostContaining(zoneRects, x, y);
            return idx >= 0 ? zones_[static_cast<std::size_t>(idx)].id : std::string();
        }

        std::vector<Entry> draggables_;
        std::vector<Zone> zones_;
        std::function<void(const std::string &, float, float)> onDragMove_;
        std::function<bool(const std::string &, const std::string &)> onDrop_;
        std::function<void(const std::string &, const std::string &)> onHover_;

        int draggingIndex_ = -1;
        float grabDX_ = 0.0f, grabDY_ = 0.0f;
        std::string currentHoverZone_;
    };
} // namespace Forge
