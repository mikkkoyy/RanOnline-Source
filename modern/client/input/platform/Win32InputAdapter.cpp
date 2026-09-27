#include "Win32InputAdapter.h"

namespace Modern::Client
{

namespace
{
	// Virtual-key constants, local to this translation unit. Duplicated from
	// Winuser.h on purpose so the public header never includes <Windows.h>.
	// Only the keys the game consumes are listed; anything else maps to
	// KeyCode::Unknown and is dropped by PushKeySample.
	constexpr uint32_t kVkEscape  = 0x1B;
	constexpr uint32_t kVkBack    = 0x08;
	constexpr uint32_t kVkTab     = 0x09;
	constexpr uint32_t kVkReturn  = 0x0D;
	constexpr uint32_t kVkShift   = 0x10;
	constexpr uint32_t kVkControl = 0x11;
	constexpr uint32_t kVkMenu    = 0x12;
	constexpr uint32_t kVkPause   = 0x13;
	constexpr uint32_t kVkSpace   = 0x20;
	constexpr uint32_t kVkPrior   = 0x21;
	constexpr uint32_t kVkNext    = 0x22;
	constexpr uint32_t kVkEnd     = 0x23;
	constexpr uint32_t kVkHome    = 0x24;
	constexpr uint32_t kVkLeft    = 0x25;
	constexpr uint32_t kVkUp      = 0x26;
	constexpr uint32_t kVkRight   = 0x27;
	constexpr uint32_t kVkDown    = 0x28;
	constexpr uint32_t kVkPrint   = 0x2A;
	constexpr uint32_t kVkInsert  = 0x2D;
	constexpr uint32_t kVkDelete  = 0x2E;
	constexpr uint32_t kVk0       = 0x30;
	constexpr uint32_t kVk1       = 0x31;
	constexpr uint32_t kVk2       = 0x32;
	constexpr uint32_t kVk3       = 0x33;
	constexpr uint32_t kVk4       = 0x34;
	constexpr uint32_t kVk5       = 0x35;
	constexpr uint32_t kVk6       = 0x36;
	constexpr uint32_t kVk7       = 0x37;
	constexpr uint32_t kVk8       = 0x38;
	constexpr uint32_t kVk9       = 0x39;
	constexpr uint32_t kVkA       = 0x41;
	constexpr uint32_t kVkZ       = 0x5A;
	constexpr uint32_t kVkLWin    = 0x5B;
	constexpr uint32_t kVkRWin    = 0x5C;
	constexpr uint32_t kVkNumpad0 = 0x60;
	constexpr uint32_t kVkNumpad9 = 0x69;
	constexpr uint32_t kVkF1      = 0x70;
	constexpr uint32_t kVkF12     = 0x7B;
	constexpr uint32_t kVkNumlock = 0x90;
	constexpr uint32_t kVkScroll  = 0x91;
	constexpr uint32_t kVkLShift  = 0xA0;
	constexpr uint32_t kVkRShift  = 0xA1;
	constexpr uint32_t kVkLControl = 0xA2;
	constexpr uint32_t kVkRControl = 0xA3;
	constexpr uint32_t kVkLMenu   = 0xA4;
	constexpr uint32_t kVkRMenu   = 0xA5;
	constexpr uint32_t kVkOem1    = 0xBA;
	constexpr uint32_t kVkOemPlus = 0xBB;
	constexpr uint32_t kVkOemComma = 0xBC;
	constexpr uint32_t kVkOemMinus = 0xBD;
	constexpr uint32_t kVkOemPeriod = 0xBE;
	constexpr uint32_t kVkOem2    = 0xBF;
	constexpr uint32_t kVkOem3    = 0xC0;
	constexpr uint32_t kVkOem4    = 0xDB;
	constexpr uint32_t kVkOem5    = 0xDC;
	constexpr uint32_t kVkOem6    = 0xDD;
	constexpr uint32_t kVkOem7    = 0xDE;
}

KeyCode Win32InputAdapter::TranslateKey(uint32_t virtualKey) noexcept
{
	// Letters arrive as contiguous VK_A..VK_Z matching KeyCode::A..KeyCode::Z
	// in declaration order; digits VK_0..VK_9 match Zero..Nine the same way.
	if (virtualKey >= kVkA && virtualKey <= kVkZ)
	{
		const uint32_t offset = virtualKey - kVkA;
		return static_cast<KeyCode>(static_cast<uint32_t>(KeyCode::A) + offset);
	}

	if (virtualKey >= kVk0 && virtualKey <= kVk9)
	{
		const uint32_t offset = virtualKey - kVk0;
		return static_cast<KeyCode>(static_cast<uint32_t>(KeyCode::Zero) + offset);
	}

	if (virtualKey >= kVkF1 && virtualKey <= kVkF12)
	{
		const uint32_t offset = virtualKey - kVkF1;
		return static_cast<KeyCode>(static_cast<uint32_t>(KeyCode::F1) + offset);
	}

	if (virtualKey >= kVkNumpad0 && virtualKey <= kVkNumpad9)
	{
		const uint32_t offset = virtualKey - kVkNumpad0;
		return static_cast<KeyCode>(static_cast<uint32_t>(KeyCode::Numpad0) + offset);
	}

	switch (virtualKey)
	{
		case kVkEscape: return KeyCode::Escape;
		case kVkBack:   return KeyCode::Backspace;
		case kVkTab:    return KeyCode::Tab;
		case kVkReturn: return KeyCode::Enter;
		case kVkSpace:  return KeyCode::Space;
		case kVkShift:  return KeyCode::LeftShift;
		case kVkControl: return KeyCode::LeftControl;
		case kVkMenu:   return KeyCode::LeftAlt;
		case kVkPause:  return KeyCode::Pause;
		case kVkPrior:  return KeyCode::PageUp;
		case kVkNext:   return KeyCode::PageDown;
		case kVkEnd:    return KeyCode::End;
		case kVkHome:   return KeyCode::Home;
		case kVkLeft:   return KeyCode::Left;
		case kVkUp:     return KeyCode::Up;
		case kVkRight:  return KeyCode::Right;
		case kVkDown:   return KeyCode::Down;
		case kVkPrint:  return KeyCode::PrintScreen;
		case kVkInsert: return KeyCode::Insert;
		case kVkDelete: return KeyCode::Delete;
		case kVkNumlock: return KeyCode::NumLock;
		case kVkScroll:  return KeyCode::ScrollLock;
		case kVkLShift:  return KeyCode::LeftShift;
		case kVkRShift:  return KeyCode::RightShift;
		case kVkLControl: return KeyCode::LeftControl;
		case kVkRControl: return KeyCode::RightControl;
		case kVkLMenu:   return KeyCode::LeftAlt;
		case kVkRMenu:   return KeyCode::RightAlt;
		case kVkOem1:    return KeyCode::Semicolon;
		case kVkOemPlus: return KeyCode::Equals;
		case kVkOemComma: return KeyCode::Comma;
		case kVkOemMinus: return KeyCode::Minus;
		case kVkOemPeriod: return KeyCode::Period;
		case kVkOem2:    return KeyCode::Slash;
		case kVkOem3:    return KeyCode::Tilde;
		case kVkOem4:    return KeyCode::LeftBracket;
		case kVkOem5:    return KeyCode::Backslash;
		case kVkOem6:    return KeyCode::RightBracket;
		case kVkOem7:    return KeyCode::Quote;
		default:         break;
	}

	// kVkLWin/kVkRWin and the numpad operator keys have no portable
	// counterpart by design: the OS keys never reach gameplay, and the
	// numpad operators ride the wheel/mouse bindings instead.
	return KeyCode::Unknown;
}

void Win32InputAdapter::PushKeySample(std::vector<InputEvent>& out, const Win32KeySample& sample)
{
	const KeyCode code = TranslateKey(sample.virtualKey);
	if (code == KeyCode::Unknown)
	{
		return;
	}

	out.push_back(InputEvent::MakeKey(code, sample.pressed, sample.repeated));
}

void Win32InputAdapter::PushMouseSample(std::vector<InputEvent>& out, const Win32MouseSample& sample)
{
	if (sample.moved)
	{
		out.push_back(InputEvent::MakeMouseMove(sample.x, sample.y));
	}

	if (sample.leftChanged)
	{
		out.push_back(InputEvent::MakeMouseButton(MouseButton::Left, sample.leftPressed, sample.x, sample.y));
	}

	if (sample.rightChanged)
	{
		out.push_back(InputEvent::MakeMouseButton(MouseButton::Right, sample.rightPressed, sample.x, sample.y));
	}

	if (sample.middleChanged)
	{
		out.push_back(InputEvent::MakeMouseButton(MouseButton::Middle, sample.middlePressed, sample.x, sample.y));
	}

	if (sample.wheelDelta != 0)
	{
		out.push_back(InputEvent::MakeMouseWheel(0, sample.wheelDelta));
	}
}

} // namespace Modern::Client
