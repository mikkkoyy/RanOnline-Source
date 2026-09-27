#include "NullAssetUploader.h"

#include "assets/ImageAsset.h"
#include "assets/MeshAsset.h"

namespace Modern::Client
{

RendererState NullAssetUploader::PeekState() const noexcept
{
	return m_renderer != nullptr ? m_renderer->GetState() : RendererState::Uninitialized;
}

bool NullAssetUploader::IsLive() const noexcept
{
	const RendererState state = PeekState();
	return state == RendererState::Initialized || state == RendererState::InFrame;
}

Status NullAssetUploader::CheckLifecycle() noexcept
{
	const RendererState state = PeekState();

	if (state == RendererState::ShutdownState)
	{
		// Shutdown policy: the renderer's resources die with it. The entries
		// were already unobservable -- IsValid* and the counts answer false
		// and 0 straight from the state -- and this is where they are
		// physically released. Dropping an already-empty registry is a no-op,
		// so the first post-shutdown call does the work and every later one
		// is equally harmless.
		m_images.clear();
		m_meshes.clear();
		return Status(ErrorCode::NotAllowed);
	}

	if (state != RendererState::Initialized && state != RendererState::InFrame)
	{
		return Status(ErrorCode::InvalidState);
	}

	return Ok();
}

Status NullAssetUploader::CheckLifecycleConst() const noexcept
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

NullAssetUploader::ImageEntry* NullAssetUploader::FindImage(ImageResourceHandle handle) noexcept
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

const NullAssetUploader::ImageEntry* NullAssetUploader::FindImage(ImageResourceHandle handle) const noexcept
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

NullAssetUploader::MeshEntry* NullAssetUploader::FindMesh(MeshResourceHandle handle) noexcept
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

const NullAssetUploader::MeshEntry* NullAssetUploader::FindMesh(MeshResourceHandle handle) const noexcept
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

Result<ImageResourceHandle> NullAssetUploader::UploadImage(const ImageAsset& image)
{
	const Status lifecycle = CheckLifecycle();
	if (!lifecycle.IsOk())
	{
		return lifecycle;
	}

	// The asset arrived already validated: ImageAsset has no default
	// constructor and Create() is its only way to a value, so there is no
	// invalid image to refuse here. What is copied is the metadata, never
	// the pixels -- this registry must not become a second ResourceManager.
	const ImageResourceHandle handle(m_nextImageId++);
	m_images.push_back(ImageEntry{ handle,
		ImageResourceInfo{
			image.GetWidth(),
			image.GetHeight(),
			image.GetFormat(),
			image.GetPixelByteCount() } });

	return handle;
}

Result<MeshResourceHandle> NullAssetUploader::UploadMesh(const MeshAsset& mesh)
{
	const Status lifecycle = CheckLifecycle();
	if (!lifecycle.IsOk())
	{
		return lifecycle;
	}

	// Same rule as the image path: validated in, metadata kept, vertices
	// and indices never retained.
	const MeshResourceHandle handle(m_nextMeshId++);
	m_meshes.push_back(MeshEntry{ handle,
		MeshResourceInfo{
			mesh.GetVertexCount(),
			mesh.GetIndexCount(),
			mesh.GetTriangleCount(),
			mesh.GetTopology(),
			mesh.GetTotalByteCount() } });

	return handle;
}

bool NullAssetUploader::IsValidImage(ImageResourceHandle handle) const noexcept
{
	return IsLive() && FindImage(handle) != nullptr;
}

bool NullAssetUploader::IsValidMesh(MeshResourceHandle handle) const noexcept
{
	return IsLive() && FindMesh(handle) != nullptr;
}

Status NullAssetUploader::ReleaseImage(ImageResourceHandle handle) noexcept
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

	m_images.erase(m_images.begin() + (entry - m_images.data()));
	return Ok();
}

Status NullAssetUploader::ReleaseMesh(MeshResourceHandle handle) noexcept
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

	m_meshes.erase(m_meshes.begin() + (entry - m_meshes.data()));
	return Ok();
}

Result<ImageResourceInfo> NullAssetUploader::GetImageInfo(ImageResourceHandle handle) const
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

Result<MeshResourceInfo> NullAssetUploader::GetMeshInfo(MeshResourceHandle handle) const
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

size_t NullAssetUploader::GetLiveImageCount() const noexcept
{
	return IsLive() ? m_images.size() : 0;
}

size_t NullAssetUploader::GetLiveMeshCount() const noexcept
{
	return IsLive() ? m_meshes.size() : 0;
}

} // namespace Modern::Client
