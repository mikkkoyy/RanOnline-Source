#pragma once

// WORLD-ENTRY-002d: the A* open set AND the per-cell search bookkeeping.
//
// This replaces legacy/Lib_Engine/NaviMesh/navigationheap.h, but it is not a
// transliteration of it - the shape changed, and the change is the point.
//
// ---------------------------------------------------------------------------
// WHY THE STATE MOVED HERE
// ---------------------------------------------------------------------------
//
// RAN's `NavigationCell` carries its own A* state - `m_SessionID`,
// `m_ArrivalCost`, `m_Heuristic`, `m_Open`, `m_ArrivalWall`
// (navigationcell.h:117-121) - and resets it LAZILY. `QueryForPath` treats a
// cell as unseen whenever its stored session id is older than the mesh's current
// one (navigationcell.cpp:325-327), so a search never walks the mesh to clear
// state; it just bumps a counter. That trick is what makes RAN's search cheap,
// and it is preserved here in the same form.
//
// It is preserved in a different PLACE because of who runs the search. RAN has
// one thread per field server, so two characters never path-find at the same
// instant and sharing those five fields is safe. WORLD-ENTRY-002d runs the
// movement service on a worker thread per Field connection and shares one
// loaded navigation mesh between Fields, so two characters in the same map DO
// path-find concurrently - and with the fields on the cells they would clobber
// each other's arrival costs and each produce the other's path.
//
// So the five fields are arrays here, indexed by `NavigationCell::Index()`, and
// one `NavigationSearchSession` belongs to one search. Two searches are two
// sessions and share nothing. The cell is left read-only, which is what lets a
// loaded `NavigationMesh` be shared across threads without a lock.
//
// ---------------------------------------------------------------------------
// WHY NOT A std::priority_queue
// ---------------------------------------------------------------------------
//
// Because RAN's ordering is not a std::priority_queue's ordering, and matching
// it matters. RAN hand-rolls a binary heap with `D3DXVec3Lerp`-style sift
// (navigationheap.h:167-216) and, notably, its `GetTop` returns the LAST element
// after sifting down from the root rather than the root itself
// (navigationheap.h:222-236). This uses `std::push_heap`/`std::pop_heap` with
// `std::greater`, which is the same min-heap on cost. The ties and the
// traversal order differ from RAN's hand-rolled sift; ties in A* decide which
// of two equal-cost paths is found first, so if this ever needs to match RAN
// exactly the sift has to be transliterated too. It is noted here rather than
// silently assumed equal.
//
// A `std::priority_queue` would give the same pop order, but it cannot do the
// in-place `Adjust` that `AdjustCell` needs (navigationheap.h:204 re-heapifies
// the prefix up to the cell), so the vector + the two algorithm calls stays.

#include "navigation/NavigationCell.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace Modern::Navigation
{
	// One entry in the open set.
	struct NavigationNode
	{
		const NavigationCell* cell = nullptr;
		float                cost = 0.0f; // g + h
	};

	// Per-search A* state.
	//
	// Not thread-safe, and not meant to be: one session belongs to one search on
	// one thread. Sharing happens at the `NavigationMesh` level, above this.
	class NavigationSearchSession
	{
	public:
		// `cellCount` sizes the per-cell arrays; pass `mesh.CellCount()`.
		explicit NavigationSearchSession(std::size_t cellCount);

		// Begins a new search toward `goal` and discards the previous one.
		//
		// Bumping the session id is the whole of the state reset - see the header.
		// The vectors are NOT cleared: every array entry is guarded by the session
		// stamp it was written under, so an entry from an older session reads as
		// unwritten.
		void Setup(const Vector3& goal) noexcept;

		const Vector3& Goal() const noexcept { return m_goal; }
		int           SessionId() const noexcept { return m_sessionId; }

		// ---- per-cell bookkeeping --------------------------------------------

		bool HasSeen(const NavigationCell& cell) const noexcept
		{
			return m_seen[cell.Index()] == m_sessionId;
		}

		bool IsOpen(const NavigationCell& cell) const noexcept
		{
			return HasSeen(cell) && m_open[cell.Index()];
		}

		float ArrivalCost(const NavigationCell& cell) const noexcept
		{
			return m_arrivalCost[cell.Index()];
		}

		float Heuristic(const NavigationCell& cell) const noexcept
		{
			return m_heuristic[cell.Index()];
		}

		int ArrivalWall(const NavigationCell& cell) const noexcept
		{
			return m_arrivalWall[cell.Index()];
		}

		// g + h, the value the heap orders on.
		float PathfindingCost(const NavigationCell& cell) const noexcept
		{
			return ArrivalCost(cell) + Heuristic(cell);
		}

		// Marks the cell unseen under the current session, as
		// `navigationcell.cpp:325-327` does on a session mismatch.
		void MarkUnseen(const NavigationCell& cell) noexcept
		{
			m_seen[cell.Index()] = m_sessionId;
		}

		void SetOpen(const NavigationCell& cell, bool open) noexcept
		{
			m_open[cell.Index()] = open;
		}

		void SetArrivalCost(const NavigationCell& cell, float cost) noexcept
		{
			m_arrivalCost[cell.Index()] = cost;
		}

		void SetHeuristic(const NavigationCell& cell, float value) noexcept
		{
			m_heuristic[cell.Index()] = value;
		}

		void SetArrivalWall(const NavigationCell& cell, int wall) noexcept
		{
			m_arrivalWall[cell.Index()] = wall;
		}

		// ---- the open set -----------------------------------------------------

		void AddCell(const NavigationCell* cell);

		// Re-heapifies the cell after its cost improved.
		//
		// `push_heap` over the PREFIX ending at this cell, not the whole range -
		// navigationheap.h:204 - because the rest of the heap is already ordered.
		void AdjustCell(const NavigationCell* cell);

		bool NotEmpty() const noexcept { return !m_nodes.empty(); }

		void GetTop(NavigationNode& out);

	private:
		std::vector<NavigationNode> m_nodes;
		Vector3                     m_goal{};
		int                         m_sessionId = 0;

		std::vector<int>   m_seen{};
		std::vector<bool>  m_open{};
		std::vector<float> m_arrivalCost{};
		std::vector<float> m_heuristic{};
		std::vector<int>   m_arrivalWall{};

		// `greater` + `push_heap` makes the SMALLEST cost the top, which is what
		// "pop the cheapest open cell" means.
		struct Compare
		{
			bool operator()(const NavigationNode& a, const NavigationNode& b) const noexcept
			{
				return a.cost > b.cost;
			}
		};
	};
}