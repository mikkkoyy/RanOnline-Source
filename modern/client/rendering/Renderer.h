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

#include "AssetUpload.h"
#include "RenderingTypes.h"
#include "types/Result.h"

#include <cstdint>

namespace Modern::Client
{

// Everything one indexed draw call needs, platform-independent: the opaque
// handles naming the uploaded mesh and (optionally) texture, the three
// row-major transforms, and the visible fallback color used when no texture
// is bound. Matrices are pointers to 16 row-major floats so the struct stays
// trivially copyable and cheap to build at the call site.
//
// CLIENT-RENDER-002: the optional index range draws one MeshSection of an
// uploaded mesh, so a character with several materials becomes several draws
// that each bind their own texture. Both fields default to the whole-mesh
// draw that existed before them: indexCount == 0 means "every index in the
// mesh", which keeps every existing construction site working unchanged.
// A non-zero range must be a whole number of triangles and must lie inside
// the mesh; both are validated where the mesh is known, in the backend.
struct MeshDrawCommand
{
	MeshResourceHandle mesh;             // required: names the uploaded mesh
	ImageResourceHandle texture;         // invalid handle: use fallbackColor
	const float*        worldMatrix;     // 16 row-major floats
	const float*        viewMatrix;      // 16 row-major floats
	const float*        projectionMatrix;// 16 row-major floats
	RenderColor         fallbackColor = RenderColor::White;

	// CLIENT-RENDER-002: first index and count of the run to draw. Count 0
	// draws every index from offset 0 (the pre-section behaviour). Count
	// non-zero requires count % 3 == 0 and offset + count <= index count.
	uint32_t indexOffset = 0;
	uint32_t indexCount  = 0;
};

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

	// One indexed draw call: a triangle-list mesh previously uploaded through
	// an IAssetUploader (CLIENT-009), transformed by the three row-major
	// matrices, optionally textured by an uploaded image. When the texture
	// handle is invalid or cannot be resolved the backend binds its default
	// texture and the fallback color becomes the visible result, so an
	// untextured draw is still a real draw.
	//
	// This is the smallest drawing contract the renderer boundary needs: it
	// carries no GPU type, no asset byte and no backend object -- only the
	// opaque upload handles and the platform-independent Mat4x4. It is
	// valid only when InFrame; NullRenderer implements it as a counted no-op.
	virtual Status DrawIndexedMesh(const MeshDrawCommand& command) = 0;
};

} // namespace Modern::Client
