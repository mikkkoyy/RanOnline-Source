// CLIENT-003: modern client input boundary tests.
//
// Covers keyboard/mouse/window events, FIFO queue behavior, current state,
// the Win32 translation boundary, and Application integration. Links
// ModernClientApplication (which links ModernClientInput and Modern) and
// nothing else: no real Win32 window, no DirectX, no legacy library.
// Deterministic scripted sources stand in for the platform.

#include "TestHarness.h"

#include "application/Application.h"
#include "input/FakeInputSource.h"
#include "input/InputSystem.h"
#include "input/platform/Win32InputAdapter.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

using namespace Modern;
using namespace Modern::Client;


namespace
{
	InputSystem MakeInput()
	{
		InputSystem input;
		if (!input.Initialize().IsOk())
		{
			CHECK(false);
		}
		return input;
	}

	// IInputSource fed from a FakeInputSource: one script entry per frame.
	class ScriptSource final : public IInputSource
	{
	public:
		explicit ScriptSource(std::vector<InputEvent> script)
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

	private:
		FakeInputSource m_fake;
	};
}

// ---------------------------------------------------------------------------
// Keyboard
// ---------------------------------------------------------------------------

MODERN_TEST(Input_KeyPressedAndReleased)
{
	InputSystem input = MakeInput();

	CHECK(input.PushEvent(InputEvent::MakeKey(KeyCode::W, true)).IsOk());
	CHECK(input.IsKeyDown(KeyCode::W));
	CHECK_EQ(input.GetEventCount(), static_cast<size_t>(1));

	CHECK(input.PushEvent(InputEvent::MakeKey(KeyCode::W, false)).IsOk());
	CHECK(!input.IsKeyDown(KeyCode::W));
	CHECK_EQ(input.GetEventCount(), static_cast<size_t>(2));
}

MODERN_TEST(Input_RepeatKeyEventsPreserveOrder)
{
	InputSystem input = MakeInput();

	CHECK(input.PushEvent(InputEvent::MakeKey(KeyCode::A, true, false)).IsOk());
	CHECK(input.PushEvent(InputEvent::MakeKey(KeyCode::A, true, true)).IsOk());
	CHECK(input.PushEvent(InputEvent::MakeKey(KeyCode::A, true, true)).IsOk());

	CHECK_EQ(input.GetEventCount(), static_cast<size_t>(3));
	CHECK(!std::get<KeyEvent>(input.GetEvent(0).payload).repeated);
	CHECK(std::get<KeyEvent>(input.GetEvent(1).payload).repeated);
	CHECK(std::get<KeyEvent>(input.GetEvent(2).payload).repeated);
	CHECK(input.IsKeyDown(KeyCode::A));
}

MODERN_TEST(Input_MultipleKeysTrackIndependently)
{
	InputSystem input = MakeInput();

	CHECK(input.PushEvent(InputEvent::MakeKey(KeyCode::W, true)).IsOk());
	CHECK(input.PushEvent(InputEvent::MakeKey(KeyCode::LeftShift, true)).IsOk());
	CHECK(input.IsKeyDown(KeyCode::W));
	CHECK(input.IsKeyDown(KeyCode::LeftShift));

	CHECK(input.PushEvent(InputEvent::MakeKey(KeyCode::W, false)).IsOk());
	CHECK(!input.IsKeyDown(KeyCode::W));
	CHECK(input.IsKeyDown(KeyCode::LeftShift));
}

// ---------------------------------------------------------------------------
// Mouse
// ---------------------------------------------------------------------------

MODERN_TEST(Input_MouseMoveUpdatesPosition)
{
	InputSystem input = MakeInput();

	CHECK(input.PushEvent(InputEvent::MakeMouseMove(120, 80)).IsOk());
	CHECK_EQ(input.GetMouseX(), 120);
	CHECK_EQ(input.GetMouseY(), 80);
	CHECK_EQ(input.GetEventCount(), static_cast<size_t>(1));
}

MODERN_TEST(Input_MouseButtonPressAndRelease)
{
	InputSystem input = MakeInput();

	CHECK(input.PushEvent(InputEvent::MakeMouseButton(MouseButton::Left, true, 10, 20)).IsOk());
	CHECK(input.IsMouseButtonDown(MouseButton::Left));
	CHECK_EQ(input.GetMouseX(), 10);
	CHECK_EQ(input.GetMouseY(), 20);

	CHECK(input.PushEvent(InputEvent::MakeMouseButton(MouseButton::Left, false, 10, 20)).IsOk());
	CHECK(!input.IsMouseButtonDown(MouseButton::Left));
}

