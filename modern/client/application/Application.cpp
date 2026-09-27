#include "Application.h"

namespace Modern::Client
{

const char* ToString(ApplicationState state) noexcept
{
	switch (state)
	{
		case ApplicationState::Uninitialized: return "Uninitialized";
		case ApplicationState::Initialized:   return "Initialized";
		case ApplicationState::Running:       return "Running";
		case ApplicationState::Stopping:      return "Stopping";
		case ApplicationState::Stopped:       return "Stopped";
	}

	return "Unknown";
}

Application::Application()
	: m_platform(std::make_unique<NullPlatformEvents>())
{
}

Application::Application(ApplicationConfig config)
	: m_config(config)
	, m_platform(std::make_unique<NullPlatformEvents>())
{
}

Application::Application(std::unique_ptr<IPlatformEvents> platform)
	: m_platform(std::move(platform))
{
	if (!m_platform)
	{
		m_platform = std::make_unique<NullPlatformEvents>();
	}
}

Application::Application(ApplicationConfig config, std::unique_ptr<IPlatformEvents> platform)
	: m_config(config)
	, m_platform(std::move(platform))
{
	if (!m_platform)
	{
		m_platform = std::make_unique<NullPlatformEvents>();
	}
}

Status Application::Initialize()
{
	if (m_state == ApplicationState::Stopped)
	{
		return Status(ErrorCode::NotAllowed);
	}

	if (m_state != ApplicationState::Uninitialized)
	{
		return Status(ErrorCode::InvalidState);
	}

	m_state = ApplicationState::Initialized;
	return Ok();
}

Status Application::Run()
{
	// Stopped is terminal: restart requires a new Application, matching the
	// core Character convention where Destroyed is terminal until Reset.
	if (m_state == ApplicationState::Stopped)
	{
		return Status(ErrorCode::NotAllowed);
	}

	if (m_state != ApplicationState::Initialized)
	{
		return Status(ErrorCode::InvalidState);
	}

	m_state = ApplicationState::Running;

	while (m_state == ApplicationState::Running)
	{
		// Platform close ends the loop without an error: the application
		// keeps its contract that Run() completes Stopped either way.
		if (!m_platform->PumpEvents())
		{
			break;
		}

		++m_frameCount;

		if (m_update)
		{
			m_update(m_frameCount);
		}

		if (m_state == ApplicationState::Stopping)
		{
			break;
		}

		if (m_config.maxFrames != 0 && m_frameCount >= m_config.maxFrames)
		{
			break;
		}
	}

	m_state = ApplicationState::Stopped;
	return Ok();
}

Status Application::RequestStop()
{
	if (m_state == ApplicationState::Stopped)
	{
		return Status(ErrorCode::NotAllowed);
	}

	if (m_state != ApplicationState::Running)
	{
		return Status(ErrorCode::InvalidState);
	}

	m_state = ApplicationState::Stopping;
	return Ok();
}

Status Application::Shutdown()
{
	if (m_state == ApplicationState::Stopped)
	{
		return Status(ErrorCode::NotAllowed);
	}

	// Shutdown from Running would skip the loop's Stopped transition and
	// silently end updates mid-frame, so it is rejected: RequestStop()
	// (or the frame budget) must end the loop first.
	if (m_state != ApplicationState::Initialized)
	{
		return Status(ErrorCode::InvalidState);
	}

	m_state = ApplicationState::Stopped;
	return Ok();
}

void Application::SetUpdateCallback(std::function<void(uint64_t)> callback)
{
	m_update = std::move(callback);
}

} // namespace Modern::Client
