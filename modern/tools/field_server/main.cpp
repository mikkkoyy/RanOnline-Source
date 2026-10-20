// MODERN-FIELD-SERVER-001: the runnable modern Field/World Server.
//
// WHAT THIS IS.
//
// The smallest process that lets the login-to-field path be driven BY HAND and
// watched. It does not implement a server: every rule already lives in
// `WorldServerRuntime`, which owns BOTH listeners the client-visible topology
// needs -
//
//   Agent role   (port A)   LOGIN_2 2049 / list 2247 / detail 2244 / select 2353
//                           -> 2050 / 2248 / 2332 / 2358 (the field redirect)
//   Field role   (port B)   2359 JOIN_FIELD_IDENTITY -> 2333 spawn, then the
//                           long-lived 3032/3034/3036 sessions
//
// - and this executable only seeds a world, starts it, prints where it is, and
// serves the Agent role from a loopback thread until interrupted.
//
// WHY TWO LISTENERS IN ONE PROCESS IS THE POINT.
//
// The 2358 the Agent emits names the Field role's REAL bound port. A client that
// completes the handoff therefore opens a SECOND socket to an endpoint this
// process is genuinely listening on - which is what makes the path "login to
// field" rather than "login to a port number". WorldServerRuntime::Start binds
// the Field FIRST and advertises it SECOND, so the first 2358 this process can
// emit already names a live endpoint (see WorldServerRuntime.h).
//
// THE ACCOUNT STORE IS A TEST STORE, AND SAYS SO.
//
// `InMemoryLoginAuthenticator` is the seam a real account store will implement
// (LoginReceiver.h). Credentials here are a fixed, documented fixture so an
// automated test can log in deterministically. This is NOT production
// authentication and the binary is not claimable as a RAN server because it
// serves one movement message.
//
// USAGE
//
//     ModernFieldServer [--host <addr>] [--agent-port <n>] [--field-port <n>]
//                       [--quiet]
//
//   --host         Bind address for BOTH roles. Default 127.0.0.1.
//
//   --agent-port   Where the Agent role listens. Default 0, meaning "let the OS
//                  choose"; the assigned port is printed on the Listening line.
//
//   --field-port   Where the Field role listens. Default 0, same convention.
//                  A fixed value is what a deployment would use; 0 is what a
//                  second copy on the same machine needs.
//
//   --quiet        Suppress the per-event lines. The Listening lines still
//                  print, because a server that will not say where it is cannot
//                  be connected to.
//
// NO CONFIG FILE, for the same reason LOGIN-002's launcher has none: a file
// format is a deployment decision and guessing at one now would make it harder
// to change later. Command-line arguments are the smallest thing that lets the
// server be started with different settings today.

#include "login/LoginReceiver.h"
#include "world/CharacterRepository.h"
#include "world/WorldCharacter.h"
#include "world/WorldServerConfig.h"
#include "world/WorldServerRuntime.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <thread>

namespace
{
	using namespace Modern;
	using namespace Modern::Server;
	using namespace Modern::Server::World;

	// -----------------------------------------------------------------------
	// The test world. Fixed ids and credentials so a test can name them.
	// -----------------------------------------------------------------------

	constexpr WorldAccountId   kAccountOne{ 1001 };
	constexpr WorldCharacterId kCharacterOne{ 5001 };
	constexpr WorldCharacterId kCharacterTwo{ 5002 };

	constexpr const char* kUserId   = "test_user";
	constexpr const char* kPassword = "test_pass";

	// One character, built the way WorldEntryTcpTests builds its fixture. Kept
	// here rather than in the library because it describes THIS executable's
	// demo world, not a rule.
	WorldCharacter MakeCharacter(WorldCharacterId id, WorldAccountId accountId,
	                             std::string name, std::string userId,
	                             Network::WireU32 characterClass,
	                             Network::WireU16 level)
	{
		WorldCharacter character;
		character.id             = id;
		character.accountId      = accountId;
		character.userId         = std::move(userId);
		character.name           = std::move(name);
		character.characterClass = characterClass;
		character.school         = 1;
		character.level          = level;

		character.hp = { 300u * level, 300u * level };
		character.mp = { 150u * level, 150u * level };
		character.sp = { 80u * level, 80u * level };

		// The ASURA campus map the modern client already loads, and a position on
		// it, so the spawned character is somewhere a client can render.
		character.saveMapId.value = 7u;
		character.savePosition   = { 100.5f, -20.25f, 3.75f };
		return character;
	}

	void SeedRepository(InMemoryCharacterRepository& repository)
	{
		(void) repository.Add(MakeCharacter(kCharacterOne, kAccountOne, "Tester",
		                                    kUserId, 3u, 10));
		(void) repository.Add(MakeCharacter(kCharacterTwo, kAccountOne, "TesterTwo",
		                                    kUserId, 3u, 20));
	}

