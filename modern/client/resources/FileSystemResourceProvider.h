#pragma once

#include "resources/ResourceData.h"
#include "resources/ResourceId.h"
#include "resources/ResourceProvider.h"
#include "types/Result.h"

#include <filesystem>
#include <string_view>

namespace Modern::Client
{

// Filesystem-backed IResourceProvider.
//
// The first real storage backend behind the CLIENT-005 resource boundary:
// a logical ResourceId is resolved against one configured root directory and
// the bytes of the resulting regular file are returned. The abstraction above
// it (IResourceProvider, ResourceManager) is unchanged.
//
// What it deliberately is not:
//   - it is not an archive reader. RCC / CryptionRCC / CCrypt / FileCrypt /
//     SFileSystem keep out of this layer; legacy archives need a separate
//     adapter that implements the same interface (see docs section 18).
//   - it holds no cache. ResourceManager owns caching; a second cache here
//     would make "was this read from disk?" unanswerable.
//   - it knows nothing about rendering, texture/mesh decoding, or GPU upload.
//     It hands back bytes and stops.
//   - it never performs asynchronous or threaded I/O. One call, one read.
//
// Lifetime: like MemoryResourceProvider, the provider owns no shared state and
// must outlive any ResourceManager pointing at it.
//
// Usage:
//   FileSystemResourceProvider provider(rootDirectory);
//   ResourceManager manager;
//   manager.SetProvider(&provider);
//   manager.Initialize();
//   const Result<ResourceId> id = ResourceId::Create("ui/login.bin");
//   const Result<ResourceData> bytes = manager.Load(id.GetValue());
class FileSystemResourceProvider final : public IResourceProvider
{
public:
	// Stores the configured root. Performs no I/O and does not throw, so a bad
	// root is reported by Initialize() rather than from the constructor.
	explicit FileSystemResourceProvider(std::filesystem::path rootDirectory);

	// Validates the configured root and resolves it to a canonical absolute
	// path, then makes the provider usable.
	//
	//   InvalidState    - Initialize() has already succeeded
	//   InvalidArgument - the root is empty, or exists but is not a directory
	//   NotFound        - the root does not exist or cannot be resolved
	//
	// No directory is ever created. A provider pointed at a missing root says
	// so; it does not quietly materialise one.
	Status Initialize();

	bool IsInitialized() const noexcept { return m_initialized; }

	// The root exactly as configured, before canonicalisation.
	const std::filesystem::path& GetConfiguredRoot() const noexcept { return m_root; }

	// The absolute, symlink-resolved root. Empty until Initialize() succeeds.
	const std::filesystem::path& GetResolvedRoot() const noexcept { return m_resolvedRoot; }

	// True only for an existing regular file inside the root.
	//
	// A metadata query: the file is never opened, so no bytes are read. False
	// for a missing path, a directory, a device name, an invalid identifier, an
	// uninitialised provider, and any identifier that would leave the root.
	bool HasResource(const ResourceId& id) const override;

	// Reads the whole file as binary bytes.
	//
	//   InvalidState    - Initialize() has not succeeded
	//   InvalidArgument - the identifier is invalid, non-portable, or would
	//                     leave the configured root
	//   NotFound        - no regular file at the resolved path, or one was found
	//                     but its bytes could not be produced
	Result<ResourceData> Load(const ResourceId& id) override;

private:
	// Extracts a path that is guaranteed to be inside the configured root, or
	// the error explaining why the identifier cannot address one. Performs
	// containment validation only; never reads file contents.
	Result<std::filesystem::path> ResolvePath(const ResourceId& id) const;

	// True when candidate is strictly below the resolved root, comparing path
	// components rather than string prefixes ("assets-extra" must not count as
	// being inside "assets").
	bool IsInsideRoot(const std::filesystem::path& candidate) const noexcept;

	// True when a logical identifier is safe to turn into path components.
	//
	// Policy, in one place: ASCII-printable only (so no encoding confusion
	// between a solidus and a lookalike), no ':' (drive letters, NTFS streams)
	// and no '\' (Windows separators), no empty or trailing-separator
	// components, no "." or ".." components, and no component ending in '.' or
	// ' ' (Windows strips those, which would let two identifiers name one
	// file). A shorter namespace than the filesystem's is the point.
	static bool IsPortableLogicalName(std::string_view name) noexcept;

	std::filesystem::path m_root;
	std::filesystem::path m_resolvedRoot;
	bool                  m_initialized = false;
};

} // namespace Modern::Client
