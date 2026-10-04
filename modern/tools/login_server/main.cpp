// LOGIN-002: the runnable modern Login Server.
//
// The smallest thing that can be started by hand and watched working. It binds
// loopback, serves the REQ_GAME_SVR exchange, and prints what happened - which is
// the whole point of it existing: before this, there was no way to see the modern
// Login Server talk to anything.
//
// USAGE
//
//     ModernLoginServer [--host <addr>] [--port <n>] [--clients <n>] [--quiet]
//
//   --host     Bind address. Default 127.0.0.1.
//
//   --port     Bind port. Default 0, meaning "let the OS choose", and the port
//              actually assigned is printed on the Listening line. A fixed port is
//              what a deployment would use; 0 is what a test or a second copy on the
//              same machine needs.
//
//   --clients  Serve this many connections and exit. Default 0, meaning "until
//              interrupted". Sequential: one connection is served to completion
//              before the next is accepted.
//
//   --quiet    Suppress the per-connection lines. The Listening line still prints,
//              because a server whose address is not printed cannot be connected to.
//
// WHY NOT A CONFIG FILE.
//
// Legacy reads ServerLogin.cfg through CCfg (s_CServer.cpp:118-143), with the
// filename derived from the executable name. That is the right shape for a
// deployment and it is deliberately not built here: a file format is a decision
// that belongs to whoever owns deployment, and guessing at one now would make it
// harder to change later. Command-line arguments are the smallest thing that lets
// the server be started with different settings today.
//
// The server list is a fixed fixture rather than a configuration input, because
// there is no Session Server to ask yet. In legacy the list is m_sGame, refreshed
// from the Session Server (s_CLoginServer.h:50); hard-coding one here makes the
// missing piece obvious rather than hiding it behind a config option that does
// nothing yet.

#include "login/LoginServerConfig.h"
#include "login/LoginServerRuntime.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace
{
	using namespace Modern;
	using namespace Modern::Server;

	struct Options
	{
		std::string host    = "127.0.0.1";
		unsigned short port = 0;
		int          clients = 0;      // 0 = until interrupted
		bool         quiet   = false;
	};

	// Returns false and prints nothing on success; the caller reports a bad option.
	// Kept separate from main so that every early exit looks the same.
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

			if (argument == "--host" || argument == "--port" || argument == "--clients")
			{
				if (i + 1 >= argc)
				{
					std::printf("LoginServer: %s needs a value\n", argument.c_str());
					return false;
				}
				const std::string value = argv[++i];

				if (argument == "--host")
				{
					out.host = value;
				}
				else if (argument == "--port")
				{
					// strtoul rather than atoi, and checked, because a port that
					// silently became 0 would turn a fixed-port request into an
					// OS-assigned one - and the server would still start, on a
					// different port than the operator asked for.
					char*        end    = nullptr;
					const unsigned long parsed = std::strtoul(value.c_str(), &end, 10);
					if (end == value.c_str() || *end != '\0' || parsed > 65535)
					{
						std::printf("LoginServer: --port needs a number 0-65535, got '%s'\n",
						            value.c_str());
						return false;
					}
					out.port = static_cast<unsigned short>(parsed);
				}
				else
				{
					char*              end    = nullptr;
					const unsigned long parsed = std::strtol(value.c_str(), &end, 10);
					if (end == value.c_str() || *end != '\0' || parsed < 0)
					{
						std::printf("LoginServer: --clients needs a number 0 or more, got '%s'\n",
						            value.c_str());
						return false;
					}
					out.clients = static_cast<int>(parsed);
				}
				continue;
			}

			std::printf("LoginServer: unknown option '%s'\n", argument.c_str());
			std::printf("usage: ModernLoginServer [--host <addr>] [--port <n>] "
			            "[--clients <n>] [--quiet]\n");
			return false;
		}
		return true;
	}

	// Renders one runtime event as the log line the milestone asks for.
	//
	// The wording lives here, in the executable, rather than in the runtime. That is
	// the reason LoginServerEvent and LoginServerLogEntry exist as an enum and a
	// struct: a server that formatted its own messages could not be tested by
	// asserting on what it did, only on how it phrased it.
	void Print(const LoginServerLogEntry& entry)
	{
		switch (entry.event)
		{
		case LoginServerEvent::Listening:
			std::printf("Listening on %s\n", entry.text.c_str());
			break;

		case LoginServerEvent::ClientConnected:
			std::printf("Client connected: %s\n", entry.text.c_str());
			break;

		case LoginServerEvent::RequestReceived:
			std::printf("REQ_GAME_SVR received\n");
			break;

		case LoginServerEvent::ListSent:
			// Two lines for one event, because one event covers the whole response.
			// The responder builds the entries and the terminator into a single buffer
			// and the transport writes it in one call, so there is no separate "the
			// terminator was sent" moment to report - see LoginServerRuntime.h for why
			// the bytes are identical either way.
			std::printf("Sending %d game server%s\n",
			            static_cast<int>(entry.count),
			            entry.count == 1 ? "" : "s");
			std::printf("SND_GAME_SVR_END sent\n");
			break;

		case LoginServerEvent::ClientRejected:
			std::printf("Client rejected: %s\n", entry.text.c_str());
			break;

		case LoginServerEvent::ClientDisconnected:
			std::printf("Client disconnected: %s\n", entry.text.c_str());
			break;
		}
		std::fflush(stdout);
	}
}

int main(int argc, char** argv)
{
	Options options;
	if (!Parse(argc, argv, options))
	{
		return 2;
	}

	LoginServerConfig config;
	config.bind.host = options.host;
	config.bind.port = options.port;
	config.servers   = LoginServerFixture::Default();

	// A --quiet run still prints the address, because a server that will not say
	// where it is cannot be connected to and the whole point of starting it is that
	// something connects to it.
	LoginServerRuntime server(config, [&options](const LoginServerLogEntry& entry) {
		if (options.quiet && entry.event != LoginServerEvent::Listening)
		{
			return;
		}
		Print(entry);
	});

	if (const Status status = server.Start(); status.IsError())
	{
		std::printf("LoginServer: could not start: %s\n", status.GetMessage());
		return 1;
	}

	if (options.clients <= 0)
	{
		std::printf("Serving until interrupted (Ctrl+C).\n");
		std::fflush(stdout);
	}

	// The accept timeout is short and the loop is a plain one. There is no signal
	// handler: Ctrl+C ends the process, and the listener's destructor closes the
	// socket on the way out. Adding a handler to print a farewell would mean claiming
	// an orderly shutdown that nothing verifies.
	//
	// 500ms means an idle server wakes twice a second, which is invisible on a
	// desktop and free on a server. It also means the accept is never the reason a
	// client waits - the client is not waiting for the accept, the kernel completed
	// its handshake already.
	while (options.clients <= 0 || server.ServedClientCount() + server.RefusedClientCount() <
	                               static_cast<std::size_t>(options.clients))
	{
		if (const Status status = server.ServeOneClient(500); status.IsError())
		{
			std::printf("LoginServer: serve failed: %s\n", status.GetMessage());
			return 1;
		}
	}

	server.Stop();

	std::printf("Served %d client%s, refused %d.\n",
	            static_cast<int>(server.ServedClientCount()),
	            server.ServedClientCount() == 1 ? "" : "s",
	            static_cast<int>(server.RefusedClientCount()));
	return 0;
}