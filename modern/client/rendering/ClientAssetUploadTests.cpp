// CLIENT-009: renderer asset upload boundary tests.
//
// The upload boundary turns validated CPU assets into opaque renderer
// handles, headlessly. These cases pin down every column of the contract
// AssetUpload.h states: handle semantics, image upload, mesh upload, both
// end-to-end pipelines, and the separation rules the milestone exists to
// protect.
//
// Coverage:
//  1.  A default-constructed handle is the explicit invalid state.
//  2.  Handle equality and ordering are deterministic; the two handle kinds
//      are unrelated types.
//  3.  A handle is a trivially copyable 8-byte value -- never a pointer.
//  4.  Deterministic identity: the same uploads in a fresh uploader produce
//      the same ids (1, 2, 3 ... per kind), across instances.
//  5.  Multiple uploads receive distinct handles; live counts agree.
//  6.  Image upload preserves every piece of metadata.
//  7.  Upload before renderer initialization fails deterministically.
//  8.  Upload with no renderer attached fails deterministically
//      (SetRenderer round-trip included).
//  9.  Upload is allowed while InFrame: uploading is not a frame operation.
// 10.  An invalid ImageAsset cannot exist to be uploaded: Create() refusals.
// 11.  Releasing a valid image handle works and removes its metadata.
// 12.  Releasing the sentinel is InvalidArgument; unknown ids are NotFound.
// 13.  Double destruction is deterministic: Ok, then NotFound, then NotFound.
// 14.  Renderer shutdown invalidates image handles immediately and refuses
//      every subsequent call with NotAllowed.
// 15.  Released ids are never recycled.
// 16.  Mesh upload preserves vertex count, index count, triangle count,
//      topology and byte count.
// 17.  Mesh upload before renderer initialization fails deterministically.
// 18.  An invalid MeshAsset cannot exist to be uploaded: Create() refusals.
// 19.  Multiple meshes receive distinct handles; releasing one leaves the
//      others live.
// 20.  Releasing a valid mesh handle works.
// 21.  Mesh release of sentinel, unknown and already-released handles.
// 22.  Renderer shutdown invalidates mesh handles, deterministically.
// 23.  Pipeline: provider -> ResourceManager -> ResourceData ->
//      TestImageDecoder -> ImageAsset -> IAssetUploader -> handle.
// 24.  Pipeline: provider -> ResourceManager -> ResourceData ->
//      TestMeshDecoder -> MeshAsset -> IAssetUploader -> handle.
// 25.  Separation: the API carries assets and handles and no storage type;
//      the boundary is an interface with a headless leaf implementation.
// 26.  Separation: Get*Info refusals follow the documented decision order.
//
// Interpreting "an invalid asset cannot be uploaded": ImageAsset and
// MeshAsset have no default constructor and Create() is their only way to a
// value, so an invalid one cannot be *constructed* -- the cases above prove
// that structurally (static_assert) and by exercising every Create()
// refusal, which is the strongest statement the types allow. The uploader
// therefore only ever sees validated assets, and what it *can* refuse is the
// lifecycle around them.
//
// No GPU, no window, no RAN installation: every byte here is assembled by
// hand from the documented MIMG / MMESH test containers.

// The boundary under test, included first and alone, so the platform guards
// below measure exactly its transitive includes.
#include "rendering/AssetUpload.h"
#include "rendering/NullAssetUploader.h"

// The upload boundary must not reach the platform or any graphics API.
// Checked at compile time on the boundary's own include chain:
//   _WINDOWS_      -> <windows.h> (and MFC / Direct3D headers cannot come
//                     in without it, so this one macro covers that family)
//   VK_VERSION_1_0 -> <vulkan_core.h>
//   __gl_h_        -> <GL/gl.h>
#ifdef _WINDOWS_
#error "the renderer asset upload boundary must not include <Windows.h>"
#endif
#ifdef VK_VERSION_1_0
#error "the renderer asset upload boundary must not include Vulkan"
#endif
#ifdef __gl_h_
#error "the renderer asset upload boundary must not include OpenGL"
#endif

#include "TestHarness.h"

#include "assets/AssetTypes.h"
#include "assets/ImageAsset.h"
#include "assets/MeshAsset.h"
#include "assets/TestImageDecoder.h"
#include "assets/TestMeshDecoder.h"
#include "rendering/NullRenderer.h"
#include "rendering/Renderer.h"
#include "rendering/RenderingTypes.h"
#include "resources/MemoryResourceProvider.h"
#include "resources/ResourceData.h"
#include "resources/ResourceId.h"
#include "resources/ResourceManager.h"
#include "types/Result.h"

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <cstring>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using namespace Modern;
using namespace Modern::Client;

namespace
{
	// ---------------------------------------------------------------------------
	// Fixtures
	// ---------------------------------------------------------------------------

	// The MIMG container described in assets/TestImageDecoder.h, assembled by
	// hand: a ten-byte header followed by width * height * bytes per pixel of
	// deterministic pixel bytes.
	std::vector<uint8_t> MakeImageBytes(uint16_t width, uint16_t height, ImageFormat format)
	{
		std::vector<uint8_t> bytes;
		bytes.push_back('M');
		bytes.push_back('I');
		bytes.push_back('M');
		bytes.push_back('G');
		bytes.push_back(TestImageDecoder::kVersion);
		bytes.push_back(static_cast<uint8_t>(format));
		bytes.push_back(static_cast<uint8_t>(width & 0xFFu));
		bytes.push_back(static_cast<uint8_t>((width >> 8) & 0xFFu));
		bytes.push_back(static_cast<uint8_t>(height & 0xFFu));
		bytes.push_back(static_cast<uint8_t>((height >> 8) & 0xFFu));

		const size_t pixelBytes = static_cast<size_t>(width) * static_cast<size_t>(height) *
			static_cast<size_t>(BytesPerPixel(format));
		for (size_t i = 0; i < pixelBytes; ++i)
		{
			bytes.push_back(static_cast<uint8_t>(i & 0xFFu));
		}
		return bytes;
	}

