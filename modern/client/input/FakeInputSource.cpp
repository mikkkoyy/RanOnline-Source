#include "FakeInputSource.h"

namespace Modern::Client
{

FakeInputSource::FakeInputSource(std::vector<InputEvent> script)
	: m_script(std::move(script))
{
}

void FakeInputSource::SetScript(std::vector<InputEvent> script)
{
	m_script         = std::move(script);
	m_position       = 0;
	m_closeRequested = false;
}

void FakeInputSource::Reset()
{
	m_position       = 0;
	m_closeRequested = false;
}

Status FakeInputSource::PumpFrame(InputSystem& input)
{
	if (m_position >= m_script.size())
	{
		return Status(ErrorCode::NotFound);
	}

	const InputEvent& event = m_script[m_position];
	const Status status     = input.PushEvent(event);
	if (status.IsError())
	{
		return status;
	}

	if (event.type == InputEventType::WindowClosed)
	{
		m_closeRequested = true;
	}

	++m_position;
	return Ok();
}

Status FakeInputSource::DrainAll(InputSystem& input)
{
	if (m_position >= m_script.size())
	{
		return Status(ErrorCode::NotFound);
	}

	while (m_position < m_script.size())
	{
		const Status status = PumpFrame(input);
		if (status.IsError())
		{
			return status;
		}
	}

	return Ok();
}

} // namespace Modern::Client
