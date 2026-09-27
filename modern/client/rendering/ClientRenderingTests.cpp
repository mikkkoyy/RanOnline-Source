// CLIENT-004: modern client rendering boundary tests.
//
// Covers:
// - Renderer lifecycle: Uninitialized -> Initialized -> InFrame -> Shutdown
// - NullRenderer deterministic behavior and frame counting
// - Clear operation constraints
// - Resize validation and state preservation
// - Application + Input + NullRenderer integration
// - Resize event propagation through Application to Renderer
// - Header isolation (no Windows.h, DirectX, Vulkan, or legacy headers)

#include "TestHarness.h"

#include "application/Application.h"
#include "input/FakeInputSource.h"
#include "input/InputEvents.h"
#include "input/InputSystem.h"
#include "rendering/NullRenderer.h"
#include "rendering/Renderer.h"
#include "rendering/RenderingTypes.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

using namespace Modern;
using namespace Modern::Client;

// ---------------------------------------------------------------------------
// Rendering Types and Defaults
// ---------------------------------------------------------------------------

MODERN_TEST(ClientRendering_DefaultConfigIsValid)
{
	RendererConfig config;
	CHECK(config.IsValid());
	CHECK_EQ(config.width, 1280u);
	CHECK_EQ(config.height, 720u);
	CHECK_EQ(config.vsync, true);
	CHECK_EQ(config.displayMode, DisplayMode::Windowed);
}

MODERN_TEST(ClientRendering_ConfigValidationZeroDimensions)
{
	RendererConfig zeroWidth{ 0, 720, DisplayMode::Windowed, true };
	CHECK(!zeroWidth.IsValid());

	RendererConfig zeroHeight{ 1280, 0, DisplayMode::Windowed, true };
	CHECK(!zeroHeight.IsValid());

	RendererConfig bothZero{ 0, 0, DisplayMode::Windowed, true };
	CHECK(!bothZero.IsValid());
}

MODERN_TEST(ClientRendering_StateAndModeStrings)
{
	CHECK_EQ(std::string(ToString(RendererState::Uninitialized)), "Uninitialized");
	CHECK_EQ(std::string(ToString(RendererState::Initialized)), "Initialized");
	CHECK_EQ(std::string(ToString(RendererState::InFrame)), "InFrame");
	CHECK_EQ(std::string(ToString(RendererState::ShutdownState)), "Shutdown");

	CHECK_EQ(std::string(ToString(DisplayMode::Windowed)), "Windowed");
	CHECK_EQ(std::string(ToString(DisplayMode::Fullscreen)), "Fullscreen");
	CHECK_EQ(std::string(ToString(DisplayMode::BorderlessFullscreen)), "BorderlessFullscreen");
}

MODERN_TEST(ClientRendering_ColorConstants)
{
	CHECK_EQ(RenderColor::Black.r, 0.0f);
	CHECK_EQ(RenderColor::Black.g, 0.0f);
	CHECK_EQ(RenderColor::Black.b, 0.0f);
	CHECK_EQ(RenderColor::Black.a, 1.0f);

	CHECK_EQ(RenderColor::White.r, 1.0f);
	CHECK_EQ(RenderColor::White.g, 1.0f);
	CHECK_EQ(RenderColor::White.b, 1.0f);
	CHECK_EQ(RenderColor::White.a, 1.0f);
}

// ---------------------------------------------------------------------------
// NullRenderer Lifecycle
// ---------------------------------------------------------------------------

MODERN_TEST(ClientRendering_InitialStateIsUninitialized)
{
	NullRenderer renderer;
	CHECK_EQ(renderer.GetState(), RendererState::Uninitialized);
	CHECK(!renderer.IsInitialized());
	CHECK_EQ(renderer.GetRenderedFrameCount(), 0u);
	CHECK_EQ(renderer.GetClearCount(), 0u);
}

MODERN_TEST(ClientRendering_InitializeSucceeds)
{
	NullRenderer renderer;
	RendererConfig config{ 1920, 1080, DisplayMode::Fullscreen, false };
	Status status = renderer.Initialize(config);

	CHECK(status.IsOk());
	CHECK_EQ(renderer.GetState(), RendererState::Initialized);
	CHECK(renderer.IsInitialized());
	CHECK_EQ(renderer.GetWidth(), 1920u);
	CHECK_EQ(renderer.GetHeight(), 1080u);
	CHECK_EQ(renderer.GetConfig().vsync, false);
}

