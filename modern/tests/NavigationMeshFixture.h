#pragma once

// Shared in-memory navigation fixture for the WORLD-ENTRY-002f movement tests.
//
// ---------------------------------------------------------------------------
// WHY THIS IS A HEADER RATHER THAN A THIRD COPY OF THE SAME BUILDER
// ---------------------------------------------------------------------------
//
// `Actor`, `GotoService` and `WorldMovementRuntime` all walk a
// `Navigation::NavigationMesh`, and `NavigationMesh::Build` accepts only what a
// `.wld` navigation block contains: `NavigationCell::kCellRecordBytes` bytes per
// cell. Every movement test therefore needs real 188-byte records, and three test
// binaries in two directories need them for the same geometry.
//
// `modern/tests/TestHarness.h` is the precedent for exactly this: one header,
// reachable from every test binary, holding what they all need.
//
// ---------------------------------------------------------------------------
// WHY THE GEOMETRY IS SYNTHESISED AND NOT READ FROM A SHIPPED MAP
// ---------------------------------------------------------------------------
//
// `WldNavigationTests.cpp` reads real `.wld` files from `RAN_ASSET_ROOT` and SKIPS
// when it is unset - the right policy for a test whose subject is the READER. A
// skipped movement test is not evidence of anything, and the rules that must never
// go unexercised are precisely the 60-unit desynchronisation check, the ±10
// destination probe, arrival and the wall slide. So the geometry is built here,
// in memory, with no asset root and no filesystem at all.
//
// ---------------------------------------------------------------------------
// WHY THE RECORDS ARE WRITTEN OUT BY HAND
// ---------------------------------------------------------------------------
//
// `NavigationCell::RestoreFromRecord` restores the BAKE and never recomputes it
// (NavigationCell.h:31-51), so a fixture that pushed cells through
// `ComputeFromVertices` and read them back would be testing the fixture. The
// arithmetic below is that of `NavigationCell::ComputeFromVertices` field for
// field, and it is short enough to state in full rather than hide behind a call.
//
// The winding is NavigationTests.cpp's: "right of all three walls is inside".
// Getting it backwards inverts every cell-inside test silently, which is why the
// fixture is not trusted blindly - `Movement_FixtureCorridorIsWalkable` asserts
// the three properties everything downstream depends on, in the open.

// ---------------------------------------------------------------------------
// THE FIXTURE
// ---------------------------------------------------------------------------
//
// A flat corridor, `kSegmentCount` ten-unit squares laid along X, each split by a
// diagonal into two triangles:
//
//     z
//     10 +---------+---------+---------+
//        | \     1 | \     3 | \     5 |
//        |  0 \    |  2 \    |  4 \    |
//     0  +---------+---------+---------+ --> x
//       0        10        20        30
//
// Every vertex is at y = 0, so every plane normal is +Y and every cell reads as
// floor - which is what makes the sticky-height-latch and the arrival-snap
// assertions read as numbers rather than as geometry.
//
// The chain links are the ones NavigationTests.cpp uses: within a segment the
// shared diagonal, and between segments the shared x edge. EVERY OTHER EDGE IS
// SOLID, which is deliberate - the corridor's four sides are walls, and the wall
// slide needs a wall to slide against.

#include "math/Vector2.h"
#include "math/Vector3.h"
#include "movement/NavigationMapSource.h"
#include "navigation/NavigationCell.h"
#include "navigation/NavigationGeometry.h"
#include "navigation/NavigationMesh.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace ModernTests
{
	namespace MeshFixture
	{
		// The vocabulary the record writer below needs, unqualified.
		//
		// `ModernTests` is a SIBLING of `Modern`, not a child - it is the harness's own
		// namespace, and a shared header should not impose a file-scope
		// `using namespace Modern` on a test binary it does not own. These two
		// directives are confined to `MeshFixture` for that reason.
		using namespace Modern::Movement;
		using namespace Modern::Navigation;
		using Modern::Vector2;
		using Modern::Vector3;

		// How many ten-unit squares the corridor is `kSegmentCount` long. Three gives
		// a thirty-unit run, which is more than any injected-elapsed-time test needs
		// and short enough that the AABB tree stays shallow.
		constexpr std::size_t kSegmentCount = 3;

		// The corridor's extents, so a caller writes a point rather than a literal.
		// `kSpanX` is the far wall.
		constexpr float kSpanX     = 10.0f * static_cast<float>(kSegmentCount);
		constexpr float kSpanZ     = 10.0f;
		constexpr float kCorridorY = 0.0f;

		// A cell index is `2 * segment + half`, so the mapping a test needs is
		// arithmetic rather than a lookup table the fixture would have to keep in step
		// with the mesh.
		constexpr std::uint32_t CellIdFor(std::size_t segment, int half) noexcept
		{
			return static_cast<std::uint32_t>(segment * 2 + static_cast<std::size_t>(half));
		}

		// A point on the corridor floor. `0 < z < kSpanZ` is inside every square's
		// column, and `0 < x < kSpanX` is inside the corridor.
		inline Vector3 PointAt(float x, float z) noexcept
		{
			return Vector3{x, kCorridorY, z};
		}

		namespace detail
		{
			inline void PutU32(std::uint8_t* record, std::size_t offset, std::uint32_t value) noexcept
			{
				for (std::size_t i = 0; i < 4; ++i)
				{
					record[offset + i] = static_cast<std::uint8_t>((value >> (8 * i)) & 0xFFu);
				}
			}

			// Little-endian by construction, which is how the kernel reads a float out
			// of a record: a DWORD reinterpreted by memcpy (NavigationCell.cpp:18-24).
			// So these bytes do not depend on the host's float layout.
			inline void PutF32(std::uint8_t* record, std::size_t offset, float value) noexcept
			{
				std::uint32_t bits = 0;
				std::memcpy(&bits, &value, sizeof(bits));
				PutU32(record, offset, bits);
			}

			// One `Line2D`'s endpoints, at `offset` inside the record.
			//
			// Only the first sixteen of the twenty-eight bytes per side are endpoints;
			// the trailing eight are the CACHED normal and its validity flag, and
			// `RestoreFromRecord` deliberately ignores them (NavigationCell.cpp:47-53).
			inline void PutSide(std::uint8_t* record, std::size_t offset, Vector2 a, Vector2 b) noexcept
			{
				PutF32(record, offset + 0, a.x);
				PutF32(record, offset + 4, a.y);
				PutF32(record, offset + 8, b.x);
				PutF32(record, offset + 12, b.y);
			}

			// Writes the whole 188-byte record for one triangle.
			//
			// Field for field `NavigationCell::ComputeFromVertices`, including the
			// midpoint pairing that reads C+B and C+A rather than B+C and A+C - that
			// pairing is what puts path waypoints on the right walls
			// (NavigationCell.cpp:114-120).
			inline void WriteCellRecord(std::uint8_t* record, std::uint32_t cellId,
			                     std::uint32_t vertexA, std::uint32_t vertexB,
			                     std::uint32_t vertexC, const Vector3& a, const Vector3& b,
			                     const Vector3& c) noexcept
			{
				std::memset(record, 0, NavigationCell::kCellRecordBytes);

				PutU32(record, NavigationCell::kOffsetCellId, cellId);
				PutU32(record, NavigationCell::kOffsetVertex + 0, vertexA);
				PutU32(record, NavigationCell::kOffsetVertex + 4, vertexB);
				PutU32(record, NavigationCell::kOffsetVertex + 8, vertexC);

				// AB, BC, CA, in the order the file stores them.
				const std::size_t sideBase = NavigationCell::kOffsetSide;
				PutSide(record, sideBase + 0 * 28, Vector2{a.x, a.z}, Vector2{b.x, b.z});
				PutSide(record, sideBase + 1 * 28, Vector2{b.x, b.z}, Vector2{c.x, c.z});
				PutSide(record, sideBase + 2 * 28, Vector2{c.x, c.z}, Vector2{a.x, a.z});

				const Plane         plane(a, b, c);
				const std::size_t   planeBase = NavigationCell::kOffsetPlane;
				PutF32(record, planeBase + 0, plane.Normal().x);
				PutF32(record, planeBase + 4, plane.Normal().y);
				PutF32(record, planeBase + 8, plane.Normal().z);
				PutF32(record, planeBase + 12, plane.Point().x);
				PutF32(record, planeBase + 16, plane.Point().y);
				PutF32(record, planeBase + 20, plane.Point().z);
				PutF32(record, planeBase + 24, plane.Distance());

				const Vector3      centre = (a + b + c) / 3.0f;
				const std::size_t  centreBase = NavigationCell::kOffsetCenterPoint;
				PutF32(record, centreBase + 0, centre.x);
				PutF32(record, centreBase + 4, centre.y);
				PutF32(record, centreBase + 8, centre.z);

				const Vector3 midpoints[3] = {
				    (a + b) / 2.0f,
				    (c + b) / 2.0f,
				    (c + a) / 2.0f,
				};

				for (int i = 0; i < 3; ++i)
				{
					const std::size_t at =
					    NavigationCell::kOffsetWallMidpoint + static_cast<std::size_t>(i) * 12;
					PutF32(record, at + 0, midpoints[i].x);
					PutF32(record, at + 4, midpoints[i].y);
					PutF32(record, at + 8, midpoints[i].z);

					// `ComputeFromVertices` pairs the midpoints cyclically: w01, w12, w20.
					const Vector3 w01 = midpoints[0] - midpoints[1];
					const Vector3 w12 = midpoints[1] - midpoints[2];
					const Vector3 w20 = midpoints[2] - midpoints[0];

					float distance = 0.0f;
					if (i == 0)
					{
						distance = std::sqrt(w01.x * w01.x + w01.y * w01.y + w01.z * w01.z);
					}
					else if (i == 1)
					{
						distance = std::sqrt(w12.x * w12.x + w12.y * w12.y + w12.z * w12.z);
					}
					else
					{
						distance = std::sqrt(w20.x * w20.x + w20.y * w20.y + w20.z * w20.z);
					}

					PutF32(record,
					       NavigationCell::kOffsetWallDistance +
					           static_cast<std::size_t>(i) * 4,
					       distance);
				}
			}
		} // namespace detail

		// The corridor, as a built mesh, or null when `Build` refused it.
		inline std::shared_ptr<NavigationMesh> MakeCorridor()
		{
			std::vector<Vector3>       vertices;
			std::vector<std::uint8_t>  records;
			std::vector<std::uint32_t> links;

			vertices.reserve(kSegmentCount * 4);
			records.reserve(kSegmentCount * 2 * NavigationCell::kCellRecordBytes);
			links.reserve(kSegmentCount * 2 * 3);

			for (std::size_t s = 0; s < kSegmentCount; ++s)
			{
				const float x0 = 10.0f * static_cast<float>(s);
				const float x1 = x0 + 10.0f;

				// Four corners, in the order the two triangles index them. Duplicates
				// across segments are harmless - `Build` only needs the indices to be in
				// range - and a shared vertex pool would make the fixture's arithmetic
				// harder to read for no gain.
				const std::uint32_t base = static_cast<std::uint32_t>(vertices.size());
				vertices.push_back(Vector3{x0, kCorridorY, 0.0f});  // +0
				vertices.push_back(Vector3{x0, kCorridorY, 10.0f}); // +1
				vertices.push_back(Vector3{x1, kCorridorY, 0.0f});  // +2
				vertices.push_back(Vector3{x1, kCorridorY, 10.0f}); // +3

				const std::uint32_t lower = CellIdFor(s, 0);
				const std::uint32_t upper = CellIdFor(s, 1);

				records.resize(records.size() + 2 * NavigationCell::kCellRecordBytes);
				std::uint8_t* lowerRecord =
				    records.data() + static_cast<std::size_t>(lower) *
				                         NavigationCell::kCellRecordBytes;
				std::uint8_t* upperRecord =
				    records.data() + static_cast<std::size_t>(upper) *
				                         NavigationCell::kCellRecordBytes;

				// lower: (x0,0) (x0,10) (x1,0).  upper: (x1,0) (x0,10) (x1,10).
				detail::WriteCellRecord(lowerRecord, lower, base + 0, base + 1, base + 2,
				                        vertices[base + 0], vertices[base + 1],
				                        vertices[base + 2]);
				detail::WriteCellRecord(upperRecord, upper, base + 2, base + 1, base + 3,
				                        vertices[base + 2], vertices[base + 1],
				                        vertices[base + 3]);

				// Three solid sides per cell; the links are opened below.
				for (int side = 0; side < 3; ++side)
				{
					links.push_back(NavigationMesh::kNoLink);
					links.push_back(NavigationMesh::kNoLink);
				}

				// The shared diagonal, in both directions.
				links[static_cast<std::size_t>(lower) * 3 + 1] = upper;
				links[static_cast<std::size_t>(upper) * 3 + 0] = lower;
			}

			// The shared x edge between consecutive segments, in both directions. The
			// upper triangle's CA side and the next lower triangle's AB side are the same
			// segment - the pattern NavigationTests.cpp links for its own chain.
			for (std::size_t s = 1; s < kSegmentCount; ++s)
			{
				const std::uint32_t previousUpper = CellIdFor(s - 1, 1);
				const std::uint32_t nextLower    = CellIdFor(s, 0);
				links[static_cast<std::size_t>(previousUpper) * 3 + 2] = nextLower;
				links[static_cast<std::size_t>(nextLower) * 3 + 0] = previousUpper;
			}

			auto mesh = std::make_shared<NavigationMesh>();
			if (!mesh->Build(vertices, records.data(), kSegmentCount * 2, links))
			{
				return nullptr;
			}
			return mesh;
		}

// A single flat square containing an ARBITRARY point, or null when `Build`
		// refused it.
		//
		// `MakeCorridor` is shaped for the injected-elapsed-time tests, which choose
		// their own start and end. The TCP tests cannot: there the start is whatever
		// position the server's own spawn logic put the character at, and that
		// position is not the fixture's to choose. A square placed AROUND a known
		// point is what lets the real end-to-end flow - spawn, then GOTO, over a real
		// socket - run against navigable ground instead of being refused for
		// `RejectedNoMesh` before the test proved anything.
		//
		// The winding, the vertex order and the link array are the corridor's, for
		// the reason given at the top of this file: getting the winding backwards
		// inverts the cell silently.
		inline std::shared_ptr<NavigationMesh> MakeFlatPad(const Vector3& contains,
		                                                   float halfX,
		                                                   float halfZ)
		{
			if (!(halfX > 0.0f) || !(halfZ > 0.0f))
			{
				return nullptr;
			}

			const float x0 = contains.x - halfX;
			const float x1 = contains.x + halfX;
			const float z0 = contains.z - halfZ;
			const float z1 = contains.z + halfZ;
			const float y  = contains.y;

			const std::vector<Vector3> vertices = {
			    Vector3{x0, y, z0}, // +0
			    Vector3{x0, y, z1}, // +1
			    Vector3{x1, y, z0}, // +2
			    Vector3{x1, y, z1}, // +3
			};

			std::vector<std::uint8_t> records(2 * NavigationCell::kCellRecordBytes, 0);
			std::vector<std::uint32_t> links(2 * 3, NavigationMesh::kNoLink);

			detail::WriteCellRecord(records.data(), 0u, 0u, 1u, 2u, vertices[0],
			                        vertices[1], vertices[2]);
			detail::WriteCellRecord(records.data() + NavigationCell::kCellRecordBytes, 1u,
			                        2u, 1u, 3u, vertices[2], vertices[1], vertices[3]);

			// The shared diagonal, opened in both directions - the same link the
			// corridor opens within a segment. A GOTO whose two ends land in the two
			// halves of this square has to cross it.
			links[1] = 1u;
			links[3] = 0u;

			auto mesh = std::make_shared<NavigationMesh>();
			if (!mesh->Build(vertices, records.data(), 2, links))
			{
				return nullptr;
			}
			return mesh;
		}
		// An `INavigationMapSource` over one in-memory mesh.
		//
		// This is the seam NavigationMapSource.h exists for, used for what it was
		// designed for: proving that the movement layer, the GOTO rule and the runtime
		// all work against a mesh that no deployed client was needed to produce.
		class SingleMapSource final : public INavigationMapSource
		{
		public:
			explicit SingleMapSource(std::shared_ptr<const NavigationMesh> mesh,
			                         std::uint32_t packedMapId) noexcept
				: m_mesh(std::move(mesh))
				, m_packedMapId(packedMapId)
			{
			}

			std::shared_ptr<const NavigationMesh> MeshForPackedMapId(
			    std::uint32_t packedMapId) const override
			{
				++calls;
				return packedMapId == m_packedMapId ? m_mesh : nullptr;
			}

			std::string Describe(std::uint32_t packedMapId) const override
			{
				if (packedMapId != m_packedMapId)
				{
					return "the fixture only holds map " + std::to_string(m_packedMapId);
				}
				if (m_mesh == nullptr)
				{
					return "map " + std::to_string(packedMapId) + " has no fixture mesh";
				}
				return "map " + std::to_string(packedMapId) + " has the fixture corridor";
			}

			// How many times the runtime asked. Lets a test prove the source is consulted
			// once per attach rather than once per tick.
			mutable std::size_t calls = 0;

		private:
			std::shared_ptr<const NavigationMesh> m_mesh{};
			std::uint32_t                          m_packedMapId = 0;
		};
	} // namespace MeshFixture
} // namespace ModernTests
