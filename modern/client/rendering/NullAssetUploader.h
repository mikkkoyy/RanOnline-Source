#pragma once

// CLIENT-009: the headless implementation of the asset upload contract.
//
// NullAssetUploader is the "null backend" of uploading, the counterpart of
// NullRenderer: it turns an ImageAsset or a MeshAsset into a deterministic
// handle and retains the asset's *metadata* so tests can prove the values
// survived the crossing -- and it creates no GPU resource, no device, no
// context and no OS object of any kind.
//
// It borrows an IRenderer, never owns it and never shuts it down, exactly as
// Application borrows its renderer. The renderer's state *is* this
// uploader's lifecycle, so there is no second state machine to keep in step
// and no Initialize()/Shutdown() pair to call in the right order:
//
//   renderer state        Upload          Release         IsValid / counts / info
//   -------------------   --------------  --------------  ------------------------
//   no renderer attached  InvalidState    InvalidState    false / 0 / InvalidState
//   Uninitialized         InvalidState    InvalidState    false / 0 / InvalidState
//   Initialized / InFrame Ok              Ok              live?
//   ShutdownState         NotAllowed      NotAllowed      false / 0 / NotAllowed
//
// Handle rules apply only once the lifecycle lets the call through: the
// invalid sentinel is InvalidArgument, an unknown or already-released id is
// NotFound, and a live id succeeds. Lifecycle is always checked before the
// handle, so a shut-down uploader refuses everything uniformly.
//
// Shutdown policy: when the renderer is shut down while handles exist, every
// handle becomes invalid *immediately* -- IsValid* and the live counts answer
// from the renderer state, so they read false and 0 without being asked
// again. The retained metadata is released at the first mutating call
// afterwards (which reports NotAllowed and drops everything it holds) or,
// failing that, when the uploader is destroyed. No id ever comes back: ids
// are assigned per uploader instance, from 1 upward, in upload order, are
// never recycled after a release, and RendererState::ShutdownState is
// terminal under the IRenderer contract, so there is no state to come back
// to.
//
// What it deliberately does not do: it does not cache decoded assets (that
// would be a second ResourceManager), it does not load anything (no
// ResourceId, provider or filesystem appears in this header), it does not
// know which IRenderer implementation it observes, and it holds no pixel or
// vertex bytes -- the numbers above are all it retains.

#include "rendering/AssetUpload.h"
#include "rendering/Renderer.h"

#include <cstdint>
#include <vector>

namespace Modern::Client
{

class NullAssetUploader final : public IAssetUploader
{
public:
	NullAssetUploader() = default;

	// Attaches the observed renderer at construction. Borrowed, never owned:
	// the caller keeps it alive across every call and shuts it down itself.
	explicit NullAssetUploader(const IRenderer* renderer) noexcept : m_renderer(renderer) {}

	NullAssetUploader(const NullAssetUploader&) = delete;
	NullAssetUploader& operator=(const NullAssetUploader&) = delete;
	NullAssetUploader(NullAssetUploader&&) = default;
	NullAssetUploader& operator=(NullAssetUploader&&) = default;
	~NullAssetUploader() = default;

	// Attaches or replaces the observed renderer. Null detaches; a detached
	// uploader answers exactly like an uninitialized one (InvalidState).
	void SetRenderer(const IRenderer* renderer) noexcept { m_renderer = renderer; }
	const IRenderer* GetRenderer() const noexcept { return m_renderer; }

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

private:
	// One retained resource: the handle and the asset's numbers, never the
	// asset's bytes. The vectors are the registry -- small, insertion
	// ordered, linearly searched, released on Release or by the shutdown
	// policy above. This is handle bookkeeping, not a byte cache.
	struct ImageEntry
	{
		ImageResourceHandle handle;
		ImageResourceInfo   info;
	};

	struct MeshEntry
	{
		MeshResourceHandle handle;
		MeshResourceInfo   info;
	};

	// The observed renderer's state, or Uninitialized when none is attached.
	RendererState PeekState() const noexcept;

	// The lifecycle column of the table above. The mutating variant also
	// applies the shutdown policy (drops retained entries the first time it
	// observes ShutdownState); the const variant answers without touching
	// them, because IsValid/counts/info are reads.
	Status CheckLifecycle() noexcept;
	Status CheckLifecycleConst() const noexcept;
	bool IsLive() const noexcept;

	ImageEntry* FindImage(ImageResourceHandle handle) noexcept;
	const ImageEntry* FindImage(ImageResourceHandle handle) const noexcept;
	MeshEntry* FindMesh(MeshResourceHandle handle) noexcept;
	const MeshEntry* FindMesh(MeshResourceHandle handle) const noexcept;

	const IRenderer*          m_renderer    = nullptr;
	uint64_t                  m_nextImageId = 1;
	uint64_t                  m_nextMeshId  = 1;
	std::vector<ImageEntry>   m_images;
	std::vector<MeshEntry>    m_meshes;
};

} // namespace Modern::Client
