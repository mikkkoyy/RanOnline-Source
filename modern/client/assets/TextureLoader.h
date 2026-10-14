#pragma once

// CLIENT-RENDER-002: the modern texture loader.
//
// The legacy client resolves a bare texture name (the `TextureFilename` string
// a `.X` material carries, e.g. `blackcatyb_luffy_timeskip.png`) by scanning
// the whole `textures` tree once into a filename index and looking the name up
// case-insensitively, preferring a same-stem `.mtf` over the exact name
// (`TextureManager.cpp`, `g_FileTree.FindPathName`). This loader is that
// behaviour without the D3D coupling: one recursive scan of a textures root
// into a filename index, the same `.mtf`-first lookup order, and a decode
// chosen by the file's own magic bytes rather than by its extension.
//
// The decode dispatch, in order:
//
//   PNG signature       -> PngImageDecoder         (ASURA item/character art)
//   "DDS " magic        -> DdsImageDecoder         (plain DDS textures)
//   MTF header          -> MtfTextureTransform
//                          then DdsImageDecoder    (RAN's obfuscated DDS)
//   anything else       -> InvalidArgument, not a guessed decoder
//
// What this class deliberately is:
//
//   - **Filesystem-scoped, like the legacy index it replaces.** It scans one
//     directory tree, owns the resulting index, and reads the files that index
//     names. It is not a renderer, not a decoder and not a cache: the decoded
//     value is handed to the caller, who owns it (the app uploads it through
//     an IAssetUploader and keeps the handle).
//   - **Deterministic.** The scan visits files in sorted path order and the
//     first file with a given lowercase filename wins the index, so the same
//     tree always resolves the same name to the same file.
//   - **Failure-transparent.** A missing name is NotFound, unreadable bytes
//     are InvalidArgument, and nothing is thrown. A texture that cannot be
//     loaded is a fallback color at the draw site, never a crash here.
//
// What it deliberately is not:
//
//   - **Not a resolver for arbitrary paths.** Only bare filenames are looked
//     up; a name containing a directory separator is refused, because the
//     material strings in RAN `.X` files never carry one and accepting one
//     would let asset data address outside the textures tree.
//   - **Not an MTF-wrapping advocate.** `.mtf` is preferred only because the
//     shipped trees put the encrypted form next to the plain one and the
//     legacy loader looked there first; a plain `.png` or `.dds` with no
//     `.mtf` sibling loads exactly as it always did.

#include "assets/ImageAsset.h"
#include "resources/ResourceData.h"
#include "types/Result.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace Modern::Client
{

class TextureLoader final
{
public:
	// Indexes every regular file under `texturesRoot` by its lowercase
	// filename. Subdirectories are walked recursively, exactly as the legacy
	// `CreateTree` did.
	//
	//   InvalidArgument - the root is empty
	//   NotFound        - the root does not exist or is not a directory
	//
	// Calling Initialize a second time replaces the previous index. No file
	// contents are read here; the scan is names and paths only.
	Status Initialize(const std::filesystem::path& texturesRoot);

	bool IsInitialized() const noexcept { return m_initialized; }

	// Number of filenames the index holds. Files whose lowercase filename
	// collides with an earlier one are counted once (first file wins).
	size_t GetIndexedFileCount() const noexcept { return m_index.size(); }

	// Resolves a bare texture name to the indexed file it names, using the
	// legacy lookup order: `<stem>.mtf` first, then the exact name. Returns an
	// empty path when nothing matches.
	//
	// A name containing a directory separator, or the empty string, is refused
	// (empty result) rather than searched: only bare filenames are addresses
	// here.
	std::filesystem::path Find(const std::string& bareName) const;

	// Resolves `bareName` with Find(), reads the file and decodes it by magic
	// bytes.
	//
	//   InvalidArgument - the name is not a bare filename, or the bytes are
	//                     not a decodable PNG/DDS/MTF texture
	//   NotFound        - the name is not in the index, or its file could not
	//                     be read
	Result<ImageAsset> Load(const std::string& bareName);

	// The decode half of Load(), with no filesystem involved: the bytes are
	// sniffed and handed to the decoder that claims them. Exposed separately
	// so the dispatch can be tested without a tree, and so a caller that
	// already holds the bytes (a resource manager, a test, an archive reader
	// in the future) does not have to round-trip through a file.
	static Result<ImageAsset> Decode(const ResourceData& data);

private:
	// Lowercase filename -> path on disk, in first-file-wins order.
	std::unordered_map<std::string, std::filesystem::path> m_index;
	bool m_initialized = false;
};

} // namespace Modern::Client