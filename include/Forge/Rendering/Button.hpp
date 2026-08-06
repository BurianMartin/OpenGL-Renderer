#pragma once
#include "Forge/Rendering/ClickableRegion.hpp"
#include "Forge/Rendering/Text.hpp"
#include "Forge/Rendering/Font.hpp"
#include "Forge/Core/ResourceManager.hpp"

#include <memory>
#include <string>

namespace Forge
{
    /**
     * @brief The common "label + clickable region" UI element — a `Text` for the visual,
     * a `ClickableRegion` (padded around the label's own Text::GetSize()) for the hit-test.
     *
     * Deliberately thin: it doesn't draw a background/border (this engine has no generic
     * screen-space quad primitive yet to draw one with — a real gap, but a separate one
     * from the clickable-region abstraction itself), so a Button today is a piece of text
     * with an invisible padded click target around it, not a boxed button sprite.
     */
    class Button
    {
    public:
        /// @param x,y Same top-left-ish screen position `Text::Create` takes.
        /// @param paddingX,paddingY Extra clickable margin around the label's own bounding box on each side.
        /// @return A new Button, or `nullptr` if the underlying Text failed to build (e.g. missing shader).
        static std::shared_ptr<Button> Create(std::shared_ptr<ResourceManager> rmanager, std::shared_ptr<Font> font,
                                               const std::string &label, float x, float y,
                                               const glm::vec4 &textColor = glm::vec4(1.0f),
                                               float paddingX = 8.0f, float paddingY = 6.0f)
        {
            auto text = Text::Create(rmanager, font, label, x, y, textColor);
            if (!text)
                return nullptr;

            // Padded around the label's actual visual bounding box (GetTopLeft(), not (x, y)
            // -- (x, y) is the text's BASELINE, which sits below where ascenders/capitals
            // actually render; using it directly here used to leave tall glyphs poking out
            // the top of the region/background box).
            glm::vec2 size = text->GetSize();
            glm::vec2 topLeft = text->GetTopLeft();
            ClickableRegion region(topLeft.x - paddingX, topLeft.y - paddingY, size.x + 2.0f * paddingX,
                                    size.y + 2.0f * paddingY);
            return std::shared_ptr<Button>(new Button(std::move(text), region));
        }

        /// Replaces the callback invoked on a successful click — see ClickableRegion::SetOnClick.
        void SetOnClick(std::function<void()> callback) { region_.SetOnClick(std::move(callback)); }

        /// @return True if `ev` landed inside this button (its callback, if any, already ran).
        bool HandleClick(const MouseButtonPressedEvent &ev) const { return region_.HandleClick(ev); }

        /// Draws the label. The clickable region itself has no visual (see class doc).
        void Draw(GLint windowWidth, GLint windowHeight) const { label_->Draw(windowWidth, windowHeight); }

        const ClickableRegion &GetRegion() const { return region_; }

    private:
        Button(std::shared_ptr<Text> label, ClickableRegion region)
            : label_(std::move(label)), region_(region) {}

        std::shared_ptr<Text> label_;
        ClickableRegion region_;
    };
} // namespace Forge
