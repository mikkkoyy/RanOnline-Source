#pragma once

// CLIENT-003: deterministic platform event source for tests and the
// headless emulator.
//
// FakeInputSource feeds a scripted InputEvent list into an InputSystem one
// frame at a time: PumpFrame pushes the next script entry (EndFrame is NOT
// called, so the test can inspect the queue), DrainAll pushes everything
// for single-frame tests. CloseRequested mirrors a platform close request
// (WindowClosed).
//
// ScriptedInputSource implements IInputSource on top of a vector of events
// (one event per frame), allowing Application to run deterministic headless
// input loops.

#include "InputSystem.h"
#include "application/Application.h"

#include <cstddef>
#include <vector>

namespace Modern::Client
{

class FakeInputSource
{
public:
	FakeInputSource() = default;
	explicit FakeInputSource(std::vector<InputEvent> script);

	void SetScript(std::vector<InputEvent> script);
	void Reset();

	// Pushes script[next] into input. Returns true while script remains.
	// WindowClosed events additionally raise CloseRequested.
	Status PumpFrame(InputSystem& input);

	// Pushes the whole remaining script (multiple events, one frame).
	Status DrainAll(InputSystem& input);

	bool CloseRequested() const noexcept { return m_closeRequested; }
	size_t GetPosition() const noexcept { return m_position; }
	size_t GetScriptSize() const noexcept { return m_script.size(); }

private:
	std::vector<InputEvent> m_script;
	size_t                  m_position       = 0;
	bool                    m_closeRequested = false;
};

// Headless IInputSource delivering one script entry per frame to Application.
class ScriptedInputSource final : public IInputSource
{
public:
	ScriptedInputSource() = default;
	explicit ScriptedInputSource(std::vector<InputEvent> script)
		: m_fake(std::move(script))
	{
	}

	std::vector<InputEvent> PollEvents() override
	{
		InputSystem sink;
		sink.Initialize();
		if (m_fake.PumpFrame(sink).IsError())
		{
			return {};
		}

		std::vector<InputEvent> events;
		sink.ForEachEvent([&](const InputEvent& event) { events.push_back(event); });
		return events;
	}

	bool RequestsClose() const override { return m_fake.CloseRequested(); }

	FakeInputSource& GetFakeSource() noexcept { return m_fake; }
	const FakeInputSource& GetFakeSource() const noexcept { return m_fake; }

private:
	FakeInputSource m_fake;
};

} // namespace Modern::Client
