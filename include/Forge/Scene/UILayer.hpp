#pragma once
#include "Forge/Scene/Layer.hpp"
#include "Forge/Rendering/Button.hpp"
#include "Forge/Rendering/Text.hpp"
#include "Forge/Rendering/Panel.hpp"

#include <memory>
#include <vector>

namespace Forge
{
    /**
     * @brief Always-visible screen-space UI: owns `Panel`s, `Button`s, and/or plain `Text`
     * labels, draws them every frame, and forwards left-clicks to each Button's hit-test.
     *
     * `Scene::Render()` draws every layer's screen-space content (via `OnRender()` ->
     * `RenderScreenSpace()`) in one pass *after* every camera's 3D pass has finished, so
     * this layer's UI always sits on top of the scene's 3D content regardless of
     * registration order. Registration order still matters for input, though:
     * `Scene::OnEvent` dispatches to layers in reverse registration order, so a `UILayer`
     * added last also gets first look at input — a click on a button is consumed
     * (returns `false`) before it can fall through to 3D object picking underneath it;
     * a click that misses every button passes through untouched.
     *
     * Deliberately generic and demo-independent — a `Scene` builds whatever `Button`s/
     * `Text` it needs (typically handed in from whichever `Layer` actually owns the
     * state a button should act on, since `UILayer` itself doesn't know about game
     * state) and just registers them here.
     */
    class UILayer : public Layer
    {
    public:
        UILayer(std::string name) : Layer(std::move(name)) {}
        ~UILayer() = default;

        void AddButton(std::shared_ptr<Button> button) { buttons_.push_back(std::move(button)); }
        void AddLabel(std::shared_ptr<Text> label) { labels_.push_back(std::move(label)); }
        /// Non-interactive background visual — e.g. marking a `DragLayer` drop zone. Drawn
        /// before labels/buttons so it sits behind them.
        void AddPanel(std::shared_ptr<Panel> panel) { panels_.push_back(std::move(panel)); }

        /// Tests every Button's HandleClick() on a left-click; consumes the event (returns
        /// `false`) the moment one of them handles it. Anything else passes through.
        bool OnEvent(Forge::Event &e, std::shared_ptr<Forge::FrameContext> ctx) override
        {
            if (e.GetEventType() == Forge::EventType::MouseButtonPressed)
            {
                // While the cursor is Captured, MouseButtonEvent::GetX()/GetY() is GLFW's
                // virtual mouse-look accumulator, not a real screen pixel (same reason
                // Layer::GetClickedOnObj() doesn't trust it either — see that method's own
                // comment). There's no sensible on-screen position to fall back to for a
                // fixed 2D button the way GetClickedOnObj falls back to the window's center
                // pixel for 3D picking, so buttons just aren't hit-testable right now — the
                // click event itself still reaches here and passes through untouched (`true`),
                // it's only ever *this layer's* interpretation of it that's skipped.
                if (ctx->cursor_mode_ != Forge::CursorMode::Captured)
                {
                    auto &ev = static_cast<Forge::MouseButtonPressedEvent &>(e);
                    for (const auto &button : buttons_)
                        if (button && button->HandleClick(ev))
                            return false;
                }
            }
            return true;
        }

        /// No-op — buttons/labels here are static once created; nothing to update per frame.
        void OnUpdate(std::shared_ptr<Forge::FrameContext>) override {}

        /// Draws every panel, then every label, then every button's label.
        void OnRender(std::shared_ptr<Forge::FrameContext> ctx) const override
        {
            for (const auto &panel : panels_)
                if (panel)
                    panel->Draw(ctx->window_width_, ctx->window_height_);
            for (const auto &label : labels_)
                if (label)
                    label->Draw(ctx->window_width_, ctx->window_height_);
            for (const auto &button : buttons_)
                if (button)
                    button->Draw(ctx->window_width_, ctx->window_height_);
        }

    private:
        std::vector<std::shared_ptr<Panel>> panels_;
        std::vector<std::shared_ptr<Button>> buttons_;
        std::vector<std::shared_ptr<Text>> labels_;
    };
} // namespace Forge
