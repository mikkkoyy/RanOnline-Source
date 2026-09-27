#include "RenderingTypes.h"

namespace Modern::Client
{

const RenderColor RenderColor::Black(0.0f, 0.0f, 0.0f, 1.0f);
const RenderColor RenderColor::White(1.0f, 1.0f, 1.0f, 1.0f);
const RenderColor RenderColor::ClearCornflowerBlue(0.392f, 0.584f, 0.929f, 1.0f);

const char* ToString(RendererState state) noexcept
{
	switch (state)
	{
	case RendererState::Uninitialized:
		return "Uninitialized";
	case RendererState::Initialized:
		return "Initialized";
	case RendererState::InFrame:
		return "InFrame";
	case RendererState::ShutdownState:
		return "Shutdown";
	}
	return "Unknown";
}

const char* ToString(DisplayMode mode) noexcept
{
	switch (mode)
	{
	case DisplayMode::Windowed:
		return "Windowed";
	case DisplayMode::Fullscreen:
		return "Fullscreen";
	case DisplayMode::BorderlessFullscreen:
		return "BorderlessFullscreen";
	}
	return "Unknown";
}

} // namespace Modern::Client