MODERN_TEST(ClientRendering_DoubleInitializeRejected)
{
	NullRenderer renderer;
	RendererConfig config;
	CHECK(renderer.Initialize(config).IsOk());

	Status second = renderer.Initialize(config);
	CHECK(!second.IsOk());
	CHECK_EQ(second.GetCode(), ErrorCode::InvalidState);
}

MODERN_TEST(ClientRendering_InitializeInvalidConfigRejected)
{
	NullRenderer renderer;
	RendererConfig invalidConfig{ 0, 720, DisplayMode::Windowed, true };
	Status status = renderer.Initialize(invalidConfig);

	CHECK(!status.IsOk());
	CHECK_EQ(status.GetCode(), ErrorCode::InvalidArgument);
	CHECK_EQ(renderer.GetState(), RendererState::Uninitialized);
}

MODERN_TEST(ClientRendering_BeginFrameBeforeInitializeRejected)
{
	NullRenderer renderer;
	Status status = renderer.BeginFrame();
	CHECK(!status.IsOk());
	CHECK_EQ(status.GetCode(), ErrorCode::InvalidState);
}

MODERN_TEST(ClientRendering_EndFrameBeforeBeginFrameRejected)
{
	NullRenderer renderer;
	RendererConfig config;
	renderer.Initialize(config);

	Status status = renderer.EndFrame();
	CHECK(!status.IsOk());
	CHECK_EQ(status.GetCode(), ErrorCode::InvalidState);
}

MODERN_TEST(ClientRendering_FrameSequenceDeterministic)
{
	NullRenderer renderer;
	RendererConfig config;
	CHECK(renderer.Initialize(config).IsOk());

	for (uint32_t i = 1; i <= 5; ++i)
	{
		CHECK(renderer.BeginFrame().IsOk());
		CHECK_EQ(renderer.GetState(), RendererState::InFrame);

		// Double BeginFrame while in frame must fail
		CHECK_EQ(renderer.BeginFrame().GetCode(), ErrorCode::InvalidState);

		// Clear during frame succeeds
		CHECK(renderer.Clear(RenderColor::ClearCornflowerBlue).IsOk());

		CHECK(renderer.EndFrame().IsOk());
		CHECK_EQ(renderer.GetState(), RendererState::Initialized);
		CHECK_EQ(renderer.GetRenderedFrameCount(), static_cast<uint64_t>(i));
	}

	CHECK_EQ(renderer.GetClearCount(), 5u);
	CHECK_EQ(renderer.GetLastClearColor().r, RenderColor::ClearCornflowerBlue.r);
}

MODERN_TEST(ClientRendering_ClearOutsideFrameRejected)
{
	NullRenderer renderer;
	RendererConfig config;
	renderer.Initialize(config);

	// In Initialized state, Clear is invalid
	Status status = renderer.Clear(RenderColor::Black);
	CHECK(!status.IsOk());
	CHECK_EQ(status.GetCode(), ErrorCode::InvalidState);
}

MODERN_TEST(ClientRendering_ShutdownLifecycle)
{
	NullRenderer renderer;
	RendererConfig config;
	renderer.Initialize(config);

	renderer.Shutdown();
	CHECK_EQ(renderer.GetState(), RendererState::ShutdownState);
	CHECK(!renderer.IsInitialized());

	// Double shutdown is safe
	renderer.Shutdown();
	CHECK_EQ(renderer.GetState(), RendererState::ShutdownState);

	// Operations after shutdown return NotAllowed
	CHECK_EQ(renderer.Initialize(config).GetCode(), ErrorCode::NotAllowed);
	CHECK_EQ(renderer.BeginFrame().GetCode(), ErrorCode::NotAllowed);
	CHECK_EQ(renderer.EndFrame().GetCode(), ErrorCode::NotAllowed);
	CHECK_EQ(renderer.Clear(RenderColor::Black).GetCode(), ErrorCode::NotAllowed);
	CHECK_EQ(renderer.Resize(800, 600).GetCode(), ErrorCode::NotAllowed);
}

// ---------------------------------------------------------------------------
// Resize Validation
// ---------------------------------------------------------------------------

MODERN_TEST(ClientRendering_ResizeValid)
{
	NullRenderer renderer;
	RendererConfig config{ 1280, 720, DisplayMode::Windowed, true };
	renderer.Initialize(config);

	Status status = renderer.Resize(1920, 1080);
	CHECK(status.IsOk());
	CHECK_EQ(renderer.GetWidth(), 1920u);
	CHECK_EQ(renderer.GetHeight(), 1080u);
	CHECK_EQ(renderer.GetState(), RendererState::Initialized);
}

