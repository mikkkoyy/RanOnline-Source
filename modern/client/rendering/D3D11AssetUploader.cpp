// CLIENT-RENDER-001B: Direct3D 11 asset uploader implementation.

#include "rendering/D3D11AssetUploader.h"

#include "assets/AssetTypes.h"
#include "assets/ImageAsset.h"
#include "assets/MeshAsset.h"
#include "rendering/D3D11Renderer.h"

#include <cstdarg>
#include <cstdio>

namespace Modern::Client
{
namespace
{

// D3D11 texture dimension ceiling; the asset layer's kMaxImageDimension is
// the same number, checked here again so a buffer is never sized by a value
// the API would refuse.
constexpr uint32_t kMaxTextureDimension = 16384u;

void Log(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	std::vfprintf(stderr, format, args);
	std::fputc('\n', stderr);
	va_end(args);
}

// Maps an ImageAsset pixel layout to the DXGI format the texture is created
// with. Returns false for Unknown or any layout the uploader does not carry
// an explicit conversion for: the decoder's pixel layout is never assumed to
// match a DXGI format without this table saying so. Notably, DXGI has no
// packed 24-bit RGB format, so R8G8B8 is refused rather than silently
// expanded to RGBA.
bool TryGetDxgiFormat(ImageFormat format, DXGI_FORMAT& out) noexcept
{
	switch (format)
	{
		case ImageFormat::R8_UNorm:       out = DXGI_FORMAT_R8_UNORM;       return true;
		case ImageFormat::R8G8B8A8_UNorm: out = DXGI_FORMAT_R8G8B8A8_UNORM; return true;
		case ImageFormat::B8G8R8A8_UNorm: out = DXGI_FORMAT_B8G8R8A8_UNORM; return true;
		case ImageFormat::Unknown:        return false;
	}
	return false;
}

template <typename T>
void SafeRelease(T*& ptr) noexcept
{
	if (ptr != nullptr)
	{
		ptr->Release();
		ptr = nullptr;
	}
}

} // namespace

D3D11AssetUploader::~D3D11AssetUploader()
{
	ReleaseAll();
}

RendererState D3D11AssetUploader::PeekState() const noexcept
{
	return m_renderer != nullptr ? m_renderer->GetState() : RendererState::Uninitialized;
}

bool D3D11AssetUploader::IsLive() const noexcept
{
	const RendererState state = PeekState();
	return state == RendererState::Initialized || state == RendererState::InFrame;
}

Status D3D11AssetUploader::CheckLifecycle() noexcept
{
	const RendererState state = PeekState();

	if (state == RendererState::ShutdownState)
	{
		// Shutdown policy: the renderer's resources die with it, and so do
		// the GPU objects this uploader created through its device. The
		// entries were already unobservable -- IsValid* and the counts answer
		// false and 0 straight from the state -- and this is where they are
		// physically released. Dropping an already-empty registry is a no-op,
		// so the first post-shutdown call does the work and every later one
		// is equally harmless.
		ReleaseAll();
		return Status(ErrorCode::NotAllowed);
	}

	if (state != RendererState::Initialized && state != RendererState::InFrame)
	{
		return Status(ErrorCode::InvalidState);
	}

	return Ok();
}

Status D3D11AssetUploader::CheckLifecycleConst() const noexcept
{
	const RendererState state = PeekState();

	if (state == RendererState::ShutdownState)
	{
		return Status(ErrorCode::NotAllowed);
	}

	if (state != RendererState::Initialized && state != RendererState::InFrame)
	{
		return Status(ErrorCode::InvalidState);
	}

	return Ok();
}

D3D11AssetUploader::ImageEntry* D3D11AssetUploader::FindImage(ImageResourceHandle handle) noexcept
{
	for (ImageEntry& entry : m_images)
	{
		if (entry.handle == handle)
		{
			return &entry;
		}
	}
	return nullptr;
}

const D3D11AssetUploader::ImageEntry* D3D11AssetUploader::FindImage(ImageResourceHandle handle) const noexcept
{
	for (const ImageEntry& entry : m_images)
	{
		if (entry.handle == handle)
		{
			return &entry;
		}
	}
	return nullptr;
}

D3D11AssetUploader::MeshEntry* D3D11AssetUploader::FindMesh(MeshResourceHandle handle) noexcept
{
	for (MeshEntry& entry : m_meshes)
	{
		if (entry.handle == handle)
		{
			return &entry;
		}
	}
	return nullptr;
}

const D3D11AssetUploader::MeshEntry* D3D11AssetUploader::FindMesh(MeshResourceHandle handle) const noexcept
{
	for (const MeshEntry& entry : m_meshes)
	{
		if (entry.handle == handle)
		{
			return &entry;
		}
	}
	return nullptr;
}

void D3D11AssetUploader::ReleaseEntry(ImageEntry& entry) noexcept
{
	SafeRelease(entry.srv);
	SafeRelease(entry.texture);
}

void D3D11AssetUploader::ReleaseEntry(MeshEntry& entry) noexcept
{
	SafeRelease(entry.vertexBuffer);
	SafeRelease(entry.indexBuffer);
	entry.indexCount = 0;
}

void D3D11AssetUploader::ReleaseAll() noexcept
{
	for (ImageEntry& entry : m_images)
	{
		ReleaseEntry(entry);
	}
	for (MeshEntry& entry : m_meshes)
	{
		ReleaseEntry(entry);
	}
	m_images.clear();
	m_meshes.clear();
}

Result<ImageResourceHandle> D3D11AssetUploader::UploadImage(const ImageAsset& image)
{
	const Status lifecycle = CheckLifecycle();
	if (!lifecycle.IsOk())
	{
		return lifecycle;
	}

	const uint32_t width  = image.GetWidth();
	const uint32_t height = image.GetHeight();

	// The asset arrived already validated (ImageAsset has no default
	// constructor), but the GPU has its own ceilings and its own format
	// table, and both are checked here rather than trusted.
	DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
	if (!TryGetDxgiFormat(image.GetFormat(), format))
	{
		Log("D3D11AssetUploader: unsupported image format %s (%u x %u)",
			ToString(image.GetFormat()), width, height);
		return Status(ErrorCode::InvalidArgument);
	}

	if (width == 0 || height == 0 ||
	    width > kMaxTextureDimension || height > kMaxTextureDimension)
	{
		Log("D3D11AssetUploader: image dimensions %u x %u exceed the D3D11 limit %u",
			width, height, kMaxTextureDimension);
		return Status(ErrorCode::InvalidArgument);
	}

	const uint32_t bytesPerPixel = BytesPerPixel(image.GetFormat());
	const size_t expectedBytes = static_cast<size_t>(width) * static_cast<size_t>(height) * bytesPerPixel;
	if (image.GetPixelByteCount() != expectedBytes)
	{
		Log("D3D11AssetUploader: image payload %zu bytes does not match %u x %u x %u",
			image.GetPixelByteCount(), width, height, bytesPerPixel);
		return Status(ErrorCode::InvalidArgument);
	}

	ID3D11Device* device = m_renderer->GetDevice();

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width            = width;
	desc.Height           = height;
	desc.MipLevels        = 1;
	desc.ArraySize        = 1;
	desc.Format           = format;
	desc.SampleDesc.Count = 1;
	desc.Usage            = D3D11_USAGE_DEFAULT;
	desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;

	// The asset's pixels are row-major, tightly packed, top row first, no row
	// padding -- so the row pitch is exactly width * bytesPerPixel and no
	// conversion is needed before the upload.
	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem     = image.GetPixels().data();
	initData.SysMemPitch = static_cast<UINT>(width * bytesPerPixel);

	ID3D11Texture2D* texture = nullptr;
	const HRESULT textureHr = device->CreateTexture2D(&desc, &initData, &texture);
	if (FAILED(textureHr) || texture == nullptr)
	{
		Log("D3D11AssetUploader: CreateTexture2D failed for %u x %u (HRESULT 0x%08X)",
			width, height, static_cast<unsigned>(textureHr));
		return Status(ErrorCode::InvalidArgument);
	}

	ID3D11ShaderResourceView* srv = nullptr;
	const HRESULT srvHr = device->CreateShaderResourceView(texture, nullptr, &srv);
	if (FAILED(srvHr) || srv == nullptr)
	{
		Log("D3D11AssetUploader: CreateShaderResourceView failed (HRESULT 0x%08X)",
			static_cast<unsigned>(srvHr));
		SafeRelease(texture);
		return Status(ErrorCode::InvalidArgument);
	}

	const ImageResourceHandle handle(m_nextImageId++);
	m_images.push_back(ImageEntry{ handle,
		ImageResourceInfo{ width, height, image.GetFormat(), image.GetPixelByteCount() },
		texture, srv });

	Log("D3D11AssetUploader: uploaded image %llu (%u x %u, format %d)",
		static_cast<unsigned long long>(handle.Get()), width, height, static_cast<int>(format));
	return handle;
}

Result<MeshResourceHandle> D3D11AssetUploader::UploadMesh(const MeshAsset& mesh)
{
	const Status lifecycle = CheckLifecycle();
	if (!lifecycle.IsOk())
	{
		return lifecycle;
	}

	// Same rule as the image path: validated in, checked against the GPU's
	// own limits, geometry never retained.
	if (mesh.GetTopology() != PrimitiveTopology::TriangleList)
	{
		Log("D3D11AssetUploader: mesh topology %s is not TriangleList",
			ToString(mesh.GetTopology()));
		return Status(ErrorCode::InvalidArgument);
	}

	const size_t vertexCount = mesh.GetVertexCount();
	const size_t indexCount  = mesh.GetIndexCount();

	if (vertexCount == 0 || indexCount == 0 || (indexCount % 3) != 0)
	{
		Log("D3D11AssetUploader: mesh has %zu vertices / %zu indices", vertexCount, indexCount);
		return Status(ErrorCode::InvalidArgument);
	}

	if (vertexCount > kMaxMeshVertices || indexCount > kMaxMeshIndices)
	{
		Log("D3D11AssetUploader: mesh exceeds D3D11 buffer limits (%zu vertices, %zu indices)",
			vertexCount, indexCount);
		return Status(ErrorCode::InvalidArgument);
	}

	const size_t vertexBytes = mesh.GetVertexByteCount();
	const size_t indexBytes  = mesh.GetIndexByteCount();
	if (vertexBytes != vertexCount * sizeof(MeshVertex) ||
	    indexBytes != indexCount * sizeof(MeshIndex))
	{
		Log("D3D11AssetUploader: mesh byte counts do not match its element counts");
		return Status(ErrorCode::InvalidArgument);
	}

	ID3D11Device* device = m_renderer->GetDevice();

	// Vertex buffer: one MeshVertex (32 bytes: position, normal, UV) per
	// vertex, copied verbatim -- the D3D11 input layout is declared with the
	// same offsets, asserted in D3D11Renderer.cpp.
	D3D11_BUFFER_DESC vertexDesc = {};
	vertexDesc.ByteWidth           = static_cast<UINT>(vertexBytes);
	vertexDesc.Usage               = D3D11_USAGE_DEFAULT;
	vertexDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = mesh.GetVertices().data();

	ID3D11Buffer* vertexBuffer = nullptr;
	const HRESULT vbHr = device->CreateBuffer(&vertexDesc, &vertexData, &vertexBuffer);
	if (FAILED(vbHr) || vertexBuffer == nullptr)
	{
		Log("D3D11AssetUploader: CreateBuffer (vertex, %zu bytes) failed (HRESULT 0x%08X)",
			vertexBytes, static_cast<unsigned>(vbHr));
		return Status(ErrorCode::InvalidArgument);
	}

	// Index buffer: 32-bit indices, matching MeshIndex.
	D3D11_BUFFER_DESC indexDesc = {};
	indexDesc.ByteWidth           = static_cast<UINT>(indexBytes);
	indexDesc.Usage               = D3D11_USAGE_DEFAULT;
	indexDesc.BindFlags           = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = mesh.GetIndices().data();

	ID3D11Buffer* indexBuffer = nullptr;
	const HRESULT ibHr = device->CreateBuffer(&indexDesc, &indexData, &indexBuffer);
	if (FAILED(ibHr) || indexBuffer == nullptr)
	{
		Log("D3D11AssetUploader: CreateBuffer (index, %zu bytes) failed (HRESULT 0x%08X)",
			indexBytes, static_cast<unsigned>(ibHr));
		SafeRelease(vertexBuffer);
		return Status(ErrorCode::InvalidArgument);
	}

	const MeshResourceHandle handle(m_nextMeshId++);
	m_meshes.push_back(MeshEntry{ handle,
		MeshResourceInfo{ vertexCount, indexCount, mesh.GetTriangleCount(),
		                  mesh.GetTopology(), mesh.GetTotalByteCount() },
		vertexBuffer, indexBuffer, static_cast<uint32_t>(indexCount) });

	Log("D3D11AssetUploader: uploaded mesh %llu (%zu vertices, %zu indices, %zu triangles)",
		static_cast<unsigned long long>(handle.Get()), vertexCount, indexCount,
		mesh.GetTriangleCount());
	return handle;
}

bool D3D11AssetUploader::IsValidImage(ImageResourceHandle handle) const noexcept
{
	return IsLive() && FindImage(handle) != nullptr;
}

bool D3D11AssetUploader::IsValidMesh(MeshResourceHandle handle) const noexcept
{
	return IsLive() && FindMesh(handle) != nullptr;
}

Status D3D11AssetUploader::ReleaseImage(ImageResourceHandle handle) noexcept
{
	const Status lifecycle = CheckLifecycle();
	if (!lifecycle.IsOk())
	{
		return lifecycle;
	}

	if (!handle.IsValid())
	{
		return Status(ErrorCode::InvalidArgument);
	}

	ImageEntry* entry = FindImage(handle);
	if (entry == nullptr)
	{
		return Status(ErrorCode::NotFound);
	}

	ReleaseEntry(*entry);
	m_images.erase(m_images.begin() + (entry - m_images.data()));
	return Ok();
}

Status D3D11AssetUploader::ReleaseMesh(MeshResourceHandle handle) noexcept
{
	const Status lifecycle = CheckLifecycle();
	if (!lifecycle.IsOk())
	{
		return lifecycle;
	}

	if (!handle.IsValid())
	{
		return Status(ErrorCode::InvalidArgument);
	}

	MeshEntry* entry = FindMesh(handle);
	if (entry == nullptr)
	{
		return Status(ErrorCode::NotFound);
	}

	ReleaseEntry(*entry);
	m_meshes.erase(m_meshes.begin() + (entry - m_meshes.data()));
	return Ok();
}

Result<ImageResourceInfo> D3D11AssetUploader::GetImageInfo(ImageResourceHandle handle) const
{
	const Status lifecycle = CheckLifecycleConst();
	if (!lifecycle.IsOk())
	{
		return lifecycle;
	}

	if (!handle.IsValid())
	{
		return Status(ErrorCode::InvalidArgument);
	}

	const ImageEntry* entry = FindImage(handle);
	if (entry == nullptr)
	{
		return Status(ErrorCode::NotFound);
	}

	return entry->info;
}

Result<MeshResourceInfo> D3D11AssetUploader::GetMeshInfo(MeshResourceHandle handle) const
{
	const Status lifecycle = CheckLifecycleConst();
	if (!lifecycle.IsOk())
	{
		return lifecycle;
	}

	if (!handle.IsValid())
	{
		return Status(ErrorCode::InvalidArgument);
	}

	const MeshEntry* entry = FindMesh(handle);
	if (entry == nullptr)
	{
		return Status(ErrorCode::NotFound);
	}

	return entry->info;
}

size_t D3D11AssetUploader::GetLiveImageCount() const noexcept
{
	return IsLive() ? m_images.size() : 0;
}

size_t D3D11AssetUploader::GetLiveMeshCount() const noexcept
{
	return IsLive() ? m_meshes.size() : 0;
}

bool D3D11AssetUploader::TryResolveMesh(MeshResourceHandle handle, D3D11MeshGpuResources& out) const noexcept
{
	if (!IsLive() || !handle.IsValid())
	{
		return false;
	}

	const MeshEntry* entry = FindMesh(handle);
	if (entry == nullptr || entry->vertexBuffer == nullptr || entry->indexBuffer == nullptr)
	{
		return false;
	}

	out.vertexBuffer = entry->vertexBuffer;
	out.indexBuffer  = entry->indexBuffer;
	out.indexCount   = entry->indexCount;
	return true;
}

bool D3D11AssetUploader::TryResolveImage(ImageResourceHandle handle, D3D11ImageGpuResources& out) const noexcept
{
	if (!IsLive() || !handle.IsValid())
	{
		return false;
	}

	const ImageEntry* entry = FindImage(handle);
	if (entry == nullptr || entry->srv == nullptr)
	{
		return false;
	}

	out.shaderResourceView = entry->srv;
	out.width             = entry->info.width;
	out.height            = entry->info.height;
	return true;
}

} // namespace Modern::Client