	// The MMESH container described in assets/TestMeshDecoder.h: header, four
	// vertices (px, py, pz, nx, ny, nz, u, v as little endian float32) and six
	// uint32 indices forming two triangles.
	std::vector<uint8_t> MakeMeshBytes()
	{
		auto pushU32 = [](std::vector<uint8_t>& bytes, uint32_t value)
		{
			bytes.push_back(static_cast<uint8_t>(value & 0xFFu));
			bytes.push_back(static_cast<uint8_t>((value >> 8) & 0xFFu));
			bytes.push_back(static_cast<uint8_t>((value >> 16) & 0xFFu));
			bytes.push_back(static_cast<uint8_t>((value >> 24) & 0xFFu));
		};
		auto pushF32 = [&pushU32](std::vector<uint8_t>& bytes, float value)
		{
			uint32_t bits = 0;
			std::memcpy(&bits, &value, sizeof(bits));
			pushU32(bytes, bits);
		};

		std::vector<uint8_t> bytes;
		bytes.push_back('M');
		bytes.push_back('E');
		bytes.push_back('S');
		bytes.push_back('H');
		bytes.push_back(TestMeshDecoder::kVersion);
		bytes.push_back(static_cast<uint8_t>(MeshVertexFormat::PositionNormalUvF32));
		pushU32(bytes, 4);  // vertex count
		pushU32(bytes, 6);  // index count

		const float positions[4][3] = {
			{ 0.0f, 0.0f, 0.0f },
			{ 1.0f, 0.0f, 0.0f },
			{ 1.0f, 1.0f, 0.0f },
			{ 0.0f, 1.0f, 0.0f },
		};
		const float uvs[4][2] = {
			{ 0.0f, 0.0f },
			{ 1.0f, 0.0f },
			{ 1.0f, 1.0f },
			{ 0.0f, 1.0f },
		};
		for (int vertex = 0; vertex < 4; ++vertex)
		{
			pushF32(bytes, positions[vertex][0]);
			pushF32(bytes, positions[vertex][1]);
			pushF32(bytes, positions[vertex][2]);
			pushF32(bytes, 0.0f);   // normal x
			pushF32(bytes, 0.0f);   // normal y
			pushF32(bytes, 1.0f);   // normal z
			pushF32(bytes, uvs[vertex][0]);
			pushF32(bytes, uvs[vertex][1]);
		}
		const uint32_t indices[6] = { 0, 1, 2, 0, 2, 3 };
		for (uint32_t index : indices)
		{
			pushU32(bytes, index);
		}
		return bytes;
	}

	// A valid image built directly through Create(): the upload tests need a
	// decoded CPU asset, and this is the shortest path to one that involves
	// no storage layer at all. 4x2 RGBA = 32 pixel bytes.
	ImageAsset MakeTestImage(uint32_t width = 4, uint32_t height = 2)
	{
		std::vector<uint8_t> pixels(static_cast<size_t>(width) * static_cast<size_t>(height) * 4, 0x5A);
		const Result<ImageAsset> asset =
			ImageAsset::Create(width, height, ImageFormat::R8G8B8A8_UNorm, std::move(pixels));
		CHECK(asset.IsOk());
		return asset.GetValue();
	}

	// A valid mesh built directly through Create(): four vertices, six indices,
	// two triangles -- 4 * 32 + 6 * 4 = 152 bytes of geometry.
	MeshAsset MakeTestMesh()
	{
		std::vector<MeshVertex> vertices = {
			MeshVertex(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), 0.0f, 0.0f),
			MeshVertex(Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), 1.0f, 0.0f),
			MeshVertex(Vector3(1.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), 1.0f, 1.0f),
			MeshVertex(Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), 0.0f, 1.0f),
		};
		std::vector<MeshIndex> indices = { 0, 1, 2, 0, 2, 3 };
		const Result<MeshAsset> asset =
			MeshAsset::Create(PrimitiveTopology::TriangleList, std::move(vertices), std::move(indices));
		CHECK(asset.IsOk());
		return asset.GetValue();
	}

	// A ResourceId for the pipeline cases; a failed Create would be a fixture
	// bug, so it is checked here rather than at every call site.
	ResourceId MakeId(const char* name)
	{
		const Result<ResourceId> id = ResourceId::Create(name);
		CHECK(id.IsOk());
		return id.GetValue();
	}

	// An initialized headless renderer, the state every "happy path" case
	// starts from.
	void InitializeRenderer(NullRenderer& renderer)
	{
		CHECK(renderer.Initialize(RendererConfig{}).IsOk());
	}
}

// ---------------------------------------------------------------------------
// Handles
// ---------------------------------------------------------------------------

MODERN_TEST(ClientUpload_DefaultHandleIsInvalid)
{
	// The explicit invalid state: default construction is the sentinel, and
	// IsValid() is the only question a caller has to ask.
	const ImageResourceHandle invalidImage;
	const MeshResourceHandle invalidMesh;

	CHECK(!invalidImage.IsValid());
	CHECK(!invalidMesh.IsValid());
	CHECK(invalidImage == ImageResourceHandle::MakeInvalid());
	CHECK(invalidMesh == MeshResourceHandle::MakeInvalid());
	CHECK_EQ(invalidImage.Get(), ImageResourceHandle::InvalidValue);
	CHECK_EQ(invalidMesh.Get(), MeshResourceHandle::InvalidValue);
	CHECK(!static_cast<bool>(invalidImage));
}

MODERN_TEST(ClientUpload_HandleComparisonsAreDeterministic)
{
	// Identity semantics: equal ids compare equal in either construction
	// order, distinct ids do not, and StrongId's ordering is stable -- which
	// is what lets handles be used directly as ordered container keys by the
	// backend that eventually owns them.
	const ImageResourceHandle first(static_cast<uint64_t>(1));
	const ImageResourceHandle second(static_cast<uint64_t>(2));
	const ImageResourceHandle firstCopy = first;

	CHECK(first == firstCopy);
	CHECK(first != second);
	CHECK(first < second);
	CHECK(second > first);
	CHECK(firstCopy <= first);
	CHECK(firstCopy >= first);

	// The same number in the other kind is still the other kind: image and
	// mesh id spaces are unrelated types (non-convertibility is
	// static_asserted in AssetUpload.h), so releasing one as the other does
	// not compile in the first place.
	const ImageResourceHandle image(static_cast<uint64_t>(7));
	const MeshResourceHandle mesh(static_cast<uint64_t>(7));
	CHECK(image.IsValid());
	CHECK(mesh.IsValid());
}

MODERN_TEST(ClientUpload_HandleIsATrivialCopyableValue)
{
	// Opaqueness and cheapness, asserted rather than described.
	static_assert(std::is_trivially_copyable<ImageResourceHandle>::value,
		"a handle must be a cheap value");
	static_assert(std::is_trivially_copyable<MeshResourceHandle>::value,
		"a handle must be a cheap value");
	static_assert(std::is_standard_layout<ImageResourceHandle>::value,
		"a handle must be a plain value, not a wrapper with hidden state");
	static_assert(sizeof(ImageResourceHandle) == 8, "one integer, nothing else");
	static_assert(sizeof(MeshResourceHandle) == 8, "one integer, nothing else");
	static_assert(!std::is_pointer<ImageResourceHandle>::value,
		"a handle is not a pointer to a backend object");
	static_assert(!std::is_convertible<ImageResourceHandle, const void*>::value,
		"a handle must never decay into a pointer");

	// Copy and move preserve identity. A handle is a value, not a capability
	// that is consumed: moving one leaves the source still holding its
	// number, and no destructor on the type can release anything behind the
	// uploader's back.
	ImageResourceHandle original(static_cast<uint64_t>(42));
	const ImageResourceHandle copied(original);
	ImageResourceHandle moved(std::move(original));
	ImageResourceHandle assigned = copied;

	CHECK(original == copied);
	CHECK(moved == copied);
	CHECK(assigned == copied);
	CHECK_EQ(copied.Get(), static_cast<uint64_t>(42));
}

MODERN_TEST(ClientUpload_DeterministicHandleIdentity)
{
	// The same uploads, from a fresh uploader, produce the same handles: ids
	// run 1, 2, 3 ... per asset kind, in upload order, from a fresh
	// instance. Two instances therefore produce identical sequences, which
	// is what makes an emulator run comparable to a test run.
	NullRenderer rendererA;
	InitializeRenderer(rendererA);
	NullAssetUploader uploaderA(&rendererA);

	NullRenderer rendererB;
	InitializeRenderer(rendererB);
	NullAssetUploader uploaderB(&rendererB);

	const ImageAsset image = MakeTestImage();
	const MeshAsset mesh = MakeTestMesh();

	const Result<ImageResourceHandle> a1 = uploaderA.UploadImage(image);
	const Result<ImageResourceHandle> a2 = uploaderA.UploadImage(image);
	const Result<MeshResourceHandle> am = uploaderA.UploadMesh(mesh);
	const Result<ImageResourceHandle> b1 = uploaderB.UploadImage(image);
	const Result<ImageResourceHandle> b2 = uploaderB.UploadImage(image);
	const Result<MeshResourceHandle> bm = uploaderB.UploadMesh(mesh);

	CHECK(a1.IsOk() && a2.IsOk() && am.IsOk());
	CHECK(b1.IsOk() && b2.IsOk() && bm.IsOk());
	if (a1.IsError() || a2.IsError() || am.IsError() ||
		b1.IsError() || b2.IsError() || bm.IsError())
	{
		return;
	}

	CHECK_EQ(a1.GetValue().Get(), static_cast<uint64_t>(1));
	CHECK_EQ(a2.GetValue().Get(), static_cast<uint64_t>(2));
	// The mesh sequence is its own: image and mesh ids are unrelated kinds,
	// each counting from 1.
	CHECK_EQ(am.GetValue().Get(), static_cast<uint64_t>(1));
	CHECK(a1.GetValue() == b1.GetValue());
	CHECK(a2.GetValue() == b2.GetValue());
	CHECK(am.GetValue() == bm.GetValue());
}

MODERN_TEST(ClientUpload_UploadsReceiveDistinctHandles)
{
	NullRenderer renderer;
	InitializeRenderer(renderer);
	NullAssetUploader uploader(&renderer);

	const ImageAsset image = MakeTestImage();
	const MeshAsset mesh = MakeTestMesh();

	const Result<ImageResourceHandle> first = uploader.UploadImage(image);
	const Result<ImageResourceHandle> second = uploader.UploadImage(image);
	const Result<ImageResourceHandle> third = uploader.UploadImage(image);
	const Result<MeshResourceHandle> meshOne = uploader.UploadMesh(mesh);
	const Result<MeshResourceHandle> meshTwo = uploader.UploadMesh(mesh);

	CHECK(first.IsOk() && second.IsOk() && third.IsOk());
	CHECK(meshOne.IsOk() && meshTwo.IsOk());
	if (first.IsError() || second.IsError() || third.IsError() ||
		meshOne.IsError() || meshTwo.IsError())
	{
		return;
	}

	CHECK(first.GetValue() != second.GetValue());
	CHECK(second.GetValue() != third.GetValue());
	CHECK(first.GetValue() != third.GetValue());
	CHECK(meshOne.GetValue() != meshTwo.GetValue());

	CHECK_EQ(uploader.GetLiveImageCount(), static_cast<size_t>(3));
	CHECK_EQ(uploader.GetLiveMeshCount(), static_cast<size_t>(2));
}

// ---------------------------------------------------------------------------
// Image resource upload
// ---------------------------------------------------------------------------

MODERN_TEST(ClientUpload_ImageUploadPreservesMetadata)
{
	NullRenderer renderer;
	InitializeRenderer(renderer);
	NullAssetUploader uploader(&renderer);
	CHECK_EQ(uploader.GetLiveImageCount(), static_cast<size_t>(0));

	const ImageAsset image = MakeTestImage(4, 2);
	const Result<ImageResourceHandle> handle = uploader.UploadImage(image);
	CHECK(handle.IsOk());
	if (handle.IsError())
	{
		return;
	}

	CHECK(handle.GetValue().IsValid());
	CHECK(uploader.IsValidImage(handle.GetValue()));
	CHECK_EQ(uploader.GetLiveImageCount(), static_cast<size_t>(1));

	const Result<ImageResourceInfo> info = uploader.GetImageInfo(handle.GetValue());
	CHECK(info.IsOk());
	if (info.IsError())
	{
		return;
	}

	// Every number the asset reported survives the crossing. The pixels do
	// not travel with it: the uploader retained the metadata, and the asset
	// itself is still owned by this scope.
	CHECK_EQ(info.GetValue().width, image.GetWidth());
	CHECK_EQ(info.GetValue().height, image.GetHeight());
	CHECK_EQ(info.GetValue().format, image.GetFormat());
	CHECK_EQ(info.GetValue().byteCount, image.GetPixelByteCount());
	CHECK_EQ(info.GetValue().width, 4u);
	CHECK_EQ(info.GetValue().height, 2u);
	CHECK_EQ(info.GetValue().format, ImageFormat::R8G8B8A8_UNorm);
	CHECK_EQ(info.GetValue().byteCount, static_cast<size_t>(32));
}

