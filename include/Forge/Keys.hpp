#pragma once

namespace Forge
{
    /**
     * @brief Engine-owned key identifiers, decoupled from any windowing library.
     *
     * Values are deliberately wired to match GLFW's own `GLFW_KEY_*` numbering
     * — not a coincidence, and not derived from `<GLFW/glfw3.h>` (this header
     * includes nothing from GLFW). `EventHandler` (see `src/Forge/EventHandler.cpp`)
     * translates a raw GLFW key code into a `Key` with a plain `static_cast`,
     * no switch/lookup table required — nothing above that boundary needs to
     * include GLFW at all. Covers letters, digits, function keys, and the
     * common whitespace/modifier/punctuation keys; not every GLFW key has an
     * entry here (e.g. numpad, print screen) — add one with its matching GLFW
     * value if a future feature needs it.
     */
    enum class Key
    {
        Unknown = -1,

        A = 65,
        B = 66,
        C = 67,
        D = 68,
        E = 69,
        F = 70,
        G = 71,
        H = 72,
        I = 73,
        J = 74,
        K = 75,
        L = 76,
        M = 77,
        N = 78,
        O = 79,
        P = 80,
        Q = 81,
        R = 82,
        S = 83,
        T = 84,
        U = 85,
        V = 86,
        W = 87,
        X = 88,
        Y = 89,
        Z = 90,

        Num0 = 48,
        Num1 = 49,
        Num2 = 50,
        Num3 = 51,
        Num4 = 52,
        Num5 = 53,
        Num6 = 54,
        Num7 = 55,
        Num8 = 56,
        Num9 = 57,

        F1 = 290,
        F2 = 291,
        F3 = 292,
        F4 = 293,
        F5 = 294,
        F6 = 295,
        F7 = 296,
        F8 = 297,
        F9 = 298,
        F10 = 299,
        F11 = 300,
        F12 = 301,

        Space = 32,
        Tab = 258,
        Enter = 257,
        Escape = 256,
        Backspace = 259,
        Delete = 261,
        Insert = 260,
        Home = 268,
        End = 269,
        PageUp = 266,
        PageDown = 267,
        CapsLock = 280,

        Up = 265,
        Down = 264,
        Left = 263,
        Right = 262,

        LeftShift = 340,
        RightShift = 344,
        LeftControl = 341,
        RightControl = 345,
        LeftAlt = 342,
        RightAlt = 346,
        LeftSuper = 343,
        RightSuper = 347,

        GraveAccent = 96,
        Minus = 45,
        Equal = 61,
        LeftBracket = 91,
        RightBracket = 93,
        Backslash = 92,
        Semicolon = 59,
        Apostrophe = 39,
        Comma = 44,
        Period = 46,
        Slash = 47,
    };

    /**
     * @brief Engine-owned mouse button identifiers, decoupled from any windowing library.
     *
     * Values match GLFW's own `GLFW_MOUSE_BUTTON_*` numbering (0-indexed from
     * `Left`), the same "wire the numbers straight through" approach as `Key`
     * — no switch/lookup table needed at the `EventHandler` boundary, just a
     * `static_cast`.
     */
    enum class MouseButton
    {
        Unknown = -1,

        Left = 0,
        Right = 1,
        Middle = 2,
        Button4 = 3,
        Button5 = 4,
    };
} // namespace Forge
