#pragma once

// WORLD-ENTRY-001 Phase C: the World Server's configuration.
//
// The whole surface is two bind addresses and the account table. That is
// deliberately the entire configuration: an operator who has to understand more
// than this in order to run the world phase has been handed a milestone's
// internals by mistake.
//
// ---------------------------------------------------------------------------
// WHY THE ACCOUNT TABLE IS CONFIGURATION AND NOT A REPOSITORY QUERY
// ---------------------------------------------------------------------------
//
// The authentication step (LOGIN_2 / LOGIN_FB, already built by WORLD-001 and
// WORLD-002) answers "is this password correct?". It does NOT answer "which
// account id owns these characters?", because nothing in the login path knows that
// a world account id exists.
//
// Legacy has the same gap and closes it the same way: nUserNum is the session's own
// account key, bound from the login result and then used in
// `WHERE ChaNum=%d AND UserNum=%d` (s_COdbcGameCharGet.cpp:41, bound at :650). The
// mapping from a login name to that number lives in the user database, not in the
// login exchange.
//
// So it is configured here, explicitly and inspectably. A test can read the table
// and know exactly which account id its characters must be owned by, rather than
// inferring it from an allocation order.
//
// `userId` is the login name on the wire. `accountId` is the ownership key the
// repository is keyed by. They are DIFFERENT numbers in legacy, and conflating them
// would be the single most damaging shortcut available in this file.

#include "NetworkTransport.h"
#include "types/Result.h"
#include "world/WorldCharacter.h"

#include <string>
#include <vector>

namespace Modern::Server::World
{
	// One login name, and the world account that owns its characters.
	struct WorldAccountBinding
	{
		std::string userId;
		WorldAccountId accountId;
	};

	class WorldServerConfig
	{
	public:
		// Where the Agent role listens. Port 0 asks the OS for a free port; read the
		// real one back from WorldServerRuntime::AgentEndpoint after Start.
		//
		// A client connects HERE for the whole first conversation: 2049, 2247, 2244,
		// 2353 and the 2358 redirect.
		Network::Endpoint agentBind;

		// Where the Field role listens, on its own port. 2358 advertises this
		// endpoint, and the client opens its SECOND connection to it.
		//
		// Deliberately a separate listener rather than a second logical session on the
		// Agent's socket: the brief's whole point is that the client-visible topology
		// is two real connections, and one socket wearing two hats would prove
		// nothing.
		Network::Endpoint fieldBind;

		// Login name -> world account. Must contain every account whose characters
		// the repository holds; an authenticated session with no binding cannot be
		// given an account id and so cannot be authenticated at all.
		std::vector<WorldAccountBinding> accounts;

		// The shared secret LOGIN_2 must carry, checked by LoginReceiver against the
		// passphrase this cluster published (LOGIN-002's seam). Empty disables the
		// check, which exists for tests that are about framing rather than
		// membership.
		std::string expectedEncryptKey;

		// Refuses a configuration that cannot serve: an empty host, a duplicate login
		// name, and two accounts sharing one account id.
		//
		// A duplicate login name is refused because it would make "which account did
		// this session authenticate as" depend on table order - and that answer decides
		// which characters the session may see.
		Status Validate() const;

		// The world account for a login name, or nullptr.
		const WorldAccountBinding* FindAccount(const std::string& userId) const noexcept;

		// Both roles on loopback with OS-assigned ports, and no accounts.
		//
		// The loopback default is what lets an automated test bind without agreeing
		// with the machine in advance on a port number: nothing here reaches a real
		// interface, a real port, or any external host.
		static WorldServerConfig MakeLoopback();
	};
}