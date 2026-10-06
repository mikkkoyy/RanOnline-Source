#pragma once

// WORLD-ENTRY-002f: "give me the immutable mesh for this map id", as an interface.
//
// The movement layer must not know HOW a map becomes a mesh, and it must not know
// WHERE meshes come from. This is the one seam between those two facts:
//
//     MapIdentity -> [ INavigationMapSource ] -> shared_ptr<const NavigationMesh>
//
// ---------------------------------------------------------------------------
// WHY AN INTERFACE AND NOT A POINTER TO MapRegistry
// ---------------------------------------------------------------------------
//
// Three reasons, and the third is the one that decided it.
//
//  1. A movement test that requires a deployed ASURA client cannot run on a machine
//     without one. That would leave the entire GOTO rule - the 60-unit check, the
//     vertical probe, arrival, the wall slide - untested in CI.
//
//  2. `MapRegistry::NavigationMesh` takes a `MapIdentity`, while a character's map
//     arrives as a packed `SNATIVEID` (`ChaSaveMap`). Making the registry the
//     dependency would push an id-conversion into every movement call site, and the
//     conversion is a detail of WHERE the id came from.
//
//  3. `MapRegistry::Load` is an expensive, one-shot, whole-file operation with its
//     own failure vocabulary. An actor asking "what is the mesh for map 7" must not
//     be able to trigger a reload, and must not have to care whether the answer came
//     from a registry, a fixture, or a table.
//
// `MapRegistryMeshSource` below is the production implementation - a thin adapter
// that borrows a loaded registry and does the packed-id conversion in exactly one
// place.
//
// ---------------------------------------------------------------------------
// WHAT THE CONTRACT PROMISES
// ---------------------------------------------------------------------------
//
//   * A returned mesh is SHARED and IMMUTABLE, and outlives the call. Several map
//     identities may receive the SAME mesh, which is exactly the relation 002e
//     measured: 16 of the 56 distinct `.wld` files behind the 99 registered maps
//     are named by more than one map id. A caller must therefore never write
//     through the pointer, and must never destroy it.
//
//   * A null result is a normal answer, not an error. A map that is not registered,
//     whose `.lev` or `.wld` is missing, or whose `.wld` reports `bExist == 0`, has
//     no mesh. `Describe` says which, so a caller can report the reason instead of
//     a bare "no".
//
//   * The source is safe to call concurrently. Both implementations are: the
//     registry adapter reads containers that are frozen after `Load`, and a fixture
//     reads a map it owns.

#include "map/MapIdentity.h"
#include "map/MapRegistry.h"
#include "navigation/NavigationMesh.h"

#include <cstdint>
#include <memory>
#include <string>

namespace Modern::Movement
{
	class INavigationMapSource
	{
	public:
		virtual ~INavigationMapSource() = default;

		// The shared immutable mesh for a PACKED `SNATIVEID`, or null.
		//
		// Packed rather than `MapIdentity`, because that is the shape a character's
		// map field has (`saveMapId`, a `SNATIVEID` union value). The conversion to
		// the two-component identity belongs to whoever owns the id, and doing it
		// inside the adapter keeps it in one place.
		virtual std::shared_ptr<const Navigation::NavigationMesh> MeshForPackedMapId(
		    std::uint32_t packedMapId) const = 0;

		// Why `MeshForPackedMapId` returned null. Empty when it returned a mesh, and
		// never a memory dump - the point is to name the stage that failed.
		virtual std::string Describe(std::uint32_t packedMapId) const = 0;
	};

	// The production source: a `MapRegistry` that has already loaded.
	//
	// Borrows the registry and does not own it. A source that outlived its registry
	// would be a dangling read on the hot path, and the registry is the expensive
	// object - a caller loads once and shares.
	class MapRegistryMeshSource final : public INavigationMapSource
	{
	public:
		explicit MapRegistryMeshSource(const Map::MapRegistry& registry) noexcept
			: m_registry(&registry)
		{
		}

		std::shared_ptr<const Navigation::NavigationMesh> MeshForPackedMapId(
		    std::uint32_t packedMapId) const override;

		std::string Describe(std::uint32_t packedMapId) const override;

		// Whether the borrowed registry actually loaded. A source over a registry
		// that failed would answer "no mesh" for every map, which is indistinguishable
		// from "every map is unreachable" unless this is exposed.
		bool RegistryLoaded() const noexcept;

	private:
		const Map::MapRegistry* m_registry = nullptr;
	};
}
