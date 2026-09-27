#include "Application.h"
#include "rendering/Renderer.h"


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
	, m_inputSource(std::make_unique<NullInputSource>())
{
}

Application::Application(ApplicationConfig config)
	: m_config(config)
	, m_platform(std::make_unique<NullPlatformEvents>())
	, m_inputSource(std::make_unique<NullInputSource>())
{
}

Application::Application(std::unique_ptr<IPlatformEvents> platform)
	: m_platform(std::move(platform))
	, m_inputSource(std::make_unique<NullInputSource>())
{
	if (!m_platform)
	{
		m_platform = std::make_unique<NullPlatformEvents>();
	}
}

Application::Application(ApplicationConfig config, std::unique_ptr<IPlatformEvents> platform)
	: m_config(config)
	, m_platform(std::move(platform))
	, m_inputSource(std::make_unique<NullInputSource>())
{
	if (!m_platform)
	{
		m_platform = std::make_unique<NullPlatformEvents>();
	}
}

void Application::SetInputSource(std::unique_ptr<IInputSource> source)
{
	m_inputSource = std::move(source);
	if (!m_inputSource)
	{
		m_inputSource = std::make_unique<NullInputSource>();
	}
}

void Application::SubscribeInput(std::function<void(const InputEvent&)> subscriber)
{
	if (subscriber)
	{
		m_inputSubscribers.push_back(std::move(subscriber));
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

		// Frame order is fixed:
		// 1. Source -> Queue -> Process Resize -> Subscribers
		// 2. Update callback
		// 3. Renderer BeginFrame -> Render callback -> Renderer EndFrame
		// 4. InputSystem EndFrame
		const std::vector<InputEvent> frameEvents = m_inputSource->PollEvents();
		for (const InputEvent& event : frameEvents)
		{
			if (event.type == InputEventType::WindowResized && m_renderer != nullptr)
			{
				const auto& resize = std::get<WindowResizeEvent>(event.payload);
				if (resize.width > 0 && resize.height > 0 && m_renderer->IsInitialized())
				{
					// Propagate resize to renderer abstraction before subscribers
					m_renderer->Resize(resize.width, resize.height);
				}
			}

			if (m_input != nullptr)
			{
				m_input->PushEvent(event);
				for (const auto& subscriber : m_inputSubscribers)
				{
					subscriber(event);
				}
			}
		}

		const bool closeRequested = m_inputSource->RequestsClose();

		++m_frameCount;

		if (m_update)
		{
			m_update(m_frameCount);
		}

		if (m_renderer != nullptr && m_renderer->IsInitialized())
		{
			if (m_renderer->BeginFrame().IsOk())
			{
				if (m_render)
				{
					m_render(m_frameCount);
				}
				m_renderer->EndFrame();
			}
		}

		if (m_input != nullptr)
		{
			m_input->EndFrame();
		}

		if (closeRequested)
		{
			break;
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
void Application::SetRenderCallback(std::function<void(uint64_t)> callback)
{
	m_render = std::move(callback);
}


} // namespace Modern::Client
