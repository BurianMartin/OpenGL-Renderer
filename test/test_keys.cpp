#include <gtest/gtest.h>
#include "Forge/Keys.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

// Keys.hpp intentionally has no GLFW dependency — Key/MouseButton's values are hand-wired to
// match GLFW_KEY_*/GLFW_MOUSE_BUTTON_* so EventHandler can translate with a plain static_cast
// instead of a lookup table. These tests are the safety net for that: this file (unlike
// Keys.hpp itself) is allowed to include GLFW, specifically to catch a fat-fingered value here
// ever drifting out of sync with GLFW's real numbering.

TEST(KeysTest, LettersMatchGLFW)
{
    EXPECT_EQ(static_cast<int>(Forge::Key::A), GLFW_KEY_A);
    EXPECT_EQ(static_cast<int>(Forge::Key::B), GLFW_KEY_B);
    EXPECT_EQ(static_cast<int>(Forge::Key::C), GLFW_KEY_C);
    EXPECT_EQ(static_cast<int>(Forge::Key::D), GLFW_KEY_D);
    EXPECT_EQ(static_cast<int>(Forge::Key::E), GLFW_KEY_E);
    EXPECT_EQ(static_cast<int>(Forge::Key::F), GLFW_KEY_F);
    EXPECT_EQ(static_cast<int>(Forge::Key::G), GLFW_KEY_G);
    EXPECT_EQ(static_cast<int>(Forge::Key::H), GLFW_KEY_H);
    EXPECT_EQ(static_cast<int>(Forge::Key::I), GLFW_KEY_I);
    EXPECT_EQ(static_cast<int>(Forge::Key::J), GLFW_KEY_J);
    EXPECT_EQ(static_cast<int>(Forge::Key::K), GLFW_KEY_K);
    EXPECT_EQ(static_cast<int>(Forge::Key::L), GLFW_KEY_L);
    EXPECT_EQ(static_cast<int>(Forge::Key::M), GLFW_KEY_M);
    EXPECT_EQ(static_cast<int>(Forge::Key::N), GLFW_KEY_N);
    EXPECT_EQ(static_cast<int>(Forge::Key::O), GLFW_KEY_O);
    EXPECT_EQ(static_cast<int>(Forge::Key::P), GLFW_KEY_P);
    EXPECT_EQ(static_cast<int>(Forge::Key::Q), GLFW_KEY_Q);
    EXPECT_EQ(static_cast<int>(Forge::Key::R), GLFW_KEY_R);
    EXPECT_EQ(static_cast<int>(Forge::Key::S), GLFW_KEY_S);
    EXPECT_EQ(static_cast<int>(Forge::Key::T), GLFW_KEY_T);
    EXPECT_EQ(static_cast<int>(Forge::Key::U), GLFW_KEY_U);
    EXPECT_EQ(static_cast<int>(Forge::Key::V), GLFW_KEY_V);
    EXPECT_EQ(static_cast<int>(Forge::Key::W), GLFW_KEY_W);
    EXPECT_EQ(static_cast<int>(Forge::Key::X), GLFW_KEY_X);
    EXPECT_EQ(static_cast<int>(Forge::Key::Y), GLFW_KEY_Y);
    EXPECT_EQ(static_cast<int>(Forge::Key::Z), GLFW_KEY_Z);
}

TEST(KeysTest, DigitsMatchGLFW)
{
    EXPECT_EQ(static_cast<int>(Forge::Key::Num0), GLFW_KEY_0);
    EXPECT_EQ(static_cast<int>(Forge::Key::Num1), GLFW_KEY_1);
    EXPECT_EQ(static_cast<int>(Forge::Key::Num2), GLFW_KEY_2);
    EXPECT_EQ(static_cast<int>(Forge::Key::Num3), GLFW_KEY_3);
    EXPECT_EQ(static_cast<int>(Forge::Key::Num4), GLFW_KEY_4);
    EXPECT_EQ(static_cast<int>(Forge::Key::Num5), GLFW_KEY_5);
    EXPECT_EQ(static_cast<int>(Forge::Key::Num6), GLFW_KEY_6);
    EXPECT_EQ(static_cast<int>(Forge::Key::Num7), GLFW_KEY_7);
    EXPECT_EQ(static_cast<int>(Forge::Key::Num8), GLFW_KEY_8);
    EXPECT_EQ(static_cast<int>(Forge::Key::Num9), GLFW_KEY_9);
}

TEST(KeysTest, FunctionKeysMatchGLFW)
{
    EXPECT_EQ(static_cast<int>(Forge::Key::F1), GLFW_KEY_F1);
    EXPECT_EQ(static_cast<int>(Forge::Key::F2), GLFW_KEY_F2);
    EXPECT_EQ(static_cast<int>(Forge::Key::F3), GLFW_KEY_F3);
    EXPECT_EQ(static_cast<int>(Forge::Key::F4), GLFW_KEY_F4);
    EXPECT_EQ(static_cast<int>(Forge::Key::F5), GLFW_KEY_F5);
    EXPECT_EQ(static_cast<int>(Forge::Key::F6), GLFW_KEY_F6);
    EXPECT_EQ(static_cast<int>(Forge::Key::F7), GLFW_KEY_F7);
    EXPECT_EQ(static_cast<int>(Forge::Key::F8), GLFW_KEY_F8);
    EXPECT_EQ(static_cast<int>(Forge::Key::F9), GLFW_KEY_F9);
    EXPECT_EQ(static_cast<int>(Forge::Key::F10), GLFW_KEY_F10);
    EXPECT_EQ(static_cast<int>(Forge::Key::F11), GLFW_KEY_F11);
    EXPECT_EQ(static_cast<int>(Forge::Key::F12), GLFW_KEY_F12);
}

