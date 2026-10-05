#pragma once

// WORLD-ENTRY-002e: the authoritative map-identity registry.
//
// This is the join between three assets that each know only part of the answer:
//
//     MapIdentity --mapslist.mst--> .lev --SLEVEL_HEAD--> .wld --> NavigationMesh
//
// and it is the ONLY place that chain is walked. The three decoders are separate and
// know nothing about each other:
//
//   * `DecodeMapsList`  (MapsList.h)   bytes -> records, no filesystem
//   * `DecodeLevHead`   (LevHead.h)    bytes -> m_strWldFile, no filesystem
//   * `LoadWldNavigationMesh` (002d)    path  -> NavigationMesh
//
// `WldNavigationLoader` is deliberately NOT made aware of map ids: it takes a path,
// and 002d's tests hold without knowing a map exists.
//
// ---------------------------------------------------------------------------
// IDENTITY IS NOT A FILENAME, AND THIS IS WHY
// ---------------------------------------------------------------------------
//
// 002c measured all three facts this type rests on:
//
//   * `m_MapID` is 0 in every one of the 87 shipped `.wld` files, so the `.wld`
//     carries no identity and must never be asked for one.
//   * 16 of the 56 distinct `.wld` files named by the 99 registered maps are named
//     by MORE THAN ONE map id - `suhak.wld` serves three, and `clubwar_inzone.wld`
//     serves the club-war maps. So "the map called suhak.wld" is not a map.
//   * `GLMapList::FindMapNode` looks up by `sNativeID.dwID` (GLMapList.cpp:246) and
//     `GLAgentServer` indexes `m_pLandMan[wMainID][wSubID]` (GLAgentServer.h:184).
//     The `SNATIVEID` pair is the key the game already uses.
//
// Hence `MapIdentity`, hence a many-to-one mesh cache, hence no string identity.
//
// ---------------------------------------------------------------------------
// THE MESH IS SHARED AND IMMUTABLE
// ---------------------------------------------------------------------------
//
// Two map ids naming one `.wld` get ONE `NavigationMesh`, and the registry hands out
// a `shared_ptr<const NavigationMesh>`. Not a copy - a copy of a 13,206-cell mesh
// per map id would multiply the memory the deployed list actually needs, and a copy
// would be a second mutable thing that Field workers could disagree about.
//
// `const` is the whole point. 002d established that `NavigationMesh` has `const` query
// methods only and that the A* state lives in the caller's `NavigationSearchSession`,
// so N Field workers can read one mesh concurrently. This type does not weaken that:
// it stores the mesh as `const` from the moment it is built and never returns a
// mutable handle.
//
// ---------------------------------------------------------------------------
// CONSTRUCTION IS THE ONLY MUTABLE PHASE
// ---------------------------------------------------------------------------
//
// `Load()` resolves every registered map eagerly and then the object is read-only.
// There is no `Resolve(id)` on demand and therefore no lock on the hot path: the
// cache is fully populated before any reader exists, and afterwards `Find` and
// `NavigationMesh` are pure lookups over containers nobody writes. A mutex here
// would protect nothing and cost every query.
//
// If a map fails to resolve, the registry still loads and still reports that map's
// failure status. One missing `.wld` must not take down the other 98.
// A failure to read or parse `mapslist.mst` DOES fail the load, because without it
// there is no map list at all.
//
// ---------------------------------------------------------------------------
// WHAT THIS DELIBERATELY DOES NOT ANSWER
// ---------------------------------------------------------------------------
//
// It resolves map id -> mesh and nothing more. It does not answer "is this a PK
// zone" (those flags are decoded and discarded - see MapsList.h), it does not place
// a character, and it does not tell a Field which channels it serves. Those are
// gameplay, and they belong to the milestone that consumes this one.

#include "map/LevHead.h"
#include "map/MapIdentity.h"
#include "map/MapsList.h"

#include "navigation/NavigationMesh.h"

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Modern::Map
{
	// The outcome of resolving ONE map. Kept as distinct values rather than a bool
	// because the seven failures have seven different fixes, and an operator
	// debugging a real ASURA install needs to be told which one occurred.
	enum class MapResolveStatus
	{
		// The full chain resolved and the `.wld` produced a navigation mesh.
		Loaded,

		// `mapslist.mst` has no record for this identity. Not an error in itself -
		// it is the normal answer for any map id the game has not deployed.
		NotRegistered,

		// In `mapslist.mst`, but RAN's own registration filter would reject it
		// (`bUsed` false, map id out of range, or the name over `MAP_NAME_MAX`).
		// Legacy skips such a record, and so does this.
		Ineligible,

		// `strFile` named a `.lev` that is not in the level directory.
		MissingLev,

		// The `.lev` exists but its `SLEVEL_HEAD` could not be read.
		MalformedLev,

		// The `.lev` named a `.wld` that is not in the map directory.
		MissingWld,

		// The `.wld` exists but the navigation reader refused it.
		MalformedWld,

		// The `.wld` carries a FileID that `NSLANDMAN_SUPPORT::IsLandManSupported`
		// does not list - legacy refuses these too, so this is compatibility rather
		// than a modern limitation.
		UnsupportedWld,

		// The `.wld` is valid and reports `bExist == 0`. A SUCCESSFUL read of a
		// file with nothing in it: the nine character-select and login maps RAN
		// never walks. Never reported as malformed.
		NoNavigation,

		// The `.wld` announced itself encrypted and no decryption pass produced a
		// readable file.
		UndecryptableWld,
	};

	const char* ToString(MapResolveStatus status) noexcept;

	// Everything the registry knows about one map id.
	//
	// Immutable once `Load` returns. `mesh` is non-null exactly when `status` is
	// `Loaded`; the others carry `levelFileName` and, when the `.lev` parsed,
	// `wldFileName` so a failure can be reported against the file that caused it.
	struct MapEntry
	{
		MapIdentity identity{};

		// `strFile`, exactly as `mapslist.mst` spells it - `w_school_01.Lev` and
		// `w_city_s_01.lev` both occur in the deployed file. Case is a FILESYSTEM
		// concern resolved at open time; it is never folded into the identity.
		std::string levelFileName;

		// `m_strWldFile`, exactly as the `.lev` spells it, once the `.lev` parsed.
		std::string wldFileName;

		std::string mapName;

		std::uint32_t fieldServerId = 0;

		MapResolveStatus status = MapResolveStatus::NotRegistered;

		// One short line naming the stage that failed. No buffer dumps.
		std::string detail;

		// The canonical absolute `.wld` path, once resolved. This is the CACHE KEY:
		// two maps naming the same `.wld` - with different case, or through
		// different `.lev` files - get the same key and therefore the same mesh.
		// Empty when the `.wld` was never reached.
		std::string wldPath;

		// Shared, immutable, and null unless `status == Loaded`.
		std::shared_ptr<const Navigation::NavigationMesh> mesh{};

		bool Available() const noexcept { return status == MapResolveStatus::Loaded; }
	};

	// Aggregate counts from a load, so a report does not have to walk 99 entries.
	struct MapRegistrySummary
	{
		// Records RAN's registration filter would keep - the number 002c calls the
		// 99 registered maps.
		std::size_t registered = 0;
		// Records read from the file, including duplicates and ineligible ones.
		std::size_t recordsRead = 0;
		// Registered maps whose `.lev` and `.wld` both exist.
		std::size_t resolved = 0;
		// Registered maps with a navigation mesh. Equal to `resolved` on the
		// deployed ASURA set; tracked separately so a `bExist == 0` map is visible
		// rather than absorbed.
		std::size_t navigationMeshes = 0;
		// Distinct `.wld` files behind those meshes. Less than `navigationMeshes`
		// whenever maps share a file - 99 maps over 56 files in the deployed set.
		std::size_t distinctWlds = 0;

		std::size_t missingLev = 0;
		std::size_t malformedLev = 0;
		std::size_t missingWld = 0;
		std::size_t malformedWld = 0;
		std::size_t unsupportedWld = 0;
		std::size_t noNavigation = 0;
		std::size_t undecryptableWld = 0;
		std::size_t notRegistered = 0;
		std::size_t ineligible = 0;
	};

	// Builds the map id -> record -> `.lev` -> `.wld` -> NavigationMesh registry.
	//
	// The asset root is the root of an ASURA client install. It is a parameter, not a
	// constant: no developer path is compiled in, and the tests supply
	// `RAN_ASSET_ROOT` exactly as the 002d WLD tests do.
	//
	//     set RAN_ASSET_ROOT=D:\FILES\project\RanOnline-Build\ASURA CLIENT
	//
	// Under that root the deployed layout is
	//
	//     <root>/Data/GLogic/mapslist.mst        GLMapList, via GLOGIC::GetPath()
	//     <root>/Data/GLogic/Level/<strFile>     SUBPATH::LEVEL_FILE_ROOT
	//     <root>/Data/Map/<m_strWldFile>         GLLandMan
	//
	// Directory discovery accepts the case variants that ship in the wild
	// (`Data/GLogic` vs the deployed `data/glogic`) by probing a short candidate
	// list, which is also how `modern/tests/WldNavigationTests.cpp` finds
	// `Data/Map`. FILE lookup inside a found directory uses the name the asset
	// spells, letting the platform's own case-insensitive filesystem resolve it.
	class MapRegistry
	{
	public:
		explicit MapRegistry(std::string assetRoot);

		// Reads `mapslist.mst`, then resolves every registered map.
		//
		// Returns false ONLY when `mapslist.mst` itself is missing, unreadable or
		// unparseable. Individual map failures are recorded in their `MapEntry` and
		// counted in the summary; they do not fail the load.
		bool Load();

		// Null until `Load` succeeds, then non-null for every map RAN would register.
		const MapEntry* Find(MapIdentity id) const;

		// The shared immutable mesh, or null. `NavigationUnavailable` is the normal
		// answer for a map id the deployed list does not contain, so a null here is
		// not by itself an error - use `Find` to tell the cases apart.
		std::shared_ptr<const Navigation::NavigationMesh> NavigationMesh(MapIdentity id) const;

		// The `.wld` path for a map, empty when the chain never reached one. Exposed
		// so a caller can log which file a map actually came from, and so the
		// shared-WLD relation is inspectable without inferring it from pointers.
		std::string WldPath(MapIdentity id) const;

		// True once `Load` has succeeded. Nothing mutates after this, so every other
		// member is safe to call concurrently.
		bool Loaded() const noexcept { return m_loaded; }

		const MapRegistrySummary& Summary() const noexcept { return m_summary; }
		const std::vector<MapEntry>& Entries() const noexcept { return m_entries; }

		const std::string& AssetRoot() const noexcept { return m_assetRoot; }

		// Why the last `Load` returned false. Empty after a successful load.
		//
		// `Load` has no other channel to report through - it returns a bool and a
		// per-map failure lands in the entry that caused it - so without this a
		// misconfigured asset root and a corrupt `mapslist.mst` would be
		// indistinguishable. Names the stage and, for the mapslist, carries the
		// decoder's own message: a stage, a short reason and an offset, never a
		// dump of the buffer.
		const std::string& LoadError() const noexcept { return m_loadError; }

		// Human-readable one line per entry, including failures. For a log, not for
		// a test to parse.
		std::vector<std::string> Report() const;

	private:
		// Walks `.lev` -> `.wld` -> mesh for one eligible record and fills `entry`.
		//
		// `meshCache` is passed in rather than being a member so it is obvious that
		// it lives only for the duration of `Load`: a cache that outlived construction
		// would need a lock on the read path to be correct, and this design needs
		// none.
		void ResolveOne(const MapRecord& record, const std::string& levelDirectory,
		                const std::string& mapDirectory,
		                std::unordered_map<std::string,
		                                  std::shared_ptr<const Navigation::NavigationMesh>>& meshCache,
		                MapEntry& entry);

		// Folds the per-entry statuses into `m_summary`. Kept separate from `Load` so
		// there is exactly one place that decides what each status contributes, and a
		// count cannot silently disagree with an entry.
		void Recount();

		// The registry is built once. `m_loaded` guards every accessor so a caller
		// that forgets to check `Load` sees an empty registry rather than a
		// half-populated one.
		std::string m_assetRoot;
		std::string m_loadError;
		bool        m_loaded = false;

		MapRegistrySummary              m_summary{};
		std::vector<MapEntry>          m_entries{};
		std::unordered_map<std::uint32_t, std::size_t> m_index{};
	};
}
