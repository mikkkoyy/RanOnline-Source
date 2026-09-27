// CLIENT-002: modern client application foundation tests.
//
// Covers the Application lifecycle, the deterministic update loop and the
// dependency boundary. Links ModernClientApplication (which links Modern)
// and nothing else: no renderer, no socket, no database, no legacy library.

#include "TestHarness.h"

#include "application/Application.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

using namespace Modern;
using namespace Modern::Client;

namespace
{
	Application MakeApp(uint32_t maxFrames = 0)
	{
		ApplicationConfig config;
		config.maxFrames = maxFrames;
		return Application(config);
	}
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

MODERN_TEST(ClientApp_DefaultStateIsUninitialized)
{
	const Application app;
	CHECK_EQ(ToString(app.GetState()), std::string("Uninitialized"));
	CHECK_EQ(app.GetFrameCount(), static_cast<uint64_t>(0));
}

MODERN_TEST(ClientApp_StatesHaveNames)
{
	CHECK(std::string(ToString(ApplicationState::Uninitialized)) == "Uninitialized");
	CHECK(std::string(ToString(ApplicationState::Initialized)) == "Initialized");
	CHECK(std::string(ToString(ApplicationState::Running)) == "Running");
	CHECK(std::string(ToString(ApplicationState::Stopping)) == "Stopping");
	CHECK(std::string(ToString(ApplicationState::Stopped)) == "Stopped");
}

MODERN_TEST(ClientApp_InitializeMovesToInitialized)
{
	Application app;
	CHECK(app.Initialize().IsOk());
	CHECK(app.GetState() == ApplicationState::Initialized);
}

MODERN_TEST(ClientApp_InitializeTwiceIsInvalidState)
{
	Application app;
	CHECK(app.Initialize().IsOk());
	CHECK_EQ(app.Initialize().GetCode(), ErrorCode::InvalidState);
	CHECK(app.GetState() == ApplicationState::Initialized);
}

MODERN_TEST(ClientApp_RunBeforeInitializeIsInvalidState)
{
	Application app;
	CHECK_EQ(app.Run().GetCode(), ErrorCode::InvalidState);
	CHECK(app.GetState() == ApplicationState::Uninitialized);
	CHECK_EQ(app.GetFrameCount(), static_cast<uint64_t>(0));
}

MODERN_TEST(ClientApp_ShutdownBeforeInitializeIsInvalidState)
{
	Application app;
	CHECK_EQ(app.Shutdown().GetCode(), ErrorCode::InvalidState);
	CHECK(app.GetState() == ApplicationState::Uninitialized);
}

MODERN_TEST(ClientApp_ShutdownAfterInitializeStops)
{
	Application app;
	CHECK(app.Initialize().IsOk());
	CHECK(app.Shutdown().IsOk());
	CHECK(app.GetState() == ApplicationState::Stopped);
}

MODERN_TEST(ClientApp_ShutdownTwiceIsNotAllowed)
{
	Application app;
	CHECK(app.Initialize().IsOk());
	CHECK(app.Shutdown().IsOk());
	CHECK_EQ(app.Shutdown().GetCode(), ErrorCode::NotAllowed);
}

MODERN_TEST(ClientApp_StoppedIsTerminal)
{
	Application app = MakeApp(1);
	CHECK(app.Initialize().IsOk());
	CHECK(app.Run().IsOk());
	CHECK(app.GetState() == ApplicationState::Stopped);

	// Restarting requires a new Application, matching the core convention
	// where Destroyed is terminal until Reset.
	CHECK_EQ(app.Initialize().GetCode(), ErrorCode::NotAllowed);
	CHECK_EQ(app.Run().GetCode(), ErrorCode::NotAllowed);
	CHECK_EQ(app.Shutdown().GetCode(), ErrorCode::NotAllowed);
	CHECK_EQ(app.RequestStop().GetCode(), ErrorCode::NotAllowed);
}

MODERN_TEST(ClientApp_RequestStopOutsideRunningIsRejected)
{
	Application app;
	CHECK_EQ(app.RequestStop().GetCode(), ErrorCode::InvalidState);

	CHECK(app.Initialize().IsOk());
	CHECK_EQ(app.RequestStop().GetCode(), ErrorCode::InvalidState);
}

// ---------------------------------------------------------------------------
// Update loop
// ---------------------------------------------------------------------------

MODERN_TEST(ClientApp_ShutdownWhileRunningIsInvalidState)
{
	Application app = MakeApp(3);
	CHECK(app.Initialize().IsOk());
	app.SetUpdateCallback([&](uint64_t) { CHECK_EQ(app.Shutdown().GetCode(), ErrorCode::InvalidState); });
	CHECK(app.Run().IsOk());
	CHECK(app.GetState() == ApplicationState::Stopped);
}

MODERN_TEST(ClientApp_RunDrainsFrameBudgetAndStops)
{
	Application app = MakeApp(3);
	CHECK(app.Initialize().IsOk());
	CHECK(app.Run().IsOk());
	CHECK(app.GetState() == ApplicationState::Stopped);
	CHECK_EQ(app.GetFrameCount(), static_cast<uint64_t>(3));
}

MODERN_TEST(ClientApp_UpdateCallbackRunsInOrder)
{
	Application app = MakeApp(4);
	std::vector<uint64_t> seen;
	app.SetUpdateCallback([&](uint64_t frame) { seen.push_back(frame); });

	CHECK(app.Initialize().IsOk());
	CHECK(app.Run().IsOk());

	CHECK_EQ(seen.size(), static_cast<size_t>(4));
	for (size_t i = 0; i < seen.size(); ++i)
	{
		CHECK_EQ(seen[i], static_cast<uint64_t>(i + 1));
	}
}

MODERN_TEST(ClientApp_RequestStopExitsLoopEarly)
{
	Application app = MakeApp(100);
	app.SetUpdateCallback([&](uint64_t frame)
	{
		if (frame == 2)
		{
			CHECK(app.RequestStop().IsOk());
			CHECK(app.GetState() == ApplicationState::Stopping);
		}
	});

	CHECK(app.Initialize().IsOk());
	CHECK(app.Run().IsOk());
	CHECK(app.GetState() == ApplicationState::Stopped);
	CHECK_EQ(app.GetFrameCount(), static_cast<uint64_t>(2));
}

MODERN_TEST(ClientApp_SecondRequestStopIsRejected)
{
	Application app = MakeApp(100);
	app.SetUpdateCallback([&](uint64_t frame)
	{
		if (frame == 1)
		{
			CHECK(app.RequestStop().IsOk());
			CHECK_EQ(app.RequestStop().GetCode(), ErrorCode::InvalidState);
		}
	});

	CHECK(app.Initialize().IsOk());
	CHECK(app.Run().IsOk());
	CHECK_EQ(app.GetFrameCount(), static_cast<uint64_t>(1));
}

MODERN_TEST(ClientApp_PlatformCloseEndsLoopCleanly)
{
	struct CloseAfterTwo final : IPlatformEvents
	{
		int pumps = 0;
		bool PumpEvents() override { return ++pumps <= 2; }
	};

	Application app(ApplicationConfig{}, std::make_unique<CloseAfterTwo>());
	CHECK(app.Initialize().IsOk());
	CHECK(app.Run().IsOk());
	CHECK(app.GetState() == ApplicationState::Stopped);
	CHECK_EQ(app.GetFrameCount(), static_cast<uint64_t>(2));
}

// ---------------------------------------------------------------------------

int main()
{
	std::printf("Modern CLIENT-002 application foundation tests\n\n");

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