MODERN_TEST(Input_MouseWheelAccumulatesWithinFrame)
{
	InputSystem input = MakeInput();

	CHECK(input.PushEvent(InputEvent::MakeMouseWheel(0, 120)).IsOk());
	CHECK(input.PushEvent(InputEvent::MakeMouseWheel(0, 120)).IsOk());
	CHECK_EQ(input.GetMouseWheelDeltaY(), 240);
	CHECK_EQ(input.GetEventCount(), static_cast<size_t>(2));
}

MODERN_TEST(Input_MultipleMouseButtonsTrackIndependently)
{
	InputSystem input = MakeInput();

	CHECK(input.PushEvent(InputEvent::MakeMouseButton(MouseButton::Left, true, 0, 0)).IsOk());
	CHECK(input.PushEvent(InputEvent::MakeMouseButton(MouseButton::Right, true, 0, 0)).IsOk());
	CHECK(input.IsMouseButtonDown(MouseButton::Left));
	CHECK(input.IsMouseButtonDown(MouseButton::Right));

	CHECK(input.PushEvent(InputEvent::MakeMouseButton(MouseButton::Left, false, 0, 0)).IsOk());
	CHECK(!input.IsMouseButtonDown(MouseButton::Left));
	CHECK(input.IsMouseButtonDown(MouseButton::Right));
}

// ---------------------------------------------------------------------------
// Window
// ---------------------------------------------------------------------------

MODERN_TEST(Input_WindowCloseQueuesEvent)
{
	InputSystem input = MakeInput();

	CHECK(input.PushEvent(InputEvent::MakeWindowClosed()).IsOk());
	CHECK_EQ(input.GetEventCount(), static_cast<size_t>(1));
	CHECK(input.GetEvent(0).type == InputEventType::WindowClosed);
}

MODERN_TEST(Input_WindowResizeQueuesEvent)
{
	InputSystem input = MakeInput();

	CHECK(input.PushEvent(InputEvent::MakeWindowResized(1280, 720)).IsOk());
	const InputEvent& event = input.GetEvent(0);
	CHECK(event.type == InputEventType::WindowResized);
	CHECK_EQ(std::get<WindowResizeEvent>(event.payload).width, static_cast<uint32_t>(1280));
	CHECK_EQ(std::get<WindowResizeEvent>(event.payload).height, static_cast<uint32_t>(720));
}

MODERN_TEST(Input_DegenerateResizeIsRejected)
{
	InputSystem input = MakeInput();

	CHECK_EQ(input.PushEvent(InputEvent::MakeWindowResized(0, 720)).GetCode(),
		ErrorCode::InvalidArgument);
	CHECK(input.IsQueueEmpty());
}

// ---------------------------------------------------------------------------
// Queue
// ---------------------------------------------------------------------------

MODERN_TEST(Input_QueuePreservesFifoOrder)
{
	InputSystem input = MakeInput();

	CHECK(input.PushEvent(InputEvent::MakeKey(KeyCode::A, true)).IsOk());
	CHECK(input.PushEvent(InputEvent::MakeMouseMove(5, 6)).IsOk());
	CHECK(input.PushEvent(InputEvent::MakeWindowResized(800, 600)).IsOk());

	CHECK_EQ(input.GetEventCount(), static_cast<size_t>(3));
	CHECK(input.GetEvent(0).type == InputEventType::Key);
	CHECK(input.GetEvent(1).type == InputEventType::MouseMove);
	CHECK(input.GetEvent(2).type == InputEventType::WindowResized);
}

MODERN_TEST(Input_ForEachEventVisitsInOrder)
{
	InputSystem input = MakeInput();

	CHECK(input.PushEvent(InputEvent::MakeKey(KeyCode::A, true)).IsOk());
	CHECK(input.PushEvent(InputEvent::MakeKey(KeyCode::B, true)).IsOk());

	std::vector<KeyCode> seen;
	input.ForEachEvent([&](const InputEvent& event) {
		seen.push_back(std::get<KeyEvent>(event.payload).code);
	});

	CHECK_EQ(seen.size(), static_cast<size_t>(2));
	CHECK(seen[0] == KeyCode::A);
	CHECK(seen[1] == KeyCode::B);
}

MODERN_TEST(Input_EndFrameClearsQueueButKeepsHeldState)
{
	InputSystem input = MakeInput();

	CHECK(input.PushEvent(InputEvent::MakeKey(KeyCode::W, true)).IsOk());
	CHECK(input.PushEvent(InputEvent::MakeMouseWheel(0, 120)).IsOk());
	CHECK_EQ(input.GetEventCount(), static_cast<size_t>(2));

	input.EndFrame();

	CHECK(input.IsQueueEmpty());
	CHECK_EQ(input.GetEventCount(), static_cast<size_t>(0));
	CHECK_EQ(input.GetMouseWheelDeltaY(), 0);
	CHECK(input.IsKeyDown(KeyCode::W));
}