MODERN_TEST(ClientUpload_ImageUploadRequiresInitializedRenderer)
{
	NullRenderer renderer;
	CHECK_EQ(renderer.GetState(), RendererState::Uninitialized);
	NullAssetUploader uploader(&renderer);

	// Upload before initialization: deterministic failure, no resource
	// half-created behind the error.
	const Result<ImageResourceHandle> beforeInit = uploader.UploadImage(MakeTestImage());
	CHECK(!beforeInit.IsOk());
	CHECK_EQ(beforeInit.GetError(), ErrorCode::InvalidState);
	CHECK_EQ(uploader.GetLiveImageCount(), static_cast<size_t>(0));
	CHECK(!uploader.IsValidImage(ImageResourceHandle()));

	// The same uploader flips to success once the renderer is live -- there
	// is no uploader Initialize() to mirror the renderer's, because the
	// renderer's state *is* the lifecycle.
	InitializeRenderer(renderer);
	const Result<ImageResourceHandle> afterInit = uploader.UploadImage(MakeTestImage());
	CHECK(afterInit.IsOk());
	CHECK_EQ(uploader.GetLiveImageCount(), static_cast<size_t>(1));
}

MODERN_TEST(ClientUpload_ImageUploadWithoutRendererIsInvalidState)
{
	// No renderer attached at all: the documented "detached ==
	// uninitialized" rule, reachable through the default constructor.
	NullAssetUploader uploader;
	CHECK(uploader.GetRenderer() == nullptr);

	const Result<ImageResourceHandle> detached = uploader.UploadImage(MakeTestImage());
	CHECK(!detached.IsOk());
	CHECK_EQ(detached.GetError(), ErrorCode::InvalidState);
	CHECK_EQ(uploader.GetLiveImageCount(), static_cast<size_t>(0));

	// SetRenderer attaches (and detaches) the borrowed observer; the
	// uploader still owns nothing and shuts nothing down.
	NullRenderer renderer;
	uploader.SetRenderer(&renderer);
	CHECK(uploader.GetRenderer() == &renderer);
	CHECK_EQ(uploader.UploadImage(MakeTestImage()).GetError(), ErrorCode::InvalidState);

	InitializeRenderer(renderer);
	CHECK(uploader.UploadImage(MakeTestImage()).IsOk());
	CHECK_EQ(uploader.GetLiveImageCount(), static_cast<size_t>(1));
}

MODERN_TEST(ClientUpload_ImageUploadAllowedWhileInFrame)
{
	// Uploading mid-frame is legal: IsInitialized covers InFrame, so the
	// CLIENT-004 frame loop never has to end a frame to load a texture.
	NullRenderer renderer;
	InitializeRenderer(renderer);
	CHECK(renderer.BeginFrame().IsOk());

	NullAssetUploader uploader(&renderer);
	const Result<ImageResourceHandle> handle = uploader.UploadImage(MakeTestImage());
	CHECK(handle.IsOk());
	if (handle.IsError())
	{
		CHECK(renderer.EndFrame().IsOk());
		return;
	}

	CHECK(uploader.IsValidImage(handle.GetValue()));
	CHECK(renderer.EndFrame().IsOk());
	// Still live after the frame completed: uploading inside a frame did
	// not tie the resource's lifetime to that frame.
	CHECK(uploader.IsValidImage(handle.GetValue()));
	CHECK(uploader.ReleaseImage(handle.GetValue()).IsOk());
}

MODERN_TEST(ClientUpload_InvalidImageAssetCannotExist)
{
	// "An invalid ImageAsset cannot be uploaded" is enforced one step
	// earlier than the uploader: there is no invalid ImageAsset to hand it.
	// Create() is the only way to a value, it re-runs every check, and no
	// default constructor produces an unchecked one.
	static_assert(!std::is_default_constructible<ImageAsset>::value,
		"an asset must not be constructible without validation");
	static_assert(!std::is_default_constructible<MeshAsset>::value,
		"an asset must not be constructible without validation");

	// Every refusal, with the code the asset layer documents.
	CHECK_EQ(ImageAsset::Create(0, 2, ImageFormat::R8G8B8A8_UNorm, {}).GetError(),
		ErrorCode::InvalidArgument);
	CHECK_EQ(ImageAsset::Create(4, 0, ImageFormat::R8G8B8A8_UNorm, {}).GetError(),
		ErrorCode::InvalidArgument);
	CHECK_EQ(ImageAsset::Create(2, 2, ImageFormat::Unknown, std::vector<uint8_t>(16, 0)).GetError(),
		ErrorCode::InvalidArgument);
	CHECK_EQ(ImageAsset::Create(2, 2, ImageFormat::R8G8B8A8_UNorm, std::vector<uint8_t>(15, 0)).GetError(),
		ErrorCode::InvalidArgument);
	CHECK_EQ(ImageAsset::Create(2, 2, ImageFormat::R8G8B8A8_UNorm, std::vector<uint8_t>(17, 0)).GetError(),
		ErrorCode::InvalidArgument);
}

// ---------------------------------------------------------------------------
// Image release and shutdown
// ---------------------------------------------------------------------------

MODERN_TEST(ClientUpload_ReleaseImageHandleWorks)
{
	NullRenderer renderer;
	InitializeRenderer(renderer);
	NullAssetUploader uploader(&renderer);

	const Result<ImageResourceHandle> handle = uploader.UploadImage(MakeTestImage());
	CHECK(handle.IsOk());
	if (handle.IsError())
	{
		return;
	}

	CHECK(uploader.ReleaseImage(handle.GetValue()).IsOk());

	// The handle is spent and the metadata went with it.
	CHECK(!uploader.IsValidImage(handle.GetValue()));
	CHECK_EQ(uploader.GetLiveImageCount(), static_cast<size_t>(0));
	CHECK_EQ(uploader.GetImageInfo(handle.GetValue()).GetError(), ErrorCode::NotFound);
}