MODERN_TEST(ClientRendering_ResizeInvalidDimensions)
{
	NullRenderer renderer;
	RendererConfig config{ 1280, 720, DisplayMode::Windowed, true };
	renderer.Initialize(config);

	CHECK_EQ(renderer.Resize(0, 720).GetCode(), ErrorCode::InvalidArgument);
	CHECK_EQ(renderer.Resize(1280, 0).GetCode(), ErrorCode::InvalidArgument);
	CHECK_EQ(renderer.Resize(0, 0).GetCode(), ErrorCode::InvalidArgument);

	// Dimensions must remain unchanged after rejected resize
	CHECK_EQ(renderer.GetWidth(), 1280u);
	CHECK_EQ(renderer.GetHeight(), 720u);
}

MODERN_TEST(ClientRendering_ResizeDuringFrameRejected)
{
	NullRenderer renderer;
	RendererConfig config;
	renderer.Initialize(config);
	renderer.BeginFrame();

	Status status = renderer.Resize(800, 600);
	CHECK(!status.IsOk());
	CHECK_EQ(status.GetCode(), ErrorCode::InvalidState);

	renderer.EndFrame();
}

// ---------------------------------------------------------------------------
// Application Integration
// ---------------------------------------------------------------------------

MODERN_TEST(ClientRendering_ApplicationRendererIntegration)
{
	ApplicationConfig appConfig;
	appConfig.maxFrames = 3;
	Application app(appConfig);

	InputSystem input;
	app.SetInputSystem(&input);

	NullRenderer renderer;
	RendererConfig renConfig{ 1024, 768, DisplayMode::Windowed, true };
	CHECK(renderer.Initialize(renConfig).IsOk());
	app.SetRenderer(&renderer);

	std::vector<std::string> sequence;

	app.SetUpdateCallback([&](uint64_t frame) {
		sequence.push_back("update:" + std::to_string(frame));
	});

	app.SetRenderCallback([&](uint64_t frame) {
		sequence.push_back("render:" + std::to_string(frame));
		renderer.Clear(RenderColor::White);
	});

	CHECK(app.Initialize().IsOk());
	CHECK(app.Run().IsOk());

	CHECK_EQ(renderer.GetRenderedFrameCount(), 3u);
	CHECK_EQ(renderer.GetClearCount(), 3u);
	CHECK_EQ(app.GetFrameCount(), 3u);

	// Verify update precedes render each frame
	std::vector<std::string> expected = {
		"update:1", "render:1",
		"update:2", "render:2",
		"update:3", "render:3"
	};
	CHECK(sequence == expected);
}

MODERN_TEST(ClientRendering_ApplicationResizePropagation)
{
	ApplicationConfig appConfig;
	appConfig.maxFrames = 1;
	Application app(appConfig);

	InputSystem input;
	app.SetInputSystem(&input);

	NullRenderer renderer;
	RendererConfig renConfig{ 800, 600, DisplayMode::Windowed, true };
	CHECK(renderer.Initialize(renConfig).IsOk());
	app.SetRenderer(&renderer);

	std::vector<InputEvent> script;
	script.push_back(InputEvent::MakeWindowResized(1920, 1080));
	app.SetInputSource(std::make_unique<ScriptedInputSource>(std::move(script)));

	CHECK(app.Initialize().IsOk());
	CHECK(app.Run().IsOk());

	// Application must have propagated the resize event to the renderer
	CHECK_EQ(renderer.GetWidth(), 1920u);
	CHECK_EQ(renderer.GetHeight(), 1080u);
}

MODERN_TEST(ClientRendering_ApplicationWithoutRendererStillRuns)
{
	ApplicationConfig appConfig;
	appConfig.maxFrames = 2;
	Application app(appConfig);

	uint64_t frames = 0;
	app.SetUpdateCallback([&](uint64_t f) { frames = f; });

	CHECK(app.Initialize().IsOk());
	CHECK(app.Run().IsOk());
	CHECK_EQ(frames, 2u);
	CHECK_EQ(app.GetRenderer(), nullptr);
}
// ---------------------------------------------------------------------------
// Main Test Runner
// ---------------------------------------------------------------------------

int main()
{
	std::printf("Modern CLIENT-004 rendering boundary tests\n\n");

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

