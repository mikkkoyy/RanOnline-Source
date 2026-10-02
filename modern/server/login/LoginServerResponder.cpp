#include "login/LoginServerResponder.h"

namespace Modern::Server
{
	// A namespace alias, not a using-declaration: a using-declaration cannot name a
	// namespace. Deliberately not called `Codec`, which would shadow Modern::Network::Codec.
	namespace ListCodec = Network::GameServerListCodec;
	using Network::WireU8;

	Status LoginServerResponder::BuildResponse(const Network::GameServerGrid& servers,
	                                           std::vector<WireU8>& out) const
	{
		m_lastEntryCount = 0;
		m_lastSkipped    = 0;

		// Canonical order: group ascending, then channel ascending.
		//
		// GameServerGrid::Servers() already yields that order, which is the same
		// nested loop CLoginServer::MsgSndGameSvrInfo walks
		// (s_CLoginServerMsg.cpp:121-134). Preserving it matters: the entry order is
		// part of the protocol as observed, and sorting by name or by id would send
		// a different list than legacy for the same grid.
		for (const Network::GameServerInfo& info : servers.Servers())
		{
			if (!ListCodec::IsAdvertisable(info))
			{
				// nServerMaxClient <= 0: the slot exists but advertises no capacity,
				// so legacy never offers it (s_CLoginServerMsg.cpp:125).
				++m_lastSkipped;
				continue;
			}

			const std::size_t before = out.size();
			if (const Status status = ListCodec::AppendEntry(out, info); status.IsError())
			{
				// Roll back the partial frame so a rejected entry cannot leave bytes
				// that would be misread as the start of the next message.
				out.resize(before);
				m_lastEntryCount = 0;
				m_lastSkipped    = 0;
				return status;
			}
			++m_lastEntryCount;
		}

		// Unconditional, including for an empty list.
		//
		// Legacy logs an error when nothing was sent and then sends this anyway
		// (s_CLoginServerMsg.cpp:136-144). A client completes on the terminator, so
		// omitting it would strand the client in "list still arriving" for ever.
		if (const Status status = ListCodec::AppendListEnd(out); status.IsError())
		{
			return status;
		}

		return Ok();
	}
}
