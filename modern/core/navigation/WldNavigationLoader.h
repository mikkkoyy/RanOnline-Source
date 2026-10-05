#pragma once

// WORLD-ENTRY-002d: turn a `.wld` file on disk into an immutable
// `NavigationMesh`.
//
// This is the top of the asset path and the only part that touches the
// filesystem:
//
//     WLD bytes -> validation -> optional decryption -> navigation marker
//               -> bExist -> NavigationCell records -> NavigationMesh::Build
//
// It deliberately does NOT know about mapslist.mst, `.lev` files, fields,
// characters, or any packet. The map registry is a separate concern and is a
// later part; this API takes a path.
//
// ---------------------------------------------------------------------------
// THREAD SAFETY
// ---------------------------------------------------------------------------
//
// The loader owns everything mutable it touches: it reads the file into its own
// buffer, decrypts its own buffer, and builds its own mesh. There is no shared
// decoder and no static scratch state, so any number of Field workers may load
// concurrently. `WldCrypt` is three constants and a byte loop for the same
// reason.
//
// The MESH is the shared object, and it is immutable once returned: `const`
// queries only, and the A* state lives in the caller's `NavigationSearchSession`
// rather than on the mesh or its cells. One load, one mesh, many readers.

#include "navigation/NavigationMesh.h"
#include "navigation/WldNavigationReader.h"

#include <memory>
#include <string>

namespace Modern::Navigation
{
	// What a load produced.
	//
	// `status` carries the outcome; `mesh` is non-null only for `Loaded`. The two
	// travel together because the interesting case - `NoNavigation` - is a
	// successful load of a file that has nothing to give.
	struct NavigationLoadResult
	{
		NavigationLoadStatus              status = NavigationLoadStatus::InvalidFile;
		std::shared_ptr<NavigationMesh>   mesh{};
		WldNavigationError                error{};
		std::uint32_t                     decryptPasses = 0;
		bool                              encrypted = false;

		bool Ok() const noexcept { return status == NavigationLoadStatus::Loaded; }
	};

	// The most decryption passes that will be attempted.
	//
	// 1 is what every shipped file needs except one, which needs 2. Four is a
	// deliberate ceiling rather than a loop-until-it-works: a bound is what turns
	// "keeps decrypting a file that is not encrypted" into a reported failure.
	inline constexpr int kMaxDecryptPasses = 4;

	// Loads `path` and builds its navigation mesh.
	//
	// The file is opened read-only and never modified; decryption happens in
	// memory. Nothing is written next to the asset, in the repository, or
	// anywhere else.
	NavigationLoadResult LoadWldNavigationMesh(const std::string& path);

	// The same, from a buffer already in memory.
	//
	// Exposed separately because the malformed-input tests need to drive
	// truncation and corruption without building files, and because a caller that
	// caches file contents should not have to write them out again to reuse this.
	NavigationLoadResult BuildNavigationMeshFromBuffer(const std::uint8_t* data, std::size_t size);
}