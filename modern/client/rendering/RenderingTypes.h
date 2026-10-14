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
    uint32_t    width       = 960;
    uint32_t    height      = 540;
	DisplayMode displayMode = DisplayMode::Windowed;
    bool        vsync       = true;

    // Opaque platform window handle (HWND on Win32, but the type is void* so
    // no platform header reaches this interface). Required by renderers that
    // bind a swap chain to a window; ignored by windowless ones.
    void* windowHandle = nullptr;

	// Validates parameters. Width and height must be non-zero.
	bool IsValid() const noexcept
	{
		return width > 0 && height > 0;
	}
};

// Row-major 4x4 transformation matrix: 16 float32 values laid out m[0..3] =
// row 0, m[4..7] = row 1, m[8..11] = row 2, m[12..15] = row 3.
//
// This is the single CPU-side matrix convention for every IRenderer draw
// call. The HLSL shaders compiled for the D3D11 backend declare their
// constant-buffer matrices `row_major` (#pragma pack_matrix(row_major)), so a
// Mat4x4 uploads byte-for-byte and HLSL's mul(matrix, vector) computes
// exactly the row-major product the caller constructed: no transposition
// step exists anywhere in the pipeline. A future backend that prefers
// column-major storage does its own conversion at the upload boundary.
struct Mat4x4
{
	float m[16] = {};

	Mat4x4() noexcept = default;

	// Copies 16 floats from a row-major source. Null leaves the matrix zeroed.
	explicit Mat4x4(const float* rowMajorValues) noexcept
	{
		if (rowMajorValues != nullptr)
		{
			for (int i = 0; i < 16; ++i)
			{
				m[i] = rowMajorValues[i];
			}
		}
	}

	const float* GetData() const noexcept { return m; }
	static const Mat4x4& Identity() noexcept
	{
		static const Mat4x4 identity = []
		{
			Mat4x4 result;
			result.m[0] = result.m[5] = result.m[10] = result.m[15] = 1.0f;
			return result;
		}();
		return identity;
	}
};

} // namespace Modern::Client
