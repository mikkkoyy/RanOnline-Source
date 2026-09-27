#include "InputEvents.h"

namespace Modern::Client
{

const char* ToString(KeyCode code) noexcept
{
	switch (code)
	{
		case KeyCode::Unknown:      return "Unknown";
		case KeyCode::Escape:       return "Escape";
		case KeyCode::F1:           return "F1";
		case KeyCode::F2:           return "F2";
		case KeyCode::F3:           return "F3";
		case KeyCode::F4:           return "F4";
		case KeyCode::F5:           return "F5";
		case KeyCode::F6:           return "F6";
		case KeyCode::F7:           return "F7";
		case KeyCode::F8:           return "F8";
		case KeyCode::F9:           return "F9";
		case KeyCode::F10:          return "F10";
		case KeyCode::F11:          return "F11";
		case KeyCode::F12:          return "F12";
		case KeyCode::PrintScreen:  return "PrintScreen";
		case KeyCode::ScrollLock:   return "ScrollLock";
		case KeyCode::Pause:        return "Pause";
		case KeyCode::Tilde:        return "Tilde";
		case KeyCode::Minus:        return "Minus";
		case KeyCode::Equals:       return "Equals";
		case KeyCode::Backspace:    return "Backspace";
		case KeyCode::Enter:        return "Enter";
		case KeyCode::Tab:          return "Tab";
		case KeyCode::Space:        return "Space";
		case KeyCode::LeftBracket:  return "LeftBracket";
		case KeyCode::RightBracket: return "RightBracket";
		case KeyCode::Backslash:    return "Backslash";
		case KeyCode::Semicolon:    return "Semicolon";
		case KeyCode::Quote:        return "Quote";
		case KeyCode::Comma:        return "Comma";
		case KeyCode::Period:       return "Period";
		case KeyCode::Slash:        return "Slash";
		case KeyCode::Zero:         return "Zero";
		case KeyCode::One:          return "One";
		case KeyCode::Two:          return "Two";
		case KeyCode::Three:        return "Three";
		case KeyCode::Four:         return "Four";
		case KeyCode::Five:         return "Five";
		case KeyCode::Six:          return "Six";
		case KeyCode::Seven:        return "Seven";
		case KeyCode::Eight:        return "Eight";
		case KeyCode::Nine:         return "Nine";
		case KeyCode::A:            return "A";
		case KeyCode::B:            return "B";
		case KeyCode::C:            return "C";
		case KeyCode::D:            return "D";
		case KeyCode::E:            return "E";
		case KeyCode::F:            return "F";
		case KeyCode::G:            return "G";
		case KeyCode::H:            return "H";
		case KeyCode::I:            return "I";
		case KeyCode::J:            return "J";
		case KeyCode::K:            return "K";
		case KeyCode::L:            return "L";
		case KeyCode::M:            return "M";
		case KeyCode::N:            return "N";
		case KeyCode::O:            return "O";
		case KeyCode::P:            return "P";
		case KeyCode::Q:            return "Q";
		case KeyCode::R:            return "R";
		case KeyCode::S:            return "S";
		case KeyCode::T:            return "T";
		case KeyCode::U:            return "U";
		case KeyCode::V:            return "V";
		case KeyCode::W:            return "W";
		case KeyCode::X:            return "X";
		case KeyCode::Y:            return "Y";
		case KeyCode::Z:            return "Z";
		case KeyCode::LeftShift:    return "LeftShift";
		case KeyCode::RightShift:   return "RightShift";
		case KeyCode::LeftControl:  return "LeftControl";
		case KeyCode::RightControl: return "RightControl";
		case KeyCode::LeftAlt:      return "LeftAlt";
		case KeyCode::RightAlt:     return "RightAlt";
		case KeyCode::Left:         return "Left";
		case KeyCode::Right:        return "Right";
		case KeyCode::Up:           return "Up";
		case KeyCode::Down:         return "Down";
		case KeyCode::Insert:       return "Insert";
		case KeyCode::Home:         return "Home";
		case KeyCode::PageUp:       return "PageUp";
		case KeyCode::End:          return "End";
		case KeyCode::PageDown:     return "PageDown";
		case KeyCode::Delete:       return "Delete";
		case KeyCode::NumLock:      return "NumLock";
		case KeyCode::Numpad0:      return "Numpad0";
		case KeyCode::Numpad1:      return "Numpad1";
		case KeyCode::Numpad2:      return "Numpad2";
		case KeyCode::Numpad3:      return "Numpad3";
		case KeyCode::Numpad4:      return "Numpad4";
		case KeyCode::Numpad5:      return "Numpad5";
		case KeyCode::Numpad6:      return "Numpad6";
		case KeyCode::Numpad7:      return "Numpad7";
		case KeyCode::Numpad8:      return "Numpad8";
		case KeyCode::Numpad9:      return "Numpad9";
		case KeyCode::NumpadDivide:   return "NumpadDivide";
		case KeyCode::NumpadMultiply: return "NumpadMultiply";
		case KeyCode::NumpadMinus:    return "NumpadMinus";
		case KeyCode::NumpadPlus:     return "NumpadPlus";
		case KeyCode::NumpadPeriod:   return "NumpadPeriod";
		case KeyCode::NumpadEnter:    return "NumpadEnter";
		case KeyCode::Count:          return "Count";
	}

	return "Unknown";
}

const char* ToString(MouseButton button) noexcept
{
	switch (button)
	{
		case MouseButton::Unknown: return "Unknown";
		case MouseButton::Left:    return "Left";
		case MouseButton::Right:   return "Right";
		case MouseButton::Middle:  return "Middle";
		case MouseButton::X1:      return "X1";
		case MouseButton::X2:      return "X2";
		case MouseButton::Count:   return "Count";
	}

	return "Unknown";
}

const char* ToString(InputEventType type) noexcept
{
	switch (type)
	{
		case InputEventType::Key:           return "Key";
		case InputEventType::MouseMove:     return "MouseMove";
		case InputEventType::MouseButton:   return "MouseButton";
		case InputEventType::MouseWheel:    return "MouseWheel";
		case InputEventType::WindowClosed:  return "WindowClosed";
		case InputEventType::WindowResized: return "WindowResized";
	}

	return "Unknown";
}

} // namespace Modern::Client