MODERN_TEST(Input_EventsDoNotLeakIntoNextFrame)
{
	InputSystem input = MakeInput();

	CHECK(input.PushEvent(InputEvent::MakeKey(KeyCode::A, true)).IsOk());
	input.EndFrame();
	CHECK(input.PushEvent(InputEvent::MakeKey(KeyCode::B, true)).IsOk());

	CHECK_EQ(input.GetEventCount(), static_cast<size_t>(1));
	CHECK(std::get<KeyEvent>(input.GetEvent(0).payload).code == KeyCode::B);
}

MODERN_TEST(Input_ResetClearsStateAndQueue)
{
	InputSystem input = MakeInput();

	CHECK(input.PushEvent(InputEvent::MakeKey(KeyCode::W, true)).IsOk());
	CHECK(input.PushEvent(InputEvent::MakeMouseMove(9, 9)).IsOk());
	input.Reset();

	CHECK(input.IsQueueEmpty());
	CHECK(!input.IsKeyDown(KeyCode::W));
	CHECK_EQ(input.GetMouseX(), 0);
	CHECK(!input.IsInitialized());
	CHECK_EQ(input.Initialize().GetCode(), ErrorCode::None);
}

MODERN_TEST(Input_UnknownKeyIsRejected)
{
	InputSystem input = MakeInput();

	CHECK_EQ(input.PushEvent(InputEvent::MakeKey(KeyCode::Unknown, true)).GetCode(),
		ErrorCode::InvalidArgument);
	CHECK(input.IsQueueEmpty());
	CHECK(!input.IsKeyDown(KeyCode::Unknown));
}

MODERN_TEST(Input_UnknownMouseButtonIsRejected)
{
	InputSystem input = MakeInput();

	CHECK_EQ(input.PushEvent(InputEvent::MakeMouseButton(MouseButton::Unknown, true, 0, 0)).GetCode(),
		ErrorCode::InvalidArgument);
	CHECK(input.IsQueueEmpty());
}

MODERN_TEST(Input_PushBeforeInitializeIsInvalidState)
{
	InputSystem input;
	CHECK_EQ(input.PushEvent(InputEvent::MakeKey(KeyCode::W, true)).GetCode(),
		ErrorCode::InvalidState);
}

// ---------------------------------------------------------------------------
// Platform adapter (no window, no <Windows.h>)
// ---------------------------------------------------------------------------

MODERN_TEST(Input_Win32LettersAndDigitsTranslate)
{
	CHECK(Win32InputAdapter::TranslateKey(0x41) == KeyCode::A);
	CHECK(Win32InputAdapter::TranslateKey(0x5A) == KeyCode::Z);
	CHECK(Win32InputAdapter::TranslateKey(0x30) == KeyCode::Zero);
	CHECK(Win32InputAdapter::TranslateKey(0x39) == KeyCode::Nine);
	CHECK(Win32InputAdapter::TranslateKey(0x1B) == KeyCode::Escape);
	CHECK(Win32InputAdapter::TranslateKey(0x70) == KeyCode::F1);
	CHECK(Win32InputAdapter::TranslateKey(0x7B) == KeyCode::F12);
	CHECK(Win32InputAdapter::TranslateKey(0x25) == KeyCode::Left);
	CHECK(Win32InputAdapter::TranslateKey(0xA0) == KeyCode::LeftShift);
	CHECK(Win32InputAdapter::TranslateKey(0xA1) == KeyCode::RightShift);
	CHECK(Win32InputAdapter::TranslateKey(0xBA) == KeyCode::Semicolon);
	CHECK(Win32InputAdapter::TranslateKey(0x5B) == KeyCode::Unknown);
	CHECK(Win32InputAdapter::TranslateKey(0xFF) == KeyCode::Unknown);
}

MODERN_TEST(Input_Win32UnknownKeyEmitsNothing)
{
	std::vector<InputEvent> events;
	Win32InputAdapter::PushKeySample(events, Win32KeySample{ 0x5B, true, false });
	CHECK(events.empty());
}

MODERN_TEST(Input_Win32KeySampleEmitsKeyEvent)
{
	std::vector<InputEvent> events;
	Win32InputAdapter::PushKeySample(events, Win32KeySample{ 0x1B, true, false });

	CHECK_EQ(events.size(), static_cast<size_t>(1));
	CHECK(events[0].type == InputEventType::Key);
	CHECK(std::get<KeyEvent>(events[0].payload).code == KeyCode::Escape);
}

