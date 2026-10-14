#pragma once

// CLIENT-RENDER-001B: the Direct3D 11 implementation of the asset upload
// contract (CLIENT-009).
//
// D3D11AssetUploader turns validated CPU assets into real D3D11 objects:
// a MeshAsset becomes one vertex buffer and one index buffer, an ImageAsset
// becomes one Texture2D plus a shader-resource view. It implements
// IAssetUploader unchanged -- same handles, same lifecycle table, same
// decision order -- and adds the D3D11-specific resolution the draw path
// needs (TryResolveMesh / TryResolveImage), which is deliberately NOT on
// the platform-independent interface.
//
// It borrows a D3D11Renderer, never owns it and never shuts it down, exactly
// as NullAssetUploader borrows its IRenderer. The renderer's state *is* this
// uploader's lifecycle:
//
//   renderer state        Upload          Release         IsValid / counts / info
//   -------------------   --------------  --------------  ------------------------
//   no renderer attached  InvalidState    InvalidState    false / 0 / InvalidState
//   Uninitialized         InvalidState    InvalidState    false / 0 / InvalidState
//   Initialized / InFrame Ok              Ok              live?
//   ShutdownState         NotAllowed      NotAllowed      false / 0 / NotAllowed
//
// GPU resources are owned by the entries below and released on Release, on
// the first mutating call after renderer shutdown (which drops the whole
// registry), and by the destructor. The CPU asset layer stays
// platform-independent: no GPU object is ever stored in a MeshAsset or an
// ImageAsset, and this header is the only place the two halves meet.

#include "rendering/AssetUpload.h"
#include "rendering/Renderer.h"

#include <Windows.h>
#include <d3d11.h>

#include <cstdint>
#include <vector>

namespace Modern::Client
{

class D3D11Renderer;

// The GPU half of an uploaded mesh, resolved for the draw path. Raw D3D11
// pointers by necessity: this struct is the D3D11 backend's own vocabulary
// and never appears in a platform-independent header.
struct D3D11MeshGpuResources
{
	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11Buffer* indexBuffer  = nullptr;
	uint32_t      indexCount   = 0;
};

// The GPU half of an uploaded image.
struct D3D11ImageGpuResources
{
	ID3D11ShaderResourceView* shaderResourceView = nullptr;
	uint32_t                   width             = 0;
	uint32_t                   height            = 0;
};

class D3D11AssetUploader final : public IAssetUploader
{
public:
	// Attaches the observed renderer at construction. Borrowed, never owned:
	// the caller keeps it alive across every call and shuts it down itself.
	explicit D3D11AssetUploader(const D3D11Renderer* renderer) noexcept
		: m_renderer(renderer)
	{
	}

	D3D11AssetUploader(const D3D11AssetUploader&) = delete;
	D3D11AssetUploader& operator=(const D3D11AssetUploader&) = delete;
	D3D11AssetUploader(D3D11AssetUploader&&) = delete;
	D3D11AssetUploader& operator=(D3D11AssetUploader&&) = delete;

	// Releases every live GPU resource. Safe to call after the renderer is
	// shut down: D3D11 child objects hold their own device reference, so
	// releasing a buffer after its device's public Release is well defined.
	~D3D11AssetUploader() override;

	Result<ImageResourceHandle> UploadImage(const ImageAsset& image) override;
	Result<MeshResourceHandle> UploadMesh(const MeshAsset& mesh) override;

	bool IsValidImage(ImageResourceHandle handle) const noexcept override;
	bool IsValidMesh(MeshResourceHandle handle) const noexcept override;

	Status ReleaseImage(ImageResourceHandle handle) noexcept override;
	Status ReleaseMesh(MeshResourceHandle handle) noexcept override;

	Result<ImageResourceInfo> GetImageInfo(ImageResourceHandle handle) const override;
	Result<MeshResourceInfo> GetMeshInfo(MeshResourceHandle handle) const override;

	size_t GetLiveImageCount() const noexcept override;
	size_t GetLiveMeshCount() const noexcept override;

	// D3D11-specific resolution for the draw path. False for an invalid,
	// released, unknown or post-shutdown handle alike; on success the GPU
	// resources are valid until the entry is released.
	bool TryResolveMesh(MeshResourceHandle handle, D3D11MeshGpuResources& out) const noexcept;
	bool TryResolveImage(ImageResourceHandle handle, D3D11ImageGpuResources& out) const noexcept;

private:
	// One retained resource: the handle, the asset's numbers, and the GPU
	// objects those numbers became. The vectors are the registry -- small,
	// insertion ordered, linearly searched, released on Release or by the
	// shutdown policy above.
	struct ImageEntry
	{
		ImageResourceHandle       handle;
		ImageResourceInfo         info;
		ID3D11Texture2D*          texture = nullptr;
		ID3D11ShaderResourceView* srv     = nullptr;
	};

	struct MeshEntry
	{
		MeshResourceHandle handle;
		MeshResourceInfo   info;
		ID3D11Buffer*      vertexBuffer = nullptr;
		ID3D11Buffer*      indexBuffer  = nullptr;
		uint32_t           indexCount   = 0;
	};

	// The observed renderer's state, or Uninitialized when none is attached.
	RendererState PeekState() const noexcept;

	// The lifecycle column of the table above. The mutating variant also
	// applies the shutdown policy (drops the registry, releasing every GPU
	// object it holds, the first time it observes ShutdownState); the const
	// variant answers without touching it, because IsValid/counts/info are
	// reads.
	Status CheckLifecycle() noexcept;
	Status CheckLifecycleConst() const noexcept;
	bool IsLive() const noexcept;

	ImageEntry* FindImage(ImageResourceHandle handle) noexcept;
	const ImageEntry* FindImage(ImageResourceHandle handle) const noexcept;
	MeshEntry* FindMesh(MeshResourceHandle handle) noexcept;
	const MeshEntry* FindMesh(MeshResourceHandle handle) const noexcept;

	void ReleaseEntry(ImageEntry& entry) noexcept;
	void ReleaseEntry(MeshEntry& entry) noexcept;
	void ReleaseAll() noexcept;

	const D3D11Renderer* m_renderer    = nullptr;
	uint64_t             m_nextImageId = 1;
	uint64_t             m_nextMeshId  = 1;
	std::vector<ImageEntry> m_images;
	std::vector<MeshEntry>  m_meshes;
};

} // namespace Modern::Client
