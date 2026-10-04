#pragma once

// LOGIN-002: what the Login Server needs to know to run.
//
// WHY A CONFIG TYPE AT ALL, GIVEN THAT IT HAS THREE FIELDS.
//
// Because the alternative is worse. The tempting shape is for LoginServerRuntime
// to take an Endpoint and a GameServerGrid, which is one fewer type in the
// directory. But then every caller - the executable, the integration test, and
// whatever comes after this milestone - repeats the same three questions in the
// same order: is the bind address usable, is the port sane, is the grid
// something we can actually serve. Answering them once, here, means a malformed
// configuration is reported before a socket is opened rather than by whatever the
// socket layer happened to say about it.
//
// It is deliberately NOT a configuration framework. There is no parser, no file
// format, no defaults table and no inheritance here. Legacy reads ServerLogin.cfg
// through CCfg (s_CServer.cpp:118-143, s_CCfg.cpp:322); a file format is a later
// milestone's decision, not this one's, and guessing at its shape now would make
// it harder to change.
//
// WHERE EACH VALUE COMES FROM.
//
//   bind.host  CCfg::GetServerIP(), token server_ip. Legacy falls back to
//              INADDR_ANY when it is not a number (s_CServer.cpp:625-632); that
//              fallback is deliberately NOT reproduced, because a typo in a
//              bind address silently becoming "every interface" is the kind of
//              surprise a server should never hand to whoever deployed it. This
//              type refuses the address instead.
//   bind.port  CCfg::GetServicePort(), token server_service_port. Legacy ships
//              12004 (CFG/ServerLogin.cfg:9). The CLIENT's hard-coded default is
//              5001 (RANPARAM.cpp:146) and the two disagree in-tree; neither is
//              baked in here. Port 0 means "any free port", which is what makes
//              an automated test possible at all.
//   servers    m_sGame[MAX_SERVER_GROUP][MAX_CHANNEL_NUMBER]
//              (s_CLoginServer.h:50), refreshed from the Session Server. In this
//              milestone it is supplied directly: there is no Session Server to
//              ask yet, so hard-coding one would only move the problem.
//
// Note what is NOT here: a compression switch, a crypt key, an account store, a
// max-client limit, a heartbeat period. None of those are exercised by the
// REQ_GAME_SVR exchange, and a field nobody reads is a field nobody can trust.

#include "GameServerListProtocol.h"
#include "NetworkTransport.h"
#include "types/Result.h"

#include <cstdint>

namespace Modern::Server
{
	struct LoginServerConfig
	{
		// Where to listen.
		//
		// A numeric dotted-quad or a resolvable name - the transport resolves either
		// (SocketAddress.h explains why that is safe even though the RAN CLIENT
		// configuration cannot use a name). "127.0.0.1" restricts the server to
		// loopback; "0.0.0.0" exposes it, and that is a deployment decision this
		// type will make possible but never make on the caller's behalf.
		Network::Endpoint bind;

		// The list to advertise. Grid semantics, not a vector: see GameServerGrid for
		// why duplicate collapse, out-of-range rejection and group-then-channel order
		// are properties of the type rather than of its callers.
		Network::GameServerGrid servers;

		// Checks everything that can be checked without opening a socket.
		//
		// Returns InvalidArgument for an empty or non-numeric host, and for a port
		// above 65535 (which cannot be represented, so this is a type-level guard
		// rather than a likely input - but Endpoint::port is a WireU16, and a caller
		// narrowing a larger number would silently wrap to a different port, which is
		// worth refusing at the boundary).
		//
		// An EMPTY grid is VALID. Legacy sends a bare SND_GAME_SVR_END when it has no
		// servers to advertise (s_CLoginServerMsg.cpp:136-144), and a client that
		// completes only on END would wait forever against silence. "No game servers
		// are running" is a legitimate state a Login Server must be able to report.
		Status Validate() const;

		// True when the bind port asks the OS to choose.
		//
		// Named so callers read as intent rather than as arithmetic. Used by the tests
		// and by the executable to decide whether to print a fixed port or the one
		// that was actually assigned.
		bool UsesEphemeralPort() const noexcept { return bind.port == 0; }
	};

	// ---------------------------------------------------------------------------
	// Fixtures
	// ---------------------------------------------------------------------------
	//
	// A deterministic multi-group, multi-channel list, used by the integration
	// tests and by the executable's default configuration.
	//
	// It exists here rather than in a test file because the executable needs the
	// same thing: a Login Server with an empty list is correct but useless to run
	// by hand, and duplicating the fixture would let the two drift.
	namespace LoginServerFixture
	{
		// Three servers on one group, one on a second group, all on loopback, all
		// with non-zero capacity so the IsAdvertisable filter keeps them.
		//
		// Group 1 deliberately has a GAP: nothing occupies (1, 2), because a sparse
		// grid is the interesting case and a dense one would not notice a client that
		// filled cells in arrival order instead of grid order.
		Network::GameServerGrid Default();

		// A single server on group 0, channel 0.
		Network::GameServerGrid Single();

		// No entries at all. The END-only response.
		Network::GameServerGrid Empty();
	}
}