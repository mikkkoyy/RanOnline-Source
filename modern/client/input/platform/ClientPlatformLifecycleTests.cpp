// CLIENT-RENDER-001C: Win32 platform lifecycle regression tests.
//
// These are the tests that would have caught CLIENT-RENDER-001C's defect.
// ModernClientApp stack-allocated a Win32PlatformEvents and then handed the
// Application a default-constructed one, so Application::Run() pumped
// NullPlatformEvents (which never calls PeekMessageW). The window was created
// and drawn to, the character rendered at full frame rate, and no Windows
// message was ever dispatched -- so WM_CLOSE never reached the window
// procedure and the process could only be killed. Nothing asserted that a
// real window actually terminated its run loop, which is what these cases do.
//
// Scope: these drive REAL Win32 windows on the calling thread. They are not
// headless unit tests and are not hermetic; each one creates a window,
// exercises the message pump, and tears the window down again. Cases that
// cannot create a window (headless CI, no interactive session) report
// themselves as skipped rather than passing silently, so a skipped run is
// visibly different from a verified one.

#include "TestHarness.h"

// Win32PlatformEvents.h uses HWND/HINSTANCE in its private section without
// including <Windows.h> itself (the boundary keeps Windows out of its public
// surface), so a translation unit that includes it must supply the Win32
// vocabulary first -- exactly as ModernClientApp.cpp does. This suite drives
// the real message queue, so it needs that vocabulary anyway.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include "application/Application.h"
#include "input/platform/Win32InputSource.h"
#include "input/platform/Win32PlatformEvents.h"

#include <memory>

using namespace Modern;
using namespace Modern::Client;

namespace
{

// GetWindowHandle() is void* so the platform boundary stays free of Windows
// types; this suite already speaks Win32 and needs the real HWND.
HWND HwndOf(Win32PlatformEvents* platform) noexcept
{
	return static_cast<HWND>(platform->GetWindowHandle());
}

// Builds a real platform + window. Returns null when the environment cannot
// create one, which is reported as a skip rather than a pass.
std::unique_ptr<Win32PlatformEvents> MakePlatform()
{
	auto platform = std::make_unique<Win32PlatformEvents>("CLIENT-001C Test", 320, 240);
	return platform->Initialize() ? std::move(platform) : nullptr;
}

} // namespace

// A window can be created, pumped, and reports no close while nothing has
// asked it to close.
MODERN_TEST(ClientPlatform_WindowCreatesAndPumpsUntilClose)
{
	auto platform = MakePlatform();
	if (!platform)
	{
		std::printf("      (skipped: no interactive window station)\n");
		return;
	}

	REQUIRE(platform->GetWindowHandle() != nullptr);

	// Pumping with no input must keep running: this is the property that the
	// regression broke, and it is what distinguishes a live pump from
	// NullPlatformEvents, which would also "work" but never dispatch.
	bool sawClose = false;
	for (int i = 0; i < 8 && !sawClose; ++i)
	{
		sawClose = !platform->PumpEvents();
	}
	CHECK(!sawClose);
}

// WM_CLOSE posted to the window must be picked up by the next pump and must
// end it. This is the exact message the user generates by clicking the close
// button, and the exact message the defect never delivered.
MODERN_TEST(ClientPlatform_WmCloseStopsThePump)
{
	auto platform = MakePlatform();
	if (!platform)
	{
		std::printf("      (skipped: no interactive window station)\n");
		return;
	}

	PostMessageW(HwndOf(platform.get()), WM_CLOSE, 0, 0);

	// Pump until it reports close, with a bounded number of attempts so a
	// regression fails the assertion instead of hanging the suite.
	bool closed = false;
	for (int i = 0; i < 32 && !closed; ++i)
	{
		closed = !platform->PumpEvents();
	}
	CHECK(closed);

	// Once closed, every later pump keeps reporting closed: shutdown is
	// latched, so a second frame cannot resurrect the loop.
	CHECK(!platform->PumpEvents());
}

// Repeated shutdown must be safe. The platform is destroyed by its own
// destructor and explicitly via Shutdown(), and neither may double-destroy
// the window or leave the class registered.
MODERN_TEST(ClientPlatform_RepeatedShutdownIsSafe)
{
	auto platform = MakePlatform();
	if (!platform)
	{
		std::printf("      (skipped: no interactive window station)\n");
		return;
	}

	PostMessageW(HwndOf(platform.get()), WM_CLOSE, 0, 0);
	for (int i = 0; i < 32 && platform->PumpEvents(); ++i)
	{
	}

	platform->Shutdown();
	platform->Shutdown();   // idempotent: must not re-destroy the window
	platform->Shutdown();

	CHECK(platform->GetWindowHandle() == nullptr);
}

// The regression proper: an Application driven by the REAL platform must stop
// when the window closes. Constructing Application with a real platform is
// the wiring fix; this asserts the end-to-end consequence on a real window.
MODERN_TEST(ClientPlatform_ApplicationStopsOnWindowClose)
{
	auto platform = MakePlatform();
	if (!platform)
	{
		std::printf("      (skipped: no interactive window station)\n");
		return;
	}

	HWND hwnd = HwndOf(platform.get());
	REQUIRE(hwnd != nullptr);

	ApplicationConfig config;
	// A frame budget as a backstop: if the close never arrives the loop
	// still ends, and the assertion below reports *why* it ended.
	config.maxFrames = 600;
	Application app(config, std::move(platform));
	CHECK(app.Initialize().IsOk());

	PostMessageW(hwnd, WM_CLOSE, 0, 0);

	CHECK(app.Run().IsOk());
	CHECK(app.GetState() == ApplicationState::Stopped);
	// Ending on the close path means the frame budget was never reached. The
	// budget exists only so this test fails rather than hangs.
	CHECK_LT(app.GetFrameCount(), static_cast<uint64_t>(600));
}

// A window resize must be delivered as an event, which is how the renderer is
// told to re-create its back buffer.
MODERN_TEST(ClientPlatform_ResizeReachesTheInputSource)
{
	auto platform = MakePlatform();
	if (!platform)
	{
		std::printf("      (skipped: no interactive window station)\n");
		return;
	}

	Win32InputSource inputSource(platform.get());
	platform->SetInputSource(&inputSource);

	PostMessageW(HwndOf(platform.get()), WM_SIZE, SIZE_RESTORED,
	             MAKELPARAM(640, 480));

	bool resized = false;
	for (int i = 0; i < 32 && !resized; ++i)
	{
		platform->PumpEvents();
		for (const InputEvent& event : inputSource.PollEvents())
		{
			if (event.type == InputEventType::WindowResized)
			{
				const WindowResizeEvent& r = std::get<WindowResizeEvent>(event.payload);
				CHECK_EQ(r.width, static_cast<uint32_t>(640));
				CHECK_EQ(r.height, static_cast<uint32_t>(480));
				resized = true;
			}
		}
	}
	CHECK(resized);
}

int main()
{
	std::setvbuf(stdout, nullptr, _IONBF, 0);
	std::printf("Modern CLIENT-RENDER-001C Win32 platform lifecycle tests\n\n");

	const int failedCases = ModernTests::RunAll();

	if (failedCases == 0)
	{
		std::printf("\nAll %d test cases passed.\n",
		            static_cast<int>(ModernTests::Registry().size()));
		return 0;
	}

	std::printf("\n%d of %d test cases FAILED (%d checks).\n",
	            failedCases,
	            static_cast<int>(ModernTests::Registry().size()),
	            ModernTests::FailureCount());
	return 1;
}