#include "navigation/NavigationSearchSession.h"

namespace Modern::Navigation
{
	// Session ids start at 0 and `Setup` increments before use, so the first
	// search runs under session 1 and no array entry - all zero-initialised - can
	// ever read as already-seen. Legacy gets the same effect from
	// `NavigationMesh::BuildNavigationPath` incrementing `m_SessionID` before it
	// searches (navigationmesh.cpp:180).
	NavigationSearchSession::NavigationSearchSession(std::size_t cellCount)
	    : m_seen(cellCount, 0),
	      m_open(cellCount, false),
	      m_arrivalCost(cellCount, 0.0f),
	      m_heuristic(cellCount, 0.0f),
	      m_arrivalWall(cellCount, 0)
	{
	}

	void NavigationSearchSession::Setup(const Vector3& goal) noexcept
	{
		m_goal      = goal;
		++m_sessionId;
		m_nodes.clear();
	}

	void NavigationSearchSession::AddCell(const NavigationCell* cell)
	{
		NavigationNode node;
		node.cell = cell;
		node.cost = PathfindingCost(*cell);

		m_nodes.push_back(node);
		std::push_heap(m_nodes.begin(), m_nodes.end(), Compare{});
	}

	void NavigationSearchSession::AdjustCell(const NavigationCell* cell)
	{
		for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it)
		{
			if (it->cell == cell)
			{
				it->cost = PathfindingCost(*cell);
				std::push_heap(m_nodes.begin(), it + 1, Compare{});
				return;
			}
		}
	}

	void NavigationSearchSession::GetTop(NavigationNode& out)
	{
		out = m_nodes.front();
		std::pop_heap(m_nodes.begin(), m_nodes.end(), Compare{});
		m_nodes.pop_back();
	}
}