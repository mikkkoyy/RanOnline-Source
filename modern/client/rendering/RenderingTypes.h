#pragma once

// CLIENT-004: platform-independent rendering configuration and types.
//
// Modern client rendering must not expose Direct3D, Vulkan, OpenGL, DXGI,
// HWND, or Windows types in public interfaces.

#include <cstdint>

namespace Modern::Client
{

// Lifecycle state of an IRenderer implementation.
enum class RendererState : uint8_t
{
	Uninitialized = 0,
	Initialized,
	InFrame,
	ShutdownState
};

const char* ToString(RendererState state) noexcept;

// Display mode for the presentation surface.
enum class DisplayMode : uint8_t
{
	Windowed = 0,
	Fullscreen,
	BorderlessFullscreen
};

const char* ToString(DisplayMode mode) noexcept;

// Normalized floating-point color (RGBA, 0.0f - 1.0f).
struct RenderColor
{
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
	float a = 1.0f;

	constexpr RenderColor() noexcept = default;
	constexpr RenderColor(float red, float green, float blue, float alpha = 1.0f) noexcept
		: r(red), g(green), b(blue), a(alpha)
	{
	}

	static const RenderColor Black;
	static const RenderColor White;
	static const RenderColor ClearCornflowerBlue;
};

// Platform-independent configuration for initializing or resizing the renderer.
struct RendererConfig
{
	uint32_t    width       = 1280;
	uint32_t    height      = 720;
	DisplayMode displayMode = DisplayMode::Windowed;
	bool        vsync       = true;

	// Validates parameters. Width and height must be non-zero.
	bool IsValid() const noexcept
	{
		return width > 0 && height > 0;
	}
};

} // namespace Modern::Client