MODERN_TEST(Input_Win32MouseSampleEmitsOrderedEvents)
{
	Win32MouseSample sample;
	sample.x             = 40;
	sample.y             = 50;
	sample.moved         = true;
	sample.leftChanged   = true;
	sample.leftPressed   = true;
	sample.wheelDelta    = 120;

	std::vector<InputEvent> events;
	Win32InputAdapter::PushMouseSample(events, sample);

	CHECK_EQ(events.size(), static_cast<size_t>(3));
	CHECK(events[0].type == InputEventType::MouseMove);
	CHECK(events[1].type == InputEventType::MouseButton);
	CHECK(events[2].type == InputEventType::MouseWheel);
	CHECK(std::get<MouseMoveEvent>(events[0].payload).x == 40);
	CHECK(std::get<MouseWheelEvent>(events[2].payload).deltaY == 120);
}

// ---------------------------------------------------------------------------
// Application integration
// ---------------------------------------------------------------------------

MODERN_TEST(Input_ApplicationDispatchesScriptedFrame)
{
	InputSystem input = MakeInput();

	ApplicationConfig config;
	config.maxFrames = 3;
	Application app(config);
	app.SetInputSystem(&input);

	std::vector<InputEvent> script;
	script.push_back(InputEvent::MakeKey(KeyCode::W, true));
	script.push_back(InputEvent::MakeMouseMove(7, 8));
	script.push_back(InputEvent::MakeKey(KeyCode::W, false));
	app.SetInputSource(std::make_unique<ScriptSource>(std::move(script)));

	std::vector<InputEventType> seen;
	app.SubscribeInput([&](const InputEvent& event) { seen.push_back(event.type); });

	CHECK(app.Initialize().IsOk());
	CHECK(app.Run().IsOk());
	CHECK(app.GetState() == ApplicationState::Stopped);
	CHECK_EQ(app.GetFrameCount(), static_cast<uint64_t>(3));

	CHECK_EQ(seen.size(), static_cast<size_t>(3));
	CHECK(seen[0] == InputEventType::Key);
	CHECK(seen[1] == InputEventType::MouseMove);
	CHECK(seen[2] == InputEventType::Key);

	// Queue is drained every frame: nothing leaks past Run().
	CHECK(input.IsQueueEmpty());
	CHECK(!input.IsKeyDown(KeyCode::W));
	CHECK_EQ(input.GetMouseX(), 7);
}

MODERN_TEST(Input_ApplicationCloseEventEndsLoop)
{
	InputSystem input = MakeInput();

	Application app;
	app.SetInputSystem(&input);

	std::vector<InputEvent> script;
	script.push_back(InputEvent::MakeKey(KeyCode::A, true));
	script.push_back(InputEvent::MakeWindowClosed());
	script.push_back(InputEvent::MakeKey(KeyCode::B, true));
	app.SetInputSource(std::make_unique<ScriptSource>(std::move(script)));

	size_t seen = 0;
	app.SubscribeInput([&](const InputEvent&) { ++seen; });

	CHECK(app.Initialize().IsOk());
	CHECK(app.Run().IsOk());
	CHECK_EQ(app.GetFrameCount(), static_cast<uint64_t>(2));
	CHECK_EQ(seen, static_cast<size_t>(2));
	CHECK(input.IsQueueEmpty());
}

MODERN_TEST(Input_ApplicationWithoutInputSystemDropsEvents)
{
	ApplicationConfig config;
	config.maxFrames = 2;
	Application app(config);

	std::vector<InputEvent> script;
	script.push_back(InputEvent::MakeKey(KeyCode::W, true));
	app.SetInputSource(std::make_unique<ScriptSource>(std::move(script)));

	size_t seen = 0;
	app.SubscribeInput([&](const InputEvent&) { ++seen; });

	CHECK(app.Initialize().IsOk());
	CHECK(app.Run().IsOk());
	CHECK_EQ(seen, static_cast<size_t>(0));
}

// ---------------------------------------------------------------------------

int main()

{
	std::printf("Modern CLIENT-003 input boundary tests\n\n");

	const int failedCases = ModernTests::RunAll();

	if (failedCases == 0)
	{
		std::printf("\nAll %d test cases passed.\n", static_cast<int>(ModernTests::Registry().size()));
		return 0;
	}

	std::printf("\n%d of %d test cases FAILED (%d checks).\n",
		failedCases,
		static_cast<int>(ModernTests::Registry().size()),
		ModernTests::FailureCount());
	return 1;
}
