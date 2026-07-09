#pragma once

namespace Forge
{
    /**
     * @brief Engine-owned key identifiers, decoupled from any windowing library.
     *
     * `EventHandler` is the only place that translates raw GLFW key codes into
     * these values (see `src/Forge/EventHandler.cpp`) — nothing above that
     * translation boundary needs to include GLFW at all. Covers letters,
     * digits, function keys, and the common whitespace/modifier/punctuation
     * keys; not every GLFW key has an entry here (e.g. numpad, print screen) —
     * extend both this enum and EventHandler's translation table together if
     * one is needed.
     */
    enum class Key
    {
        Unknown = -1,

        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,

        Num0,
        Num1,
        Num2,
        Num3,
        Num4,
        Num5,
        Num6,
        Num7,
        Num8,
        Num9,

        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,

        Space,
        Tab,
        Enter,
        Escape,
        Backspace,
        Delete,
        Insert,
        Home,
        End,
        PageUp,
        PageDown,
        CapsLock,

        Up,
        Down,
        Left,
        Right,

        LeftShift,
        RightShift,
        LeftControl,
        RightControl,
        LeftAlt,
        RightAlt,
        LeftSuper,
        RightSuper,

        GraveAccent,
        Minus,
        Equal,
        LeftBracket,
        RightBracket,
        Backslash,
        Semicolon,
        Apostrophe,
        Comma,
        Period,
        Slash,
    };

    /// Engine-owned mouse button identifiers — see `Key`'s doc comment for the same GLFW-decoupling rationale.
    enum class MouseButton
    {
        Unknown = -1,

        Left,
        Right,
        Middle,
        Button4,
        Button5,
    };
} // namespace Forge
