// GameInput.h
//
// Portable game-input contract for the RAN Online modernization.
//
// This header is intentionally platform-neutral. It must be includable
// from Windows, Android, and iOS projects without pulling in Windows.h,
// dinput.h, MFC, or any DirectX/D3D headers.
//
// The interface represents GAME INPUT (what happened), not how Windows
// obtained it (DirectInput / Win32). The Windows-specific implementation
// lives in Lib_Engine/DxCommon/DxInputDevice and is the ONLY place that
// may reference dinput.h, DIK_*, LPDIRECTINPUT*, etc.
//
// Key codes are kept as a scoped enum. Unmapped / platform-specific keys
// fall back to GameKey::Unknown; the Windows implementation owns the
// DIK scan-code mapping and never exposes it through this interface.

#pragma once

#include "GameTypes.h"

namespace GameInput
{
    // ---------------------------------------------------------------------------
    // Portable key representation.
    //
    // This enum is intentionally limited to the keys the game actually
    // consumes. It is NOT a 1:1 mirror of every DIK_* code. Keys not listed
    // here are reported as GameKey::Unknown by the platform implementation.
    // ---------------------------------------------------------------------------
    enum class GameKey : GameUInt16
    {
        Unknown = 0,

        // Function keys
        Escape,
        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
        PrintScreen, ScrollLock, Pause,

        // Common punctuation / number row
        Tilde, Minus, Equals, Backspace, Enter,
        Tab, Space,
        LeftBracket, RightBracket, Backslash, Semicolon, Quote,
        Comma, Period, Slash,

        // Number row
        Zero, One, Two, Three, Four, Five,
        Six, Seven, Eight, Nine,

        // Letters
        A, B, C, D, E, F, G, H, I, J, K, L, M,
        N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

        // Modifier keys
        LeftShift, RightShift,
        LeftControl, RightControl,
        LeftAlt, RightAlt,

        // Navigation cluster
        Left, Right, Up, Down,
        Insert, Home, PageUp, End, PageDown, Delete,

        // Numeric keypad
        NumLock,
        Numpad0, Numpad1, Numpad2, Numpad3, Numpad4,
        Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,
        NumpadDivide, NumpadMultiply, NumpadMinus,
        NumpadPlus, NumpadPeriod, NumpadEnter,

        // Mouse buttons
        MouseLeft, MouseRight, MouseMiddle,
        Mouse3, Mouse4, Mouse5, Mouse6, Mouse7,
    };

    // ---------------------------------------------------------------------------
    // Portable key-state flags.
    //
    // These mirror the legacy EMDXKEYFLAGS semantics exactly so the Windows
    // implementation can pass them through unchanged.
    // ---------------------------------------------------------------------------
    enum class GameKeyState : GameUInt16
    {
        Idle      = 0x0001,
        Down      = 0x0002,
        Pressed   = 0x0004,
        Drag      = 0x0008,
        Up        = 0x0010,
        Dup       = 0x0020,

        Downed    = Down | Pressed | Drag,
    };

    // ---------------------------------------------------------------------------
    // Portable mouse state snapshot.
    //
    // Carries the data gameplay actually consumes: screen position, per-frame
    // delta, and per-button state. No DIMOUSESTATE / POINT / HWND exposure.
    // ---------------------------------------------------------------------------
    struct GameMouseState
    {
        int x;             // current cursor x
        int y;             // current cursor y
        int dx;            // per-frame delta x
        int dy;            // per-frame delta y
        int dz;            // per-frame wheel delta
        GameKeyState buttons[8];
    };

    // ---------------------------------------------------------------------------
    // Portable input manager contract.
    //
    // Implementations own ALL platform-specific state. Consumers receive only
    // the portable state above.
    // ---------------------------------------------------------------------------
    class IInputDevice
    {
    public:
        virtual ~IInputDevice() = default;

        // Lifecycle. The Windows implementation receives the bare HWND; the
        // portable contract treats it as an opaque platform handle.
        virtual void Initialize(void* windowHandle) = 0;
        virtual void Shutdown() = 0;
        virtual void Update(float time, float elapsedTime,
                            bool defaultWindow, bool inGame) = 0;

        // Keyboard
        virtual GameKeyState GetKeyState(GameKey key) const = 0;
        virtual bool IsKeyDown(GameKey key) const = 0;
        virtual bool IsKeyReleased(GameKey key) const = 0;

        // Mouse
        virtual GameMouseState GetMouseState() const = 0;
        virtual void GetMouseMove(int& dx, int& dy, int& dz) const = 0;
        virtual void GetMouseLocate(int& x, int& y, int& z) const = 0;
    };
}