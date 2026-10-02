#pragma once

// LOGIN-001: the modern Login Server's answer to REQ_GAME_SVR.
//
// The pre-login counterpart of Modern::Server::LoginResponder. They are siblings,
// not stages of one machine: WORLD-002's responder answers the Agent's LOGIN_2 on
// the Agent connection, this one answers the Login Server's REQ_GAME_SVR on the
// Login connection. Nothing here knows about minTea, LOGIN_2 or LOGIN_FB, and
// LoginResponder knows nothing about the game-server list.
//
// What it reproduces from CLoginServer::MsgSndGameSvrInfo
// (s_CLoginServerMsg.cpp:95-147):
//
//   * walk the (group, channel) grid in that order,
//   * emit an entry only when nServerMaxClient > 0,
//   * ALWAYS finish with a bare SND_GAME_SVR_END, including when nothing was sent.
//
// That last point is the one that is easy to get wrong and that the source settles
// outright: when dwCount == 0 legacy logs "ERROR:Check Session Server Connection"
// and then sends the terminator anyway (lines 136-144). An empty list is therefore
// END-only, not silence. Silence would leave a client that only completes on END
// waiting forever.

#include "GameServerListProtocol.h"
#include "types/Result.h"

#include <cstddef>
#include <vector>

namespace Modern::Server
{
	// Builds the complete game-server-list response for one connection.
	//
	// Stateless and deterministic: the same grid always produces the same bytes.
	// Per-connection send buffering belongs to the transport, which is where V030's
	// ServerBatchEncoder lives - and which this protocol deliberately does NOT use,
	// because legacy sends these messages raw.
	class LoginServerResponder
	{
	public:
		// Appends the full response - zero or more entries, then the terminator -
		// to `out`, so a caller can stream several connections' worth or prepend
		// framing without this type knowing.
		//
		// Returns InvalidArgument if an entry cannot be encoded; because the
		// grid has already been range-checked by GameServerGrid::Add, the only way
		// to reach that is an over-long IP, and it is reported rather than truncated
		// so a misconfigured fixture cannot silently produce an unusable address.
		Status BuildResponse(const Network::GameServerGrid& servers,
		                     std::vector<Network::WireU8>& out) const;

		// How many entries the last BuildResponse emitted. The response carries no
		// count on the wire, so a caller logging the exchange needs it from here.
		std::size_t LastEntryCount() const noexcept { return m_lastEntryCount; }

		// Number of occupied cells that were NOT advertised, i.e. maxClients <= 0.
		// Legacy skips these silently; counting them keeps a misconfigured fixture
		// visible instead of producing a mysteriously short list.
		std::size_t LastSkippedCount() const noexcept { return m_lastSkipped; }

	private:
		mutable std::size_t m_lastEntryCount = 0;
		mutable std::size_t m_lastSkipped    = 0;
	};
}