MODERN_TEST(ClientUpload_ReleaseOfInvalidAndUnknownImageHandle)
{
	NullRenderer renderer;
	InitializeRenderer(renderer);
	NullAssetUploader uploader(&renderer);

	const Result<ImageResourceHandle> live = uploader.UploadImage(MakeTestImage());
	CHECK(live.IsOk());
	if (live.IsError())
	{
		return;
	}

	// The sentinel is an invalid *argument*, not a lookup miss: the caller
	// handed over a handle that can never name anything.
	CHECK_EQ(uploader.ReleaseImage(ImageResourceHandle()).GetCode(),
		ErrorCode::InvalidArgument);

	// A well-formed id that names nothing is a lookup miss.
	CHECK_EQ(uploader.ReleaseImage(ImageResourceHandle(static_cast<uint64_t>(9999))).GetCode(),
		ErrorCode::NotFound);

	// Neither refusal touched the live resource.
	CHECK(uploader.IsValidImage(live.GetValue()));
	CHECK_EQ(uploader.GetLiveImageCount(), static_cast<size_t>(1));
}

MODERN_TEST(ClientUpload_DoubleReleaseImageHandleIsDeterministic)
{
	NullRenderer renderer;
	InitializeRenderer(renderer);
	NullAssetUploader uploader(&renderer);

	const Result<ImageResourceHandle> handle = uploader.UploadImage(MakeTestImage());
	CHECK(handle.IsOk());
	if (handle.IsError())
	{
		return;
	}

	// First release consumes the handle; every later release finds nothing
	// and says so. No crash, no underflow, no double-free shape.
	CHECK(uploader.ReleaseImage(handle.GetValue()).IsOk());
	CHECK_EQ(uploader.ReleaseImage(handle.GetValue()).GetCode(), ErrorCode::NotFound);
	CHECK_EQ(uploader.ReleaseImage(handle.GetValue()).GetCode(), ErrorCode::NotFound);
	CHECK_EQ(uploader.GetLiveImageCount(), static_cast<size_t>(0));
}

MODERN_TEST(ClientUpload_ShutdownInvalidatesImageResources)
{
	NullRenderer renderer;
	InitializeRenderer(renderer);
	NullAssetUploader uploader(&renderer);

	const Result<ImageResourceHandle> first = uploader.UploadImage(MakeTestImage());
	const Result<ImageResourceHandle> second = uploader.UploadImage(MakeTestImage());
	CHECK(first.IsOk() && second.IsOk());
	if (first.IsError() || second.IsError())
	{
		return;
	}
	CHECK_EQ(uploader.GetLiveImageCount(), static_cast<size_t>(2));

	// The renderer shuts down while handles exist. Documented policy:
	// every handle becomes invalid *immediately* -- answered from the
	// renderer state, before anything looks inside the registry -- and the
	// retained metadata is dropped at the first mutating call afterwards.
	renderer.Shutdown();

	CHECK(!uploader.IsValidImage(first.GetValue()));
	CHECK(!uploader.IsValidImage(second.GetValue()));
	CHECK_EQ(uploader.GetLiveImageCount(), static_cast<size_t>(0));
	CHECK_EQ(uploader.GetImageInfo(first.GetValue()).GetError(), ErrorCode::NotAllowed);

	// Lifecycle precedes the handle: everything is refused uniformly, even
	// the ids that were live a moment ago.
	CHECK_EQ(uploader.UploadImage(MakeTestImage()).GetError(), ErrorCode::NotAllowed);
	CHECK_EQ(uploader.ReleaseImage(first.GetValue()).GetCode(), ErrorCode::NotAllowed);
	// The second mutating call sees the same code -- the registry was
	// already dropped by the first.
	CHECK_EQ(uploader.ReleaseImage(second.GetValue()).GetCode(), ErrorCode::NotAllowed);
}

MODERN_TEST(ClientUpload_ReleasedImageIdIsNotRecycled)
{
	NullRenderer renderer;
	InitializeRenderer(renderer);
	NullAssetUploader uploader(&renderer);

	const Result<ImageResourceHandle> first = uploader.UploadImage(MakeTestImage());
	const Result<ImageResourceHandle> second = uploader.UploadImage(MakeTestImage());
	CHECK(first.IsOk() && second.IsOk());
	if (first.IsError() || second.IsError())
	{
		return;
	}
	CHECK(uploader.ReleaseImage(first.GetValue()).IsOk());

	// The next upload gets the next id, not the released one: ids run
	// upward forever per uploader instance, so a stale handle can never
	// alias a newer resource.
	const Result<ImageResourceHandle> third = uploader.UploadImage(MakeTestImage());
	CHECK(third.IsOk());
	if (third.IsError())
	{
		return;
	}

	CHECK_EQ(third.GetValue().Get(), static_cast<uint64_t>(3));
	CHECK(third.GetValue() != first.GetValue());
	CHECK(!uploader.IsValidImage(first.GetValue()));
	CHECK(uploader.IsValidImage(second.GetValue()));
	CHECK(uploader.IsValidImage(third.GetValue()));
}

// ---------------------------------------------------------------------------
// Mesh resource upload
// ---------------------------------------------------------------------------

MODERN_TEST(ClientUpload_MeshUploadPreservesMetadata)
{
	NullRenderer renderer;
	InitializeRenderer(renderer);
	NullAssetUploader uploader(&renderer);
	CHECK_EQ(uploader.GetLiveMeshCount(), static_cast<size_t>(0));

	const MeshAsset mesh = MakeTestMesh();
	const Result<MeshResourceHandle> handle = uploader.UploadMesh(mesh);
	CHECK(handle.IsOk());
	if (handle.IsError())
	{
		return;
	}

	CHECK(handle.GetValue().IsValid());
	CHECK(uploader.IsValidMesh(handle.GetValue()));
	CHECK_EQ(uploader.GetLiveMeshCount(), static_cast<size_t>(1));

	const Result<MeshResourceInfo> info = uploader.GetMeshInfo(handle.GetValue());
	CHECK(info.IsOk());
	if (info.IsError())
	{
		return;
	}

	// Every number the asset reported survives the crossing -- counts,
	// derived triangle count, topology and byte total -- while the
	// vertices and indices themselves stay in the asset, owned by this
	// scope.
	CHECK_EQ(info.GetValue().vertexCount, mesh.GetVertexCount());
	CHECK_EQ(info.GetValue().indexCount, mesh.GetIndexCount());
	CHECK_EQ(info.GetValue().triangleCount, mesh.GetTriangleCount());
	CHECK_EQ(info.GetValue().topology, mesh.GetTopology());
	CHECK_EQ(info.GetValue().byteCount, mesh.GetTotalByteCount());
	CHECK_EQ(info.GetValue().vertexCount, static_cast<size_t>(4));
	CHECK_EQ(info.GetValue().indexCount, static_cast<size_t>(6));
	CHECK_EQ(info.GetValue().triangleCount, static_cast<size_t>(2));
	CHECK_EQ(info.GetValue().topology, PrimitiveTopology::TriangleList);
	CHECK_EQ(info.GetValue().byteCount, static_cast<size_t>(152));
}

MODERN_TEST(ClientUpload_MeshUploadRequiresInitializedRenderer)
{
	NullRenderer renderer;
	CHECK_EQ(renderer.GetState(), RendererState::Uninitialized);
	NullAssetUploader uploader(&renderer);

	const Result<MeshResourceHandle> beforeInit = uploader.UploadMesh(MakeTestMesh());
	CHECK(!beforeInit.IsOk());
	CHECK_EQ(beforeInit.GetError(), ErrorCode::InvalidState);
	CHECK_EQ(uploader.GetLiveMeshCount(), static_cast<size_t>(0));
	CHECK(!uploader.IsValidMesh(MeshResourceHandle()));

	InitializeRenderer(renderer);
	CHECK(uploader.UploadMesh(MakeTestMesh()).IsOk());
	CHECK_EQ(uploader.GetLiveMeshCount(), static_cast<size_t>(1));
}

MODERN_TEST(ClientUpload_InvalidMeshAssetCannotExist)
{
	// Same structural answer as the image half: there is no invalid
	// MeshAsset to upload, because Create() is the only way to a value and
	// there is no unchecked constructor. Every refusal carries
	// InvalidArgument, in the documented validation order.
	static_assert(!std::is_default_constructible<MeshAsset>::value,
		"an asset must not be constructible without validation");

	std::vector<MeshVertex> oneVertex = {
		MeshVertex(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), 0.0f, 0.0f)
	};
	std::vector<MeshVertex> threeVertices = {
		MeshVertex(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), 0.0f, 0.0f),
		MeshVertex(Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), 1.0f, 0.0f),
		MeshVertex(Vector3(1.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), 1.0f, 1.0f),
	};

	// Unknown topology, refused before the geometry rules it would decide.
	CHECK_EQ(MeshAsset::Create(PrimitiveTopology::Unknown, threeVertices, { 0, 1, 2 }).GetError(),
		ErrorCode::InvalidArgument);
	// Empty geometry, both halves.
	CHECK_EQ(MeshAsset::Create(PrimitiveTopology::TriangleList, {}, { 0, 1, 2 }).GetError(),
		ErrorCode::InvalidArgument);
	CHECK_EQ(MeshAsset::Create(PrimitiveTopology::TriangleList, threeVertices, {}).GetError(),
		ErrorCode::InvalidArgument);
	// An index count that is not whole triangles.
	CHECK_EQ(MeshAsset::Create(PrimitiveTopology::TriangleList, threeVertices, { 0, 1 }).GetError(),
		ErrorCode::InvalidArgument);
	// An index naming a vertex that does not exist.
	CHECK_EQ(MeshAsset::Create(PrimitiveTopology::TriangleList, oneVertex, { 0, 0, 9 }).GetError(),
		ErrorCode::InvalidArgument);

	// A non-finite vertex smuggled into otherwise perfect geometry.
	std::vector<MeshVertex> poisoned = threeVertices;
	poisoned[1].position.x = std::nanf("");
	CHECK_EQ(MeshAsset::Create(PrimitiveTopology::TriangleList, poisoned, { 0, 1, 2 }).GetError(),
		ErrorCode::InvalidArgument);
}

MODERN_TEST(ClientUpload_MultipleMeshesReceiveDistinctHandles)
{
	NullRenderer renderer;
	InitializeRenderer(renderer);
	NullAssetUploader uploader(&renderer);

	const MeshAsset mesh = MakeTestMesh();
	const Result<MeshResourceHandle> first = uploader.UploadMesh(mesh);
	const Result<MeshResourceHandle> second = uploader.UploadMesh(mesh);
	const Result<MeshResourceHandle> third = uploader.UploadMesh(mesh);

	CHECK(first.IsOk() && second.IsOk() && third.IsOk());
	if (first.IsError() || second.IsError() || third.IsError())
	{
		return;
	}

	CHECK(first.GetValue() != second.GetValue());
	CHECK(second.GetValue() != third.GetValue());
	CHECK(first.GetValue() != third.GetValue());
	CHECK_EQ(uploader.GetLiveMeshCount(), static_cast<size_t>(3));

	// Releasing one leaves the others live -- the registry holds three
	// independent resources, not one shared fate.
	CHECK(uploader.ReleaseMesh(second.GetValue()).IsOk());
	CHECK_EQ(uploader.GetLiveMeshCount(), static_cast<size_t>(2));
	CHECK(!uploader.IsValidMesh(second.GetValue()));
	CHECK(uploader.IsValidMesh(first.GetValue()));
	CHECK(uploader.IsValidMesh(third.GetValue()));
	CHECK_EQ(uploader.GetMeshInfo(second.GetValue()).GetError(), ErrorCode::NotFound);
}

// ---------------------------------------------------------------------------
// Mesh release and shutdown
// ---------------------------------------------------------------------------

MODERN_TEST(ClientUpload_ReleaseMeshHandleWorks)
{
	NullRenderer renderer;
	InitializeRenderer(renderer);
	NullAssetUploader uploader(&renderer);

	const Result<MeshResourceHandle> handle = uploader.UploadMesh(MakeTestMesh());
	CHECK(handle.IsOk());
	if (handle.IsError())
	{
		return;
	}

	CHECK(uploader.ReleaseMesh(handle.GetValue()).IsOk());
	CHECK(!uploader.IsValidMesh(handle.GetValue()));
	CHECK_EQ(uploader.GetLiveMeshCount(), static_cast<size_t>(0));
	CHECK_EQ(uploader.GetMeshInfo(handle.GetValue()).GetError(), ErrorCode::NotFound);
}

MODERN_TEST(ClientUpload_MeshReleaseOfInvalidAndDoubleHandle)
{
	NullRenderer renderer;
	InitializeRenderer(renderer);
	NullAssetUploader uploader(&renderer);

	// Sentinel and unknown ids, in that documented order.
	CHECK_EQ(uploader.ReleaseMesh(MeshResourceHandle()).GetCode(),
		ErrorCode::InvalidArgument);
	CHECK_EQ(uploader.ReleaseMesh(MeshResourceHandle(static_cast<uint64_t>(9999))).GetCode(),
		ErrorCode::NotFound);
	// Get*Info answers the same table.
	CHECK_EQ(uploader.GetMeshInfo(MeshResourceHandle()).GetError(),
		ErrorCode::InvalidArgument);
	CHECK_EQ(uploader.GetMeshInfo(MeshResourceHandle(static_cast<uint64_t>(9999))).GetError(),
		ErrorCode::NotFound);

	const Result<MeshResourceHandle> handle = uploader.UploadMesh(MakeTestMesh());
	CHECK(handle.IsOk());
	if (handle.IsError())
	{
		return;
	}

	// Double destruction: Ok once, then NotFound forever after.
	CHECK(uploader.ReleaseMesh(handle.GetValue()).IsOk());
	CHECK_EQ(uploader.ReleaseMesh(handle.GetValue()).GetCode(), ErrorCode::NotFound);
	CHECK_EQ(uploader.ReleaseMesh(handle.GetValue()).GetCode(), ErrorCode::NotFound);
	CHECK_EQ(uploader.GetLiveMeshCount(), static_cast<size_t>(0));
}

MODERN_TEST(ClientUpload_ShutdownInvalidatesMeshResources)
{
	NullRenderer renderer;
	InitializeRenderer(renderer);
	NullAssetUploader uploader(&renderer);

	const Result<MeshResourceHandle> handle = uploader.UploadMesh(MakeTestMesh());
	CHECK(handle.IsOk());
	if (handle.IsError())
	{
		return;
	}
	CHECK_EQ(uploader.GetLiveMeshCount(), static_cast<size_t>(1));

	// Renderer shutdown while the mesh handle exists: immediately invalid,
	// immediately empty, uniformly refused afterwards.
	renderer.Shutdown();

	CHECK(!uploader.IsValidMesh(handle.GetValue()));
	CHECK_EQ(uploader.GetLiveMeshCount(), static_cast<size_t>(0));
	CHECK_EQ(uploader.GetMeshInfo(handle.GetValue()).GetError(), ErrorCode::NotAllowed);
	CHECK_EQ(uploader.UploadMesh(MakeTestMesh()).GetError(), ErrorCode::NotAllowed);
	CHECK_EQ(uploader.ReleaseMesh(handle.GetValue()).GetCode(), ErrorCode::NotAllowed);
	CHECK_EQ(uploader.ReleaseMesh(handle.GetValue()).GetCode(), ErrorCode::NotAllowed);
}

// ---------------------------------------------------------------------------
// Resource pipelines
// ---------------------------------------------------------------------------

MODERN_TEST(ClientUpload_ImagePipelineStorageToResourceHandle)
{
	// The complete CLIENT-009 path, storage to renderer resource:
	//
	//   MemoryResourceProvider -> ResourceManager -> ResourceData
	//     -> TestImageDecoder -> ImageAsset -> IAssetUploader
	//     -> ImageResourceHandle
	//
	// No GPU, no window, no RAN installation: the bytes were assembled by
	// hand two helpers above.
	TestImageDecoder decoder;
	const std::vector<uint8_t> bytes = MakeImageBytes(4, 2, ImageFormat::R8G8B8A8_UNorm);

	MemoryResourceProvider provider;
	ResourceManager manager;
	manager.SetProvider(&provider);
	CHECK(manager.Initialize().IsOk());

	const ResourceId id = MakeId("ui/upload_target.mimg");
	CHECK(provider.RegisterResource(id, ResourceData(bytes)).IsOk());

	// provider -> manager -> bytes.
	const Result<ResourceData> loaded = manager.Load(id);
	CHECK(loaded.IsOk());
	if (loaded.IsError())
	{
		return;
	}

	// bytes -> asset.
	const Result<ImageAsset> decoded = decoder.DecodeImage(loaded.GetValue());
	CHECK(decoded.IsOk());
	if (decoded.IsError())
	{
		return;
	}

	// asset -> renderer resource.
	NullRenderer renderer;
	InitializeRenderer(renderer);
	NullAssetUploader uploader(&renderer);

	const Result<ImageResourceHandle> handle = uploader.UploadImage(decoded.GetValue());
	CHECK(handle.IsOk());
	if (handle.IsError())
	{
		return;
	}
	CHECK(uploader.IsValidImage(handle.GetValue()));

	const Result<ImageResourceInfo> info = uploader.GetImageInfo(handle.GetValue());
	CHECK(info.IsOk());
	if (info.IsError())
	{
		return;
	}
	CHECK_EQ(info.GetValue().width, 4u);
	CHECK_EQ(info.GetValue().height, 2u);
	CHECK_EQ(info.GetValue().format, ImageFormat::R8G8B8A8_UNorm);
	CHECK_EQ(info.GetValue().byteCount, static_cast<size_t>(32));

	// The two sides own different things: shutting the storage layer down
	// leaves the uploaded resource untouched (the uploader never saw the
	// manager), and releasing the handle never touched the byte cache.
	CHECK(manager.Shutdown().IsOk());
	CHECK(uploader.IsValidImage(handle.GetValue()));
	CHECK(uploader.ReleaseImage(handle.GetValue()).IsOk());
}

MODERN_TEST(ClientUpload_MeshPipelineStorageToResourceHandle)
{
	// The geometry half of the same path:
	//
	//   MemoryResourceProvider -> ResourceManager -> ResourceData
	//     -> TestMeshDecoder -> MeshAsset -> IAssetUploader
	//     -> MeshResourceHandle
	TestMeshDecoder decoder;
	const std::vector<uint8_t> bytes = MakeMeshBytes();

	MemoryResourceProvider provider;
	ResourceManager manager;
	manager.SetProvider(&provider);
	CHECK(manager.Initialize().IsOk());

	const ResourceId id = MakeId("world/upload_target.mmsh");
	CHECK(provider.RegisterResource(id, ResourceData(bytes)).IsOk());

	const Result<ResourceData> loaded = manager.Load(id);
	CHECK(loaded.IsOk());
	if (loaded.IsError())
	{
		return;
	}

	const Result<MeshAsset> decoded = decoder.DecodeMesh(loaded.GetValue());
	CHECK(decoded.IsOk());
	if (decoded.IsError())
	{
		return;
	}

	NullRenderer renderer;
	InitializeRenderer(renderer);
	NullAssetUploader uploader(&renderer);

	const Result<MeshResourceHandle> handle = uploader.UploadMesh(decoded.GetValue());
	CHECK(handle.IsOk());
	if (handle.IsError())
	{
		return;
	}
	CHECK(uploader.IsValidMesh(handle.GetValue()));

	const Result<MeshResourceInfo> info = uploader.GetMeshInfo(handle.GetValue());
	CHECK(info.IsOk());
	if (info.IsError())
	{
		return;
	}
	CHECK_EQ(info.GetValue().vertexCount, static_cast<size_t>(4));
	CHECK_EQ(info.GetValue().indexCount, static_cast<size_t>(6));
	CHECK_EQ(info.GetValue().triangleCount, static_cast<size_t>(2));
	CHECK_EQ(info.GetValue().topology, PrimitiveTopology::TriangleList);
	CHECK_EQ(info.GetValue().byteCount, static_cast<size_t>(152));

	// Storage shuts down; the renderer resource lives on, and the release
	// reaches only the uploader.
	CHECK(manager.Shutdown().IsOk());
	CHECK(uploader.IsValidMesh(handle.GetValue()));
	CHECK(uploader.ReleaseMesh(handle.GetValue()).IsOk());
	CHECK_EQ(uploader.GetLiveMeshCount(), static_cast<size_t>(0));
}

// ---------------------------------------------------------------------------
// Separation
// ---------------------------------------------------------------------------

MODERN_TEST(ClientUpload_BoundaryContractCarriesNoStorageTypes)
{
	// The API speaks assets and handles -- pinned here as exact types, so a
	// ResourceId, ResourceData or ResourceManager parameter (or a backend
	// object) would not compile against this interface.
	using UploadImageSignature = Result<ImageResourceHandle> (IAssetUploader::*)(const ImageAsset&);
	using UploadMeshSignature = Result<MeshResourceHandle> (IAssetUploader::*)(const MeshAsset&);
	using ReleaseImageSignature = Status (IAssetUploader::*)(ImageResourceHandle) noexcept;
	using GetImageInfoSignature = Result<ImageResourceInfo> (IAssetUploader::*)(ImageResourceHandle) const;

	static_assert(std::is_same<decltype(&IAssetUploader::UploadImage), UploadImageSignature>::value,
		"UploadImage must consume a CPU asset and return an opaque handle");
	static_assert(std::is_same<decltype(&IAssetUploader::UploadMesh), UploadMeshSignature>::value,
		"UploadMesh must consume a CPU asset and return an opaque handle");
	static_assert(std::is_same<decltype(&IAssetUploader::ReleaseImage), ReleaseImageSignature>::value,
		"release must take a handle and nothing else");
	static_assert(std::is_same<decltype(&IAssetUploader::GetImageInfo), GetImageInfoSignature>::value,
		"info must take a handle and return this layer's own metadata struct");

	// The boundary is an interface; the shipped implementation is a headless
	// leaf behind it -- no singleton, no global, no backend type in between.
	static_assert(std::is_abstract<IAssetUploader>::value,
		"the upload boundary must be an interface, not a backend");
	static_assert(std::is_base_of<IAssetUploader, NullAssetUploader>::value,
		"the null implementation is a leaf behind the interface");

	// And the runtime half: uploading works with no resource-layer object in
	// sight -- hand-built assets and a renderer are the only things created.
	NullRenderer renderer;
	InitializeRenderer(renderer);
	NullAssetUploader uploader(&renderer);

	const ImageAsset image = MakeTestImage();
	const MeshAsset mesh = MakeTestMesh();
	CHECK(uploader.UploadImage(image).IsOk());
	CHECK(uploader.UploadMesh(mesh).IsOk());
}

MODERN_TEST(ClientUpload_InfoRefusalsFollowDecisionOrder)
{
	// Get*Info answers exactly like Release: lifecycle first, then the
	// sentinel as a bad argument, then the lookup.
	NullRenderer renderer;
	InitializeRenderer(renderer);
	NullAssetUploader uploader(&renderer);

	CHECK_EQ(uploader.GetImageInfo(ImageResourceHandle()).GetError(),
		ErrorCode::InvalidArgument);
	CHECK_EQ(uploader.GetImageInfo(ImageResourceHandle(static_cast<uint64_t>(9999))).GetError(),
		ErrorCode::NotFound);

	// A live handle answers with metadata while the lifecycle allows it...
	const Result<ImageResourceHandle> handle = uploader.UploadImage(MakeTestImage());
	CHECK(handle.IsOk());
	if (handle.IsError())
	{
		return;
	}
	CHECK(uploader.GetImageInfo(handle.GetValue()).IsOk());

	// ...and the lifecycle outranks all of it once the renderer stops.
	renderer.Shutdown();
	CHECK_EQ(uploader.GetImageInfo(handle.GetValue()).GetError(), ErrorCode::NotAllowed);
	CHECK_EQ(uploader.GetImageInfo(ImageResourceHandle()).GetError(), ErrorCode::NotAllowed);
}

// ---------------------------------------------------------------------------
// Main test runner
// ---------------------------------------------------------------------------

int main()
{
	std::printf("Modern CLIENT-009 renderer asset upload boundary tests\n\n");

	const int failedCases = ModernTests::RunAll();

	if (failedCases == 0)
	{
		std::printf("\nAll %d test cases passed.\n", static_cast<int>(ModernTests::Registry().size()));
		return 0;
	}

	std::printf("\n%d of %d test cases FAILED (%d checks).\n",
		failedCases,
		static_cast<int>(ModernTests::Registry().size()),
		ModernTests::FailureCount());
	return 1;
}
