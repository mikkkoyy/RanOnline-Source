#pragma once

#include "types/Result.h"

#include <cstdint>
#include <functional>
#include <memory>

namespace Modern::Client
{

// Lifecycle of the modern client application.
//
// The application owns startup, shutdown and the frame/update loop. It owns
// no gameplay, no rendering implementation, no UI implementation and no
// network protocol: those are future systems that will plug into this
// boundary without pulling legacy client architecture into modern/.
//
// Source reference: the ASURA client spreads this responsibility across
// CGameClient2App (MFC CWinApp), CGameClient2Wnd and CD3DApplication, which
// couples the message loop to MFC 7.1 and a specific DirectX device. This
// boundary keeps the loop and replaces the coupling. See
// reference/client/ASURA_CLIENT_ARCHITECTURE.md section 1 and
// reference/client/MODERN_CLIENT_MAPPING.md ("Application Framework").
enum class ApplicationState : uint8_t
{
	Uninitialized = 0,
	Initialized,
	Running,
	Stopping,
	Stopped,
};

// Human-readable name of a lifecycle state. Stable for logs and test output.
const char* ToString(ApplicationState state) noexcept;

// Platform event boundary.
//
// The application loop must not spread Win32 calls through the client.
// Anything platform specific stays behind this interface; the default
// implementation is headless so the loop is testable without a window,
// DirectX, or MFC.
class IPlatformEvents
{
public:
	virtual ~IPlatformEvents() = default;

	// Pumps pending platform events. Returns false when the platform is
	// asking the application to exit, e.g. the window was closed.
	virtual bool PumpEvents() = 0;
};

// Headless platform: never asks the application to exit. The loop then ends
// through the frame budget (ApplicationConfig::maxFrames) or RequestStop(),
// which is what makes the update loop deterministic in tests.
class NullPlatformEvents final : public IPlatformEvents
{
public:
	bool PumpEvents() override { return true; }
};

struct ApplicationConfig
{
	// Maximum frames Run() executes before returning. Zero means unbounded:
	// the loop runs until RequestStop() or the platform closes. Tests always
	// set a budget or stop the loop from the update callback.
	uint32_t maxFrames = 0;
};

class Application
{
public:
	Application();
	explicit Application(ApplicationConfig config);
	explicit Application(std::unique_ptr<IPlatformEvents> platform);
	Application(ApplicationConfig config, std::unique_ptr<IPlatformEvents> platform);

	Application(const Application&)            = delete;
	Application& operator=(const Application&) = delete;
	Application(Application&&)                 = default;
	Application& operator=(Application&&)      = default;
	~Application()                             = default;

	// Uninitialized -> Initialized. Any other state reports InvalidState,
	// except Stopped, which is terminal and reports NotAllowed.
	Status Initialize();

	// Runs the deterministic update loop to completion and ends Stopped:
	//
	//   while running { pump platform events; run one update; count one frame }
	//
	// Only valid from Initialized. Returns InvalidState from any other live
	// state and NotAllowed once stopped. Rendering is intentionally absent:
	// the update callback is the placeholder future systems will attach to.
	Status Run();

	// Requests the running loop to stop. Takes Running -> Stopping; the loop
	// observes it at the next frame boundary and Run() returns Stopped.
	// Valid only from Running (InvalidState otherwise, NotAllowed from
	// Stopped), so it is safe to call from the update callback.
	Status RequestStop();

	// Releases the application. Valid from Initialized (start-then-stop
	// without running) and consumed by Run() implicitly: calling it after
	// Run() reports NotAllowed because the application is already Stopped.
	// Never valid from Running: stop the loop first.
	//
	// Declared Status rather than void so invalid transitions follow the
	// core value-based convention instead of failing silently.
	Status Shutdown();

	// Receives the 1-based frame index after platform events are pumped.
	// Executed in call order, so frame N always precedes frame N+1.
	void SetUpdateCallback(std::function<void(uint64_t)> callback);

	ApplicationState GetState() const noexcept { return m_state; }
	uint64_t         GetFrameCount() const noexcept { return m_frameCount; }

private:
	ApplicationState                  m_state      = ApplicationState::Uninitialized;
	ApplicationConfig                 m_config{};
	std::unique_ptr<IPlatformEvents>  m_platform;
	std::function<void(uint64_t)>     m_update;
	uint64_t                          m_frameCount = 0;
};

} // namespace Modern::Client
