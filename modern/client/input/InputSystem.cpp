#include "InputSystem.h"

namespace Modern::Client
{

Status InputSystem::Initialize()
{
	if (m_initialized)
	{
		return Status(ErrorCode::InvalidState);
	}

	m_initialized = true;
	return Ok();
}

void InputSystem::Reset()
{
	m_queue.clear();
	m_keysDown.fill(false);
	m_buttonsDown.fill(false);
	m_mouseX      = 0;
	m_mouseY      = 0;
	m_wheelDeltaX = 0;
	m_wheelDeltaY = 0;
	m_initialized = false;
}

void InputSystem::EndFrame()
{
	m_queue.clear();
	m_wheelDeltaX = 0;
	m_wheelDeltaY = 0;
}

Status InputSystem::PushEvent(const InputEvent& event)
{
	if (!m_initialized)
	{
		return Status(ErrorCode::InvalidState);
	}

	switch (event.type)
	{
		case InputEventType::Key:
		{
			const KeyEvent& key = std::get<KeyEvent>(event.payload);
			if (!IsValidKey(key.code))
			{
				return Status(ErrorCode::InvalidArgument);
			}

			m_keysDown[static_cast<size_t>(key.code)] = key.pressed;
			break;
		}

		case InputEventType::MouseMove:
		{
			const MouseMoveEvent& move = std::get<MouseMoveEvent>(event.payload);
			m_mouseX = move.x;
			m_mouseY = move.y;
			break;
		}

		case InputEventType::MouseButton:
		{
			const MouseButtonEvent& mouse = std::get<MouseButtonEvent>(event.payload);
			if (!IsValidButton(mouse.button))
			{
				return Status(ErrorCode::InvalidArgument);
			}

			m_buttonsDown[static_cast<size_t>(mouse.button)] = mouse.pressed;
			m_mouseX = mouse.x;
			m_mouseY = mouse.y;
			break;
		}

		case InputEventType::MouseWheel:
		{
			const MouseWheelEvent& wheel = std::get<MouseWheelEvent>(event.payload);
			m_wheelDeltaX += wheel.deltaX;
			m_wheelDeltaY += wheel.deltaY;
			break;
		}

		case InputEventType::WindowClosed:
			break;

		case InputEventType::WindowResized:
		{
			const WindowResizeEvent& resize = std::get<WindowResizeEvent>(event.payload);
			if (resize.width == 0 || resize.height == 0)
			{
				return Status(ErrorCode::InvalidArgument);
			}

			break;
		}
	}

	m_queue.push_back(event);
	return Ok();
}

void InputSystem::ForEachEvent(const std::function<void(const InputEvent&)>& fn) const
{
	for (const InputEvent& event : m_queue)
	{
		fn(event);
	}
}

bool InputSystem::IsKeyDown(KeyCode code) const
{
	if (!IsValidKey(code))
	{
		return false;
	}

	return m_keysDown[static_cast<size_t>(code)];
}

bool InputSystem::IsMouseButtonDown(MouseButton button) const
{
	if (!IsValidButton(button))
	{
		return false;
	}

	return m_buttonsDown[static_cast<size_t>(button)];
}

} // namespace Modern::Client