TEST(KeysTest, WhitespaceAndControlKeysMatchGLFW)
{
    EXPECT_EQ(static_cast<int>(Forge::Key::Space), GLFW_KEY_SPACE);
    EXPECT_EQ(static_cast<int>(Forge::Key::Tab), GLFW_KEY_TAB);
    EXPECT_EQ(static_cast<int>(Forge::Key::Enter), GLFW_KEY_ENTER);
    EXPECT_EQ(static_cast<int>(Forge::Key::Escape), GLFW_KEY_ESCAPE);
    EXPECT_EQ(static_cast<int>(Forge::Key::Backspace), GLFW_KEY_BACKSPACE);
    EXPECT_EQ(static_cast<int>(Forge::Key::Delete), GLFW_KEY_DELETE);
    EXPECT_EQ(static_cast<int>(Forge::Key::Insert), GLFW_KEY_INSERT);
    EXPECT_EQ(static_cast<int>(Forge::Key::Home), GLFW_KEY_HOME);
    EXPECT_EQ(static_cast<int>(Forge::Key::End), GLFW_KEY_END);
    EXPECT_EQ(static_cast<int>(Forge::Key::PageUp), GLFW_KEY_PAGE_UP);
    EXPECT_EQ(static_cast<int>(Forge::Key::PageDown), GLFW_KEY_PAGE_DOWN);
    EXPECT_EQ(static_cast<int>(Forge::Key::CapsLock), GLFW_KEY_CAPS_LOCK);
}

TEST(KeysTest, ArrowKeysMatchGLFW)
{
    EXPECT_EQ(static_cast<int>(Forge::Key::Up), GLFW_KEY_UP);
    EXPECT_EQ(static_cast<int>(Forge::Key::Down), GLFW_KEY_DOWN);
    EXPECT_EQ(static_cast<int>(Forge::Key::Left), GLFW_KEY_LEFT);
    EXPECT_EQ(static_cast<int>(Forge::Key::Right), GLFW_KEY_RIGHT);
}

TEST(KeysTest, ModifierKeysMatchGLFW)
{
    EXPECT_EQ(static_cast<int>(Forge::Key::LeftShift), GLFW_KEY_LEFT_SHIFT);
    EXPECT_EQ(static_cast<int>(Forge::Key::RightShift), GLFW_KEY_RIGHT_SHIFT);
    EXPECT_EQ(static_cast<int>(Forge::Key::LeftControl), GLFW_KEY_LEFT_CONTROL);
    EXPECT_EQ(static_cast<int>(Forge::Key::RightControl), GLFW_KEY_RIGHT_CONTROL);
    EXPECT_EQ(static_cast<int>(Forge::Key::LeftAlt), GLFW_KEY_LEFT_ALT);
    EXPECT_EQ(static_cast<int>(Forge::Key::RightAlt), GLFW_KEY_RIGHT_ALT);
    EXPECT_EQ(static_cast<int>(Forge::Key::LeftSuper), GLFW_KEY_LEFT_SUPER);
    EXPECT_EQ(static_cast<int>(Forge::Key::RightSuper), GLFW_KEY_RIGHT_SUPER);
}

TEST(KeysTest, PunctuationKeysMatchGLFW)
{
    EXPECT_EQ(static_cast<int>(Forge::Key::GraveAccent), GLFW_KEY_GRAVE_ACCENT);
    EXPECT_EQ(static_cast<int>(Forge::Key::Minus), GLFW_KEY_MINUS);
    EXPECT_EQ(static_cast<int>(Forge::Key::Equal), GLFW_KEY_EQUAL);
    EXPECT_EQ(static_cast<int>(Forge::Key::LeftBracket), GLFW_KEY_LEFT_BRACKET);
    EXPECT_EQ(static_cast<int>(Forge::Key::RightBracket), GLFW_KEY_RIGHT_BRACKET);
    EXPECT_EQ(static_cast<int>(Forge::Key::Backslash), GLFW_KEY_BACKSLASH);
    EXPECT_EQ(static_cast<int>(Forge::Key::Semicolon), GLFW_KEY_SEMICOLON);
    EXPECT_EQ(static_cast<int>(Forge::Key::Apostrophe), GLFW_KEY_APOSTROPHE);
    EXPECT_EQ(static_cast<int>(Forge::Key::Comma), GLFW_KEY_COMMA);
    EXPECT_EQ(static_cast<int>(Forge::Key::Period), GLFW_KEY_PERIOD);
    EXPECT_EQ(static_cast<int>(Forge::Key::Slash), GLFW_KEY_SLASH);
}

TEST(KeysTest, UnknownMatchesGLFWUnknownSentinel)
{
    EXPECT_EQ(static_cast<int>(Forge::Key::Unknown), GLFW_KEY_UNKNOWN);
}

TEST(MouseButtonTest, ButtonsMatchGLFW)
{
    EXPECT_EQ(static_cast<int>(Forge::MouseButton::Left), GLFW_MOUSE_BUTTON_LEFT);
    EXPECT_EQ(static_cast<int>(Forge::MouseButton::Right), GLFW_MOUSE_BUTTON_RIGHT);
    EXPECT_EQ(static_cast<int>(Forge::MouseButton::Middle), GLFW_MOUSE_BUTTON_MIDDLE);
    EXPECT_EQ(static_cast<int>(Forge::MouseButton::Button4), GLFW_MOUSE_BUTTON_4);
    EXPECT_EQ(static_cast<int>(Forge::MouseButton::Button5), GLFW_MOUSE_BUTTON_5);
}
