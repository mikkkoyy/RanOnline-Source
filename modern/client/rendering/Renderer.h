#pragma once

// CLIENT-004: platform-independent renderer interface.
//
// Replaces the legacy CD3DApplication / DxRendererDX9 / DxRendererDX11
// inheritance tree with a clean contract.
//
// Modern client components interact only with this interface and never directly
// with graphics APIs (Direct3D 11/12, Vulkan, OpenGL, Metal) or platform window
// structures.
//
// Lifecycle:
//   Uninitialized -> Initialized -> InFrame <-> Initialized -> Shutdown
//
// State transitions enforce strict ErrorCode returns (e.g. InvalidState,
// InvalidArgument, NotAllowed).

#include "RenderingTypes.h"
#include "types/Result.h"

#include <cstdint>

namespace Modern::Client
{

class IRenderer
{
public:
	virtual ~IRenderer() = default;

	// Initializes the renderer with the specified configuration.
	// Valid only when Uninitialized.
	virtual Status Initialize(const RendererConfig& config) = 0;

	// Begins a new render frame.
	// Valid only when Initialized (not already InFrame).
	virtual Status BeginFrame() = 0;

	// Completes the active render frame and presents the backbuffer.
	// Valid only when InFrame.
	virtual Status EndFrame() = 0;

	// Clears the active render target with the specified color.
	// Valid only when InFrame.
	virtual Status Clear(const RenderColor& color) = 0;

	// Resizes the render surface. Width and height must be > 0.
	// Valid only when Initialized (cannot resize while actively InFrame).
	virtual Status Resize(uint32_t width, uint32_t height) = 0;

	// Shuts down the renderer and releases resources.
	// Valid from Initialized or InFrame (forces frame end), transitioning
	// to ShutdownState. Calling Shutdown multiple times is safe (idempotent).
	virtual void Shutdown() noexcept = 0;

	// Queries the current lifecycle state.
	virtual RendererState GetState() const noexcept = 0;

	// Convenience check: true if Initialized or InFrame.
	virtual bool IsInitialized() const noexcept = 0;

	// Current dimensions.
	virtual uint32_t GetWidth() const noexcept = 0;
	virtual uint32_t GetHeight() const noexcept = 0;

	// Active configuration.
	virtual const RendererConfig& GetConfig() const noexcept = 0;
};

} // namespace Modern::Client
