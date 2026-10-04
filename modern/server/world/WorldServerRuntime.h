#pragma once

// WORLD-ENTRY-001 Phase C: the World Server - both roles, one process.
//
// ---------------------------------------------------------------------------
// WHY ONE EXECUTABLE WITH TWO LISTENERS
// ---------------------------------------------------------------------------
//
// The design agreed in the investigation (§9) is two listeners in one process, so
// the client genuinely opens a second TCP connection and 2358/2359 are exercised
// rather than stubbed. A single object owning both is what makes that expressible:
//
//   WorldServerRuntime
//     ├── AgentRoleRuntime   port A   2049 / 2247 / 2244 / 2353 -> 2050 / 2248 /
//     │                                2332 / 2358
//     └── FieldRoleRuntime   port B   2359 -> 2333
//
// Both roles read and write the SAME ICharacterRepository and the SAME
// FieldEntryRegistry, and that sharing is the load-bearing part: the Agent
// authorizes a character into the registry, and the Field validates that same
// registry. Two repositories would make the whole flow meaningless - the Field
// would be authorizing itself against its own copy of the world.
//
// ---------------------------------------------------------------------------
// THE 2358 ENDPOINT IS THE RUNTIME'S OWN, NEVER A CONSTANT
// ---------------------------------------------------------------------------
//
// Start() binds the FIELD listener FIRST, reads back the port the OS assigned, and
// only then starts the Agent and hands it that address. So the redirect a client
// receives names an endpoint that is genuinely listening in this process.
//
// That ordering is the entire reason port 0 is usable. A hard-coded Field port
// would make the integration suite collide with a developer's own server, with
// another run of the suite, and with itself; a configured-then-advertised one makes
// the test deterministic without anyone agreeing on a number in advance.
//
// A Field endpoint that failed to bind is a START failure, not a deferred problem:
// starting an Agent that will advertise an address nobody is listening on would
// produce a client that dials into nothing and is told the redirect was valid.
//
// ---------------------------------------------------------------------------
// STILL NO TCP IN THE SERVICES, AND STILL NO 2356/2357 NETWORK HOP
// ---------------------------------------------------------------------------
//
// CharacterRepository, CharacterSelectService, WorldEntryService and
// FieldEntryRegistry are Phase B's, untouched, and still know nothing about
// sockets. The Agent-to-Field hop remains the in-process call Phase B documented -
// WorldEntryService holds a FieldEntryRegistry by reference and calls Reserve,
// which is 2356 and 2357 collapsed. There is no third listener and no backbone
// protocol, and adding one would be inventing a network hop the client never sees.

#include "login/LoginReceiver.h"
#include "types/Result.h"
#include "world/AgentRoleRuntime.h"
#include "world/CharacterRepository.h"
#include "world/FieldRoleRuntime.h"
#include "world/WorldEntryService.h"
#include "world/WorldServerConfig.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace Modern::Server::World
{
	class WorldServerRuntime
	{
	public:
		// `repository` is the caller's, seeded before Start. Borrowed, not owned: the
		// caller usually wants to inspect what the characters became after the
		// exchange, and a server that copied the world would be a second source of
		// truth about it.
		WorldServerRuntime(WorldServerConfig config,
		                   ILoginAuthenticator& authenticator,
		                   ICharacterRepository& repository);

		~WorldServerRuntime();

		WorldServerRuntime(const WorldServerRuntime&)            = delete;
		WorldServerRuntime& operator=(const WorldServerRuntime&) = delete;

		// Binds the Field role, then the Agent role, then publishes the Field's real
		// endpoint into the redirect. Idempotent-safe to call once; a second Start on
		// a running server is reported rather than silently rebinding.
		Status Start();

		// Stops both listeners. Idempotent, and safe with no server running.
		//
		// Both listeners are closed explicitly rather than left to their destructors so
		// that "stop" is observable at the moment it is called - a server that only
		// stopped when its object was destroyed could not be stopped and restarted,
		// and could not be shut down deliberately.
		void Stop() noexcept;

		bool IsRunning() const noexcept;

		// The addresses actually bound. Empty until Start succeeds.
		Network::Endpoint AgentEndpoint() const noexcept;
		// Named FieldBoundEndpoint() rather than FieldEndpoint(): a member function called
		// FieldEndpoint would shadow the FieldEndpoint TYPE, and the local below would
		// stop compiling - the same trap AgentSession::Account() and
		// WorldEntryService::Endpoint() avoid.
		Network::Endpoint FieldBoundEndpoint() const noexcept;

		// Serves one Agent conversation / one Field claim.
		//
		// Separate calls rather than one ServeOneClient that might take either role:
		// the two listeners are separate sockets and a caller driving them from one
		// thread has to say which it means. Returns Ok when the accept timed out with
		// nobody knocking - an idle pass, not a failure.
		Status ServeOneAgentClient(int timeoutMilliseconds);
		Status ServeOneFieldClient(int timeoutMilliseconds);

		AgentRoleRuntime& Agent() noexcept { return m_agent; }
		FieldRoleRuntime& Field() noexcept { return m_field; }

		const WorldServerConfig& Config() const noexcept { return m_config; }

		// The registry both roles share. Exposed so a test can assert that a
		// successful world entry left exactly one pending authorization, and that a
		// Field claim consumed it.
		FieldEntryRegistry& Registry() noexcept { return m_registry; }

	private:
		WorldServerConfig m_config;
		ICharacterRepository& m_repository;

		// The Agent->Field hop, in process. See the header.
		FieldEntryRegistry m_registry;

		AgentRoleRuntime m_agent;
		FieldRoleRuntime m_field;

		bool m_running = false;
	};
}