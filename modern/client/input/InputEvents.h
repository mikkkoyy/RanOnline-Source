#pragma once

// CLIENT-003: platform-independent input event types.
//
// The modern input API must not expose VK_*, HWND, WPARAM, LPARAM, MFC
// message types or DirectInput scan codes. A Windows implementation
// translates those values internally (see
// platform/Win32InputAdapter.h) and only these types cross the boundary.
//
// Source reference (behavior studied, not copied): legacy
// GameClient2/GameClient2Wnd.cpp (OnKeyDown/OnMouseMove forward Win32
// messages), Lib_Engine/GUInterface/UIKeyCheck.h (Check consumes a key
// event once, CheckSimple polls raw state — the event/state split below),
// Lib_Engine/GUInterface/UIMessageQueue.h (FIFO Post/Peek queue), and
// Lib_Engine/Common/GameInput.h (portable GameKey/GameMouseState model
// whose key coverage this enum mirrors).

#include <cstdint>
#include <variant>

namespace Modern::Client
{

// Keys the game consumes. This is intentionally not a 1:1 mirror of every
// platform key: unmapped platform keys arrive as KeyCode::Unknown and are
// rejected by InputSystem::PushEvent instead of entering the queue.
enum class KeyCode : uint16_t
{
	Unknown = 0,

	Escape,
	F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
	PrintScreen, ScrollLock, Pause,

	Tilde, Minus, Equals, Backspace, Enter, Tab, Space,
	LeftBracket, RightBracket, Backslash, Semicolon, Quote,
	Comma, Period, Slash,

	Zero, One, Two, Three, Four, Five, Six, Seven, Eight, Nine,

	A, B, C, D, E, F, G, H, I, J, K, L, M,
	N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

	LeftShift, RightShift,
	LeftControl, RightControl,
	LeftAlt, RightAlt,

	Left, Right, Up, Down,
	Insert, Home, PageUp, End, PageDown, Delete,

	NumLock,
	Numpad0, Numpad1, Numpad2, Numpad3, Numpad4,
	Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,
	NumpadDivide, NumpadMultiply, NumpadMinus,
	NumpadPlus, NumpadPeriod, NumpadEnter,

	Count
};

const char* ToString(KeyCode code) noexcept;

enum class MouseButton : uint8_t
{
	Unknown = 0,
	Left,
	Right,
	Middle,
	X1,
	X2,

	Count
};

const char* ToString(MouseButton button) noexcept;

enum class InputEventType : uint8_t
{
	Key = 0,
	MouseMove,
	MouseButton,
	MouseWheel,
	WindowClosed,
	WindowResized
};

const char* ToString(InputEventType type) noexcept;


// Mouse payloads use raw pixel integers, not floats: coordinates are exact
// values the platform reports, and any DPI/scaling conversion belongs to a
// future UI layer, not to the event.
struct KeyEvent
{
	KeyCode code     = KeyCode::Unknown;
	bool    pressed  = false;
	bool    repeated = false;
};

struct MouseMoveEvent
{
	int32_t x = 0;
	int32_t y = 0;
};

struct MouseButtonEvent
{
	MouseButton button  = MouseButton::Unknown;
	bool        pressed = false;
	int32_t     x       = 0;
	int32_t     y       = 0;
};

struct MouseWheelEvent
{
	int32_t deltaX = 0;
	int32_t deltaY = 0;
};

struct WindowResizeEvent
{
	uint32_t width  = 0;
	uint32_t height = 0;
};

struct InputEvent
{
	InputEventType type = InputEventType::WindowClosed;

	// Only the member matching type carries meaning. A struct + variant
	// would add a second discriminator for no benefit; the type field is
	// the single source of truth, matching the core's small-convention
	// style.
	std::variant<
		KeyEvent,
		MouseMoveEvent,
		MouseButtonEvent,
		MouseWheelEvent,
		WindowResizeEvent,
		std::monostate>
		payload{ std::monostate{} };

	static InputEvent MakeKey(KeyCode code, bool pressed, bool repeated = false)
	{
		InputEvent event;
		event.type    = InputEventType::Key;
		event.payload = KeyEvent{ code, pressed, repeated };
		return event;
	}

	static InputEvent MakeMouseMove(int32_t x, int32_t y)
	{
		InputEvent event;
		event.type    = InputEventType::MouseMove;
		event.payload = MouseMoveEvent{ x, y };
		return event;
	}

	static InputEvent MakeMouseButton(MouseButton button, bool pressed, int32_t x, int32_t y)
	{
		InputEvent event;
		event.type    = InputEventType::MouseButton;
		event.payload = MouseButtonEvent{ button, pressed, x, y };
		return event;
	}

	static InputEvent MakeMouseWheel(int32_t deltaX, int32_t deltaY)
	{
		InputEvent event;
		event.type    = InputEventType::MouseWheel;
		event.payload = MouseWheelEvent{ deltaX, deltaY };
		return event;
	}

	static InputEvent MakeWindowClosed()
	{
		InputEvent event;
		event.type    = InputEventType::WindowClosed;
		event.payload = std::monostate{};
		return event;
	}

	static InputEvent MakeWindowResized(uint32_t width, uint32_t height)
	{
		InputEvent event;
		event.type    = InputEventType::WindowResized;
		event.payload = WindowResizeEvent{ width, height };
		return event;
	}
};

} // namespace Modern::Client