	// -----------------------------------------------------------------------
	// Options
	// -----------------------------------------------------------------------

	struct Options
	{
		std::string    host      = "127.0.0.1";
		unsigned short agentPort = 0;
		unsigned short fieldPort = 0;
		bool           quiet     = false;
	};

	bool ParsePort(const std::string& argument, const std::string& value,
	               unsigned short& out)
	{
		char*              end    = nullptr;
		const unsigned long parsed = std::strtoul(value.c_str(), &end, 10);
		if (end == value.c_str() || *end != '\0' || parsed > 65535)
		{
			std::printf("FieldServer: %s needs a number 0-65535, got '%s'\n",
			            argument.c_str(), value.c_str());
			return false;
		}
		out = static_cast<unsigned short>(parsed);
		return true;
	}

	bool Parse(int argc, char** argv, Options& out)
	{
		for (int i = 1; i < argc; ++i)
		{
			const std::string argument = argv[i];

			if (argument == "--quiet")
			{
				out.quiet = true;
				continue;
			}

			if (argument == "--host" || argument == "--agent-port" ||
			    argument == "--field-port")
			{
				if (i + 1 >= argc)
				{
					std::printf("FieldServer: %s needs a value\n", argument.c_str());
					return false;
				}
				const std::string value = argv[++i];

				if (argument == "--host")
				{
					out.host = value;
				}
				else if (argument == "--agent-port")
				{
					if (!ParsePort(argument, value, out.agentPort)) { return false; }
				}
				else
				{
					if (!ParsePort(argument, value, out.fieldPort)) { return false; }
				}
				continue;
			}

			std::printf("FieldServer: unknown option '%s'\n", argument.c_str());
			std::printf("usage: ModernFieldServer [--host <addr>] "
			            "[--agent-port <n>] [--field-port <n>] [--quiet]\n");
			return false;
		}
		return true;
	}
}

int main(int argc, char** argv)
{
	Options options;
	if (!Parse(argc, argv, options))
	{
		return 2;
	}

	// ---- the world -------------------------------------------------------
	InMemoryLoginAuthenticator  authenticator;
	InMemoryCharacterRepository repository;
	SeedRepository(repository);
	authenticator.AddAccount(kUserId, kPassword);

	WorldServerConfig config;
	config.agentBind.host = options.host;
	config.agentBind.port = options.agentPort;
	config.fieldBind.host = options.host;
	config.fieldBind.port = options.fieldPort;
	config.accounts.push_back({ kUserId, kAccountOne });

	if (const Status status = config.Validate(); status.IsError())
	{
		std::printf("FieldServer: configuration refused: %s\n", status.GetMessage());
		return 1;
	}

	WorldServerRuntime runtime(config, authenticator, repository);

	if (const Status status = runtime.Start(); status.IsError())
	{
		std::printf("FieldServer: could not start: %s\n", status.GetMessage());
		return 1;
	}

	// The address the Agent advertises is the Field role's REAL port. Printing
	// both is what lets a client - or a person - connect: a line that printed the
	// port that was ASKED for (0) would name nothing reachable.
	const Network::Endpoint agent = runtime.AgentEndpoint();
	const Network::Endpoint field = runtime.FieldBoundEndpoint();

	std::printf("Listening: agent %s:%u\n", agent.host.c_str(),
	            static_cast<unsigned int>(agent.port));
	std::printf("Listening: field %s:%u\n", field.host.c_str(),
	            static_cast<unsigned int>(field.port));
	std::printf("World: %zu character(s), login '%s' / '%s'\n",
	            repository.Size(), kUserId, kPassword);
	std::fflush(stdout);

	// The Agent conversation is short-lived (login -> list -> select -> 2358),
	// so it is driven from one loopback thread. The Field role owns its own
	// accept thread and one worker per connection from Start(); there is
	// deliberately no Field thread here, exactly as WorldEntryTcpTests explains:
	// a single-threaded pump could not serve two clients each holding a Field
	// connection open waiting for the other's broadcast.
	std::atomic<bool> stop{ false };
	std::thread agentThread([&] {
		while (!stop.load(std::memory_order_acquire))
		{
			if (runtime.ServeOneAgentClient(200).IsError())
			{
				// InvalidState after Stop, or a listener failure: leave the loop.
				return;
			}
		}
	});

	if (options.quiet)
	{
		std::printf("Serving until interrupted (Ctrl+C).\n");
		std::fflush(stdout);
	}

	// A plain wait. Ctrl+C ends the process; the runtime's destructor stops both
	// listeners on the way out. No signal handler is installed: a farewell line
	// would claim an orderly shutdown that nothing verifies.
	while (!stop.load(std::memory_order_acquire))
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}

	stop.store(true, std::memory_order_release);
	if (agentThread.joinable())
	{
		agentThread.join();
	}
	runtime.Stop();

	return 0;
}
