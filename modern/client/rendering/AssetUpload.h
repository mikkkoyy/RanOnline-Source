#pragma once

// CLIENT-009: the renderer-facing CPU asset upload contract.
//
// This is the seam where the two halves of the modern client meet for the
// first time. The asset layer (CLIENT-007 images, CLIENT-008 meshes) produces
// validated CPU-side values; CLIENT-004 drew the renderer boundary. Neither
// side could talk to the other. This header is the contract between them:
// hand it an ImageAsset or a MeshAsset, get back an opaque handle naming the
// renderer resource that value became. One interface, two handle types, one
// headless implementation -- no device, no graphics API, no GPU.
//
// What the contract says, and what it refuses to say:
//
//   - **The renderer receives CPU assets, not storage.** UploadImage takes an
//     ImageAsset; UploadMesh takes a MeshAsset. There is no ResourceId,
//     ResourceData, ResourceManager or IResourceProvider anywhere in this
//     header. Turning bytes into an asset is the decoder's job (CLIENT-007 /
//     CLIENT-008), and turning bytes into a cache entry is the resource
//     layer's job (CLIENT-005 / CLIENT-006). A renderer that could Load()
//     resources would quietly become a resource loader, and this boundary is
//     what stops that.
//
//   - **Handles are opaque.** An ImageResourceHandle is an 8-byte strongly
//     typed id (Modern's detail::StrongId): cheap to copy, trivially
//     copyable, with an explicit invalid state (the core's all-ones
//     sentinel). It contains no pointer, so there is no
//     IDirect3DTexture9, ID3D11Texture2D, ID3D11Buffer, VkImage, VkBuffer,
//     GLuint, HWND or device pointer to leak -- and no <Windows.h> behind
//     this include. Whether the resource a handle names ever *becomes* a GPU
//     object, and which API makes it one, is entirely the future backend's
//     business.
//
//   - **Assets never learn any of this.** No asset header includes this one.
//     The edge is one-way: uploading consumes assets, and assets know
//     nothing about uploading. ModernClientAssets still links Modern and
//     ModernClientResources only -- it compiles without this target, without
//     ModernClientRendering, and without a renderer existing anywhere.
//
//   - **Lifecycle is the renderer's, not a second copy of it.** This
//     interface has no Initialize() and no Shutdown(). The provided
//     implementation observes a borrowed IRenderer and answers with the same
//     RendererState / ErrorCode conventions IRenderer itself uses
//     (InvalidState from a live-but-wrong state, NotAllowed from the
//     terminal ShutdownState), so there is no second state machine that can
//     drift out of step with the first.
//
// The codes every method can produce, in decision order (lifecycle first,
// then handle shape, then the operation itself):
//
//   Uninitialized renderer / no renderer attached   -> InvalidState
//   ShutdownState (terminal)                        -> NotAllowed
//   the invalid handle sentinel as an argument      -> InvalidArgument
//   an id that names nothing live                   -> NotFound
//   otherwise                                       -> Ok
//
// CLIENT-009 establishes the renderer asset contract only. It does not
// implement a real graphics backend.

#include "assets/ImageAsset.h"
#include "assets/MeshAsset.h"
#include "assets/AssetTypes.h"
#include "types/Ids.h"
#include "types/Result.h"

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace Modern::Client
{

// Opaque identity of an uploaded image resource, assigned by an
// IAssetUploader.
//
// The whole handle contract:
//
//   - Default-constructed is *invalid* (StrongId's all-ones sentinel, the
//     same convention as the core's EntityId / ItemId). IsValid() is the
//     explicit invalid-state check.
//   - Cheap to copy: one uint64 of integer, no vtable, no resource object.
//   - Deterministic identity: an uploader assigns ids 1, 2, 3, ... per asset
//     kind, in upload order, from a fresh instance. The same sequence of
//     uploads produces the same handles in every run, and ids are never
//     recycled after a release.
//   - Meaningful only to the uploader that issued it. A fabricated id names
//     nothing and is refused (NotFound; InvalidArgument for the sentinel),
//     so a handle cannot be forged into a resource by picking a number.
//   - Distinct from MeshResourceHandle by construction: the two are
//     unrelated types, so an image handle cannot be released as a mesh.
using ImageResourceHandle = detail::StrongId<struct ImageResourceHandleTag, uint64_t>;

// Opaque identity of an uploaded mesh resource. Identical semantics to
// ImageResourceHandle; a separate type so the two id spaces cannot be mixed.
using MeshResourceHandle = detail::StrongId<struct MeshResourceHandleTag, uint64_t>;

// The opacity claims, checked where every consumer sees them: a handle is a
// trivial 8-byte value, it is not a pointer, it is not implicitly forged
// from an integer, and the two kinds do not convert into each other.
static_assert(std::is_trivially_copyable<ImageResourceHandle>::value,
	"an image resource handle must be a cheap value");
static_assert(sizeof(ImageResourceHandle) == sizeof(uint64_t),
	"an image resource handle is one integer and nothing else");
static_assert(std::is_trivially_copyable<MeshResourceHandle>::value,
	"a mesh resource handle must be a cheap value");
static_assert(sizeof(MeshResourceHandle) == sizeof(uint64_t),
	"a mesh resource handle is one integer and nothing else");
static_assert(!std::is_convertible<ImageResourceHandle, MeshResourceHandle>::value &&
	!std::is_convertible<MeshResourceHandle, ImageResourceHandle>::value,
	"image and mesh handles are unrelated types");
static_assert(!std::is_convertible<ImageResourceHandle, const void*>::value &&
	!std::is_convertible<MeshResourceHandle, const void*>::value,
	"a handle must never decay into a pointer to a backend object");
static_assert(!std::is_convertible<uint64_t, ImageResourceHandle>::value,
	"handles are issued by an uploader, not forged by the caller");

// Everything the upload boundary retained about an uploaded image: metadata,
// and deliberately *not* the pixels.
//
// The uploader is not a second ResourceManager. It holds no bytes that a
// decoder already produced and no bytes that a manager already cached -- only
// the numbers a test (or a future backend) needs to prove the asset survived
// the crossing. The asset itself is read during UploadImage and not retained.
struct ImageResourceInfo
{
	uint32_t    width     = 0;
	uint32_t    height    = 0;
	ImageFormat format    = ImageFormat::Unknown;
	size_t      byteCount = 0;  // packed pixel bytes, as the asset reported them
};

// Same rule for geometry: counts and topology, not the vertices.
struct MeshResourceInfo
{
	size_t            vertexCount   = 0;
	size_t            indexCount    = 0;
	size_t            triangleCount = 0;
	PrimitiveTopology topology      = PrimitiveTopology::Unknown;
	size_t            byteCount     = 0;  // vertex bytes + index bytes
};

// The renderer asset upload boundary (CLIENT-009).
//
// Consumes validated CPU assets and produces opaque resource handles. It
// exposes no GPU API object, no backend type, no storage type and no
// lifecycle of its own: an implementation derives "may I run at all?" from
// the renderer it observes, which is why the table in AssetUpload.h speaks
// in RendererState terms.
//
// The provided headless implementation is NullAssetUploader (this
// directory). A future GPU backend implements this same interface behind the
// same handles; nothing above it -- the loaders, the decoders, the asset
// types -- changes when that happens, which is the entire point of the
// boundary.
class IAssetUploader
{
public:
	virtual ~IAssetUploader() = default;

	// Uploads a decoded CPU asset, returning the handle that names the
	// renderer resource it became.
	//
	// The asset must already exist as a value: ImageAsset and MeshAsset have
	// no default constructor and Create() is their only way to a value, so
	// "an invalid asset was uploaded" is not a state this method can observe
	// -- the caller can only ever hand over a validated asset. What this
	// method *can* refuse is the lifecycle around it (InvalidState /
	// NotAllowed per the table above).
	//
	// On success the asset's metadata is retained; its bytes are not. The
	// caller keeps owning the asset and may release it immediately.
	virtual Result<ImageResourceHandle> UploadImage(const ImageAsset& image) = 0;
	virtual Result<MeshResourceHandle> UploadMesh(const MeshAsset& mesh) = 0;

	// True only while the renderer is live *and* this uploader still holds
	// the resource. False for an invalid, released, unknown or
	// post-shutdown handle alike: false is the whole answer to "is this
	// usable", with no error channel to misuse.
	virtual bool IsValidImage(ImageResourceHandle handle) const noexcept = 0;
	virtual bool IsValidMesh(MeshResourceHandle handle) const noexcept = 0;

	// Destroys a live resource. A live handle returns Ok; anything else
	// follows the decision table (lifecycle, then InvalidArgument for the
	// sentinel, then NotFound for an unknown or already-released id).
	//
	// Destroying twice is deterministic rather than harmful: the first call
	// releases, the second finds nothing and reports NotFound. Destroying
	// after renderer shutdown reports NotAllowed -- lifecycle is checked
	// before the handle, so a shut-down renderer refuses everything
	// uniformly.
	virtual Status ReleaseImage(ImageResourceHandle handle) noexcept = 0;
	virtual Status ReleaseMesh(MeshResourceHandle handle) noexcept = 0;

	// The metadata retained at upload time. Same decision order as Release.
	virtual Result<ImageResourceInfo> GetImageInfo(ImageResourceHandle handle) const = 0;
	virtual Result<MeshResourceInfo> GetMeshInfo(MeshResourceHandle handle) const = 0;

	// Live resource counts. Zero whenever the renderer is not live, so a
	// shut-down uploader reads empty without being told to.
	virtual size_t GetLiveImageCount() const noexcept = 0;
	virtual size_t GetLiveMeshCount() const noexcept = 0;
};

} // namespace Modern::Client
