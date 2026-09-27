#pragma once

// CLIENT-003: per-frame input queue plus current input state.
//
// EVENT and STATE are separate on purpose, mirroring the legacy split the
// audit found: UIKeyCheck::Check consumes a key event once while
// UIKeyCheck::CheckSimple polls the raw DirectInput state, and
// CUIMessageQueue is a FIFO Post/Peek queue (see CLIENT-003 source notes in
// InputEvents.h).
//
// Queue: FIFO, drained by Application at the end of every frame, so events
// never leak into the next frame unless a consumer re-posts them.
// State: held-down keys/buttons, cursor position and this frame's wheel
// delta. Ends cleanly: PushEvent validates (Unknown codes and degenerate
// resizes are InvalidArgument) instead of queueing garbage.

#include "InputEvents.h"
#include "types/Result.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>

namespace Modern::Client
{

class InputSystem
{
public:
	InputSystem() = default;

	InputSystem(const InputSystem&)            = delete;
	InputSystem& operator=(const InputSystem&) = delete;
	InputSystem(InputSystem&&)                 = default;
	InputSystem& operator=(InputSystem&&)      = default;
	~InputSystem()                             = default;

	// Queues one event and folds it into current state. Only valid after
	// Initialize: pushing into an uninitialized system is InvalidState, so
	// a mis-wired platform adapter fails loudly instead of dropping input.
	Status PushEvent(const InputEvent& event);

	// FIFO access to this frame's events, in arrival order.
	size_t            GetEventCount() const noexcept { return m_queue.size(); }
	bool              IsQueueEmpty() const noexcept { return m_queue.empty(); }
	const InputEvent& GetEvent(size_t index) const { return m_queue.at(index); }

	// Runs fn over each queued event in FIFO order. Used by Application to
	// dispatch to subscribers; a plain index loop keeps ordering obvious.
	void ForEachEvent(const std::function<void(const InputEvent&)>& fn) const;

	Status Initialize();
	void   Reset();

	// Ends the frame: drops every queued event and zeroes the wheel delta,
	// which is per-frame like the legacy dz field. Held keys, held buttons
	// and the cursor position persist — they are state, not events.
	void EndFrame();

	// --- Current state (polling) ---

	bool IsKeyDown(KeyCode code) const;
	bool IsMouseButtonDown(MouseButton button) const;

	int32_t GetMouseX() const noexcept { return m_mouseX; }
	int32_t GetMouseY() const noexcept { return m_mouseY; }
	int32_t GetMouseWheelDeltaX() const noexcept { return m_wheelDeltaX; }
	int32_t GetMouseWheelDeltaY() const noexcept { return m_wheelDeltaY; }

	bool IsInitialized() const noexcept { return m_initialized; }

private:
	static constexpr size_t kKeyCount    = static_cast<size_t>(KeyCode::Count);
	static constexpr size_t kButtonCount = static_cast<size_t>(MouseButton::Count);

	static bool IsValidKey(KeyCode code) noexcept
	{
		const size_t index = static_cast<size_t>(code);
		return index > static_cast<size_t>(KeyCode::Unknown) && index < kKeyCount;
	}

	static bool IsValidButton(MouseButton button) noexcept
	{
		const size_t index = static_cast<size_t>(button);
		return index > static_cast<size_t>(MouseButton::Unknown) && index < kButtonCount;
	}

	bool                                   m_initialized = false;
	std::deque<InputEvent>                 m_queue;
	std::array<bool, kKeyCount>            m_keysDown{};
	std::array<bool, kButtonCount>         m_buttonsDown{};
	int32_t                                m_mouseX      = 0;
	int32_t                                m_mouseY      = 0;
	int32_t                                m_wheelDeltaX = 0;
	int32_t                                m_wheelDeltaY = 0;
};

} // namespace Modern::Client
