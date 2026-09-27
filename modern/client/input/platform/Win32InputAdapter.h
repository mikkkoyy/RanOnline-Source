#pragma once

// CLIENT-003: Windows translation boundary.
//
// Win32 message types (VK_*, WPARAM/LPARAM, WM_*) must not leak past this
// adapter. The public surface speaks only KeyCode/MouseButton/InputEvent;
// the raw Win32 constants live in the .cpp as local translation tables,
// mirroring how legacy Lib_Engine/DxCommon/DxInputDevice owns the DIK_*
// mapping behind the portable GameInput.h contract.
//
// The adapter is intentionally dumb: it translates one raw sample into zero
// or more InputEvents and pushes them. No lifecycle, no queue ownership.

#include "../InputEvents.h"

#include <cstdint>
#include <vector>

namespace Modern::Client
{

// A single raw sample from the Windows message pump, already stripped of
// HWND/WPARAM/LPARAM typing at the call site. The adapter translates it;
// nothing Win32-typed crosses into InputSystem.
struct Win32KeySample
{
	uint32_t virtualKey = 0;
	bool     pressed    = false;
	bool     repeated   = false;
};

struct Win32MouseSample
{
	int32_t  x              = 0;
	int32_t  y              = 0;
	int32_t  wheelDelta     = 0;
	bool     leftPressed    = false;
	bool     leftChanged    = false;
	bool     rightPressed   = false;
	bool     rightChanged   = false;
	bool     middlePressed  = false;
	bool     middleChanged  = false;
	bool     moved          = false;
};

class Win32InputAdapter
{
public:
	// Maps a virtual-key code to the portable KeyCode. Unmapped codes
	// yield KeyCode::Unknown; callers then skip the event rather than
	// queueing it.
	static KeyCode TranslateKey(uint32_t virtualKey) noexcept;

	// Emits at most one key event. Returns 0 events for Unknown codes.
	static void PushKeySample(std::vector<InputEvent>& out, const Win32KeySample& sample);

	// Emits move, button-edge and wheel events for one raw sample.
	static void PushMouseSample(std::vector<InputEvent>& out, const Win32MouseSample& sample);
};

} // namespace Modern::Client
