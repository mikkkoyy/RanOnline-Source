// MODERN-FIELD-SERVER-001: the login-to-field path against the REAL
// ModernFieldServer.exe process.
//
// WHY A SEPARATE BINARY, LAUNCHED AS A CHILD PROCESS.
//
// WorldEntryTcpTests already proves the whole path over two real sockets, but it
// builds its server IN-PROCESS from WorldServerRuntime. That leaves one claim
// untested: that the executable shipped to an operator - started from a shell,
// with a configured bind and a seeded world - serves the same path. A server
// that only works when a test constructs its internals in the right order is not
// a server; it is a fixture with a listener.
//
// So this file starts ModernFieldServer.exe, reads the endpoints it prints on
// stdout, and drives a genuine client (WorldEntryClient over two TcpTransports)
// against it:
//
//   login        -> Agent #1   2049 LOGIN_2      -> 2050 LOGIN_FB
//   character    -> Agent #1   2247 REQ_ALL      -> 2248 SND_ALL
//   select       -> Agent #1   2353 GAME_JOIN    -> 2358 CONNECT_FIELD (redirect)
//   enter field  -> Field #2   2359 JOIN_FIELD   -> 2333 spawn
//   move         -> Field #2   3032 MOVE_STATE   -> 3033 MOVE_STATE_BRD
//
// ACCEPTANCE, in the brief's words, is the movement round trip: a 3032 sent to a
// real process is processed by the authoritative session and answered by a 3033
// that the client decodes back to the state it asked for.
//
// THE TEST READS THE SERVER'S OWN WORDS. The endpoints come from parsing the
// "Listening: agent <host>:<port>" / "Listening: field <host>:<port>" lines the
// executable prints. Nothing agrees on a port in advance: the server binds port
// 0, the OS chooses, and the client dials what the server said. A hard-coded
// port in the test would pass against a server that ignored --agent-port.
//
// EVERY BLOCKING CALL IS BOUNDED. The child is given a deadline; every client
// read carries a deadline. The child is killed and reaped on every exit path, so
// a failed assertion leaves no orphaned process holding a port.
//
// NOTHING HERE REACHES THE NETWORK. The child binds 127.0.0.1 only.

#include "TestHarness.h"

#include "CompressionCodec.h"
#include "GotoProtocol.h"
#include "MovementStateProtocol.h"
#include "NetworkTransport.h"
#include "WorldEntryProtocol.h"
#include "login/World001LoginClient.h"
#include "world/WorldEntryClient.h"
#include "world/WorldEntryConnections.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>
#include <thread>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// <windows.h> defines GetMessage as a macro to GetMessageA/GetMessageW, which
// collides with Modern::Status::GetMessage. The tests that reach a socket have
// to include windows.h for CreateProcess, so the macro is undone here rather
// than avoided - a call site that reads `status.GetMessage()` must keep meaning
// the Status method.
#ifdef GetMessage
#undef GetMessage
#endif

namespace
{
	using namespace Modern;
	using namespace Modern::Network;
	using namespace Modern::Network::MovementState;
	using namespace Modern::Client;

	constexpr int kDeadline = 8000;

	// -----------------------------------------------------------------------
	// The child process.
	// -----------------------------------------------------------------------

	// Where ModernFieldServer.exe lives, relative to THIS test binary.
	//
	// Both are built into the same output directory (build-debug\Debug), so the
	// path is a sibling of the running executable rather than a configured
	// value. A test that could not find the server it is about to test would
	// fail here, at a name, instead of timing out on a connect.
	std::string ServerExePath()
	{
		char module[MAX_PATH] = {};
		const DWORD length = GetModuleFileNameA(nullptr, module, MAX_PATH);
		if (length == 0 || length >= MAX_PATH)
		{
			return std::string();
		}
		std::filesystem::path exe(module);
		return (exe.parent_path() / "ModernFieldServer.exe").string();
	}

	// A ModernFieldServer.exe child, with its stdout captured to a file that is
	// read back once the server has printed its Listening lines.
	//
	// The stdout pipe is a file rather than a pipe because the server runs until
	// killed: a pipe would fill and block the child on a write once the buffer
	// was full, and the child would then never reach the state under test. A file
	// never blocks the writer.
	class ServerProcess
	{
	public:
		~ServerProcess() { Stop(); }

		ServerProcess(const ServerProcess&)            = delete;
		ServerProcess& operator=(const ServerProcess&) = delete;
		ServerProcess()                                = default;

		// Launches the server and waits for it to print both Listening lines.
		bool Start(const std::string& exe, std::string& failure)
		{
			m_logPath = std::filesystem::temp_directory_path() /
			            ("modern_field_server_" +
			             std::to_string(GetCurrentProcessId()) + "_" +
			             std::to_string(reinterpret_cast<uintptr_t>(this)) + ".log");

			SECURITY_ATTRIBUTES attributes = {};
			attributes.nLength              = sizeof(attributes);
			attributes.bInheritHandle       = TRUE;

			m_logHandle = CreateFileA(m_logPath.string().c_str(), GENERIC_WRITE,
			                          FILE_SHARE_READ, &attributes, CREATE_ALWAYS,
			                          FILE_ATTRIBUTE_NORMAL, nullptr);
			if (m_logHandle == INVALID_HANDLE_VALUE)
			{
				failure = "could not open the server log";
				return false;
			}

			// lpApplicationName is the real path, so the executable is never
			// resolved by cmd or a search path. lpCommandLine carries argv[0] and
			// the arguments, quoted conventionally. --quiet suppresses the
			// per-event lines; the Listening lines still print, which is exactly
			// what is needed and nothing more.
			std::string commandLine = "\"" + exe + "\" --quiet";
			std::vector<char> mutableCommand(commandLine.begin(), commandLine.end());
			mutableCommand.push_back('\0');

			STARTUPINFOA startup = {};
			startup.cb           = sizeof(startup);
			startup.dwFlags      = STARTF_USESTDHANDLES;
			startup.hStdInput    = nullptr;
			startup.hStdOutput   = m_logHandle;
			startup.hStdError    = m_logHandle;

			PROCESS_INFORMATION process = {};
			if (CreateProcessA(exe.c_str(), mutableCommand.data(), nullptr, nullptr,
			                   TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &startup,
			                   &process) == FALSE)
			{
				failure = "CreateProcess failed (win32 " +
				          std::to_string(GetLastError()) + ") for " + exe;
				CloseHandle(m_logHandle);
				m_logHandle = INVALID_HANDLE_VALUE;
				return false;
			}

			m_process = process.hProcess;
			CloseHandle(process.hThread);

			if (!WaitForListening(failure))
			{
				Stop();
				return false;
			}
			return true;
		}

		// Polls the log file until both "Listening:" lines are present, or the
		// budget runs out. The server prints them immediately after Start()
		// returns, so this is a short wait in practice.
		bool WaitForListening(std::string& failure)
		{
			const auto deadline = std::chrono::steady_clock::now() +
			                      std::chrono::milliseconds(kDeadline);
			while (std::chrono::steady_clock::now() < deadline)
			{
				if (m_process != nullptr &&
				    WaitForSingleObject(m_process, 0) == WAIT_OBJECT_0)
				{
					failure = "the server process exited before printing its endpoints";
					return false;
				}

				FlushFileBuffers(m_logHandle);
				const std::string text = ReadLog();
				if (text.find("Listening: agent ") != std::string::npos &&
				    text.find("Listening: field ") != std::string::npos)
				{
					return true;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(25));
			}
			failure = "the server did not print its endpoints before the deadline";
			return false;
		}

		// Kills the child and reaps it. Idempotent.
		void Stop()
		{
			if (m_process != nullptr)
			{
				TerminateProcess(m_process, 0);
				WaitForSingleObject(m_process, 5000);
				CloseHandle(m_process);
				m_process = nullptr;
			}
			if (m_logHandle != INVALID_HANDLE_VALUE)
			{
				CloseHandle(m_logHandle);
				m_logHandle = INVALID_HANDLE_VALUE;
			}
			std::error_code ignored;
			std::filesystem::remove(m_logPath, ignored);
		}

		std::string Log() { return ReadLog(); }

	private:
		std::string ReadLog() const
		{
			HANDLE read = CreateFileA(m_logPath.string().c_str(), GENERIC_READ,
			                          FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
			                          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (read == INVALID_HANDLE_VALUE)
			{
				return std::string();
			}

			std::string contents;
			char        buffer[512];
			DWORD       bytesRead = 0;
			while (ReadFile(read, buffer, static_cast<DWORD>(sizeof(buffer)),
			                &bytesRead, nullptr) != FALSE &&
			       bytesRead > 0)
			{
				contents.append(buffer, bytesRead);
			}
			CloseHandle(read);
			return contents;
		}

		std::filesystem::path m_logPath;
		HANDLE                m_logHandle = INVALID_HANDLE_VALUE;
		HANDLE                m_process   = nullptr;
	};

	// Extracts "host:port" from a "Listening: <role> <host>:<port>" line.
	EndpointAddress ParseEndpoint(const std::string& log, const std::string& role)
	{
		EndpointAddress address;
		const std::string marker = "Listening: " + role + " ";
		const std::size_t at     = log.find(marker);
		if (at == std::string::npos)
		{
			return address;
		}
		std::size_t begin = at + marker.size();
		std::size_t end   = log.find('\n', begin);
		if (end == std::string::npos)
		{
			end = log.size();
		}
		// Trim any trailing CR.
		while (end > begin && (log[end - 1] == '\r' || log[end - 1] == ' '))
		{
			--end;
		}
		std::string text = log.substr(begin, end - begin);

		const std::size_t colon = text.rfind(':');
		if (colon == std::string::npos)
		{
			return address;
		}
		address.ip   = text.substr(0, colon);
		address.port = static_cast<WireU16>(std::stoi(text.substr(colon + 1)));
		return address;
	}

	// -----------------------------------------------------------------------
	// The client flow, against the child.
	// -----------------------------------------------------------------------

	constexpr const char* kUserId   = "test_user";
	constexpr const char* kPassword = "test_pass";

	// The character ids ModernFieldServer seeds, from its own main.cpp fixture.
	constexpr WireU32 kCharacterOne = 5001;

	struct FlowOutcome
	{
		bool        ok = false;
		std::string failure;

		WireU32 gaeaId = 0;
		bool    spawnSeen = false;
		WireU32 spawnGaeaId = 0;

		bool    moveSeen = false;
		WireU32 moveState = 0;
		WireU32 moveGaeaId = 0;
	};

	// Runs login -> list -> select -> redirect -> field join -> spawn -> move,
	// leaving nothing open. Every socket is closed before return.
	FlowOutcome RunFullFlow(const EndpointAddress& agentAddress, WireU32 selectId,
	                        bool sendMove)
	{
		FlowOutcome outcome;

		MinLzo1xCodec    codec;
		WorldEntryClient protocol(codec);

		// ---- Agent connection: login ----
		AgentConnection agent(protocol);
		if (const Status status = agent.Connect(agentAddress, kDeadline); status.IsError())
		{
			outcome.failure = std::string("agent connect: ") + status.GetMessage();
			return outcome;
		}

		LoginRequestData login;
		login.userId   = kUserId;
		login.password = kPassword;

		std::vector<WireU8> request;
		if (protocol.BuildLogin(login, request).IsError())
		{
			outcome.failure = "build login";
			return outcome;
		}
		if (agent.Send(request).IsError())
		{
			outcome.failure = "send login";
			return outcome;
		}
		{
			const Status status = agent.PumpUntil(WorldEntryPhase::Authenticated, kDeadline);
			if (status.IsError() || protocol.Phase() == WorldEntryPhase::LoginRejected)
			{
				outcome.failure = std::string("await login: ") + status.GetMessage();
				return outcome;
			}
		}

		// ---- character list ----
		std::vector<WireU8> listRequest;
		if (protocol.BuildRequestCharacterList(listRequest).IsError())
		{
			outcome.failure = "build 2247";
			return outcome;
		}
		if (agent.Send(listRequest).IsError())
		{
			outcome.failure = "send 2247";
			return outcome;
		}
		{
			const Status status =
			    agent.PumpUntil(WorldEntryPhase::CharacterListReady, kDeadline);
			if (status.IsError())
			{
				outcome.failure = std::string("await 2248: ") + status.GetMessage();
				return outcome;
			}
		}

		// ---- select ----
		std::vector<WireU8> select;
		if (protocol.BuildSelectCharacter(selectId, select).IsError())
		{
			outcome.failure = "build 2353";
			return outcome;
		}
		if (agent.Send(select).IsError())
		{
			outcome.failure = "send 2353";
			return outcome;
		}
		{
			const Status status =
			    agent.PumpUntil(WorldEntryPhase::RedirectReceived, kDeadline);
			if (status.IsError())
			{
				outcome.failure = std::string("await 2358: ") + status.GetMessage();
				return outcome;
			}
		}

		outcome.gaeaId = protocol.Redirect().gaeaId;

		// ---- Field connection: join ----
		FieldConnection field(protocol);
		if (const Status status = field.Connect(protocol.Redirect(), kDeadline);
		    status.IsError())
		{
			outcome.failure = std::string("field connect: ") + status.GetMessage();
			return outcome;
		}

		agent.Disconnect();

		FieldIdentity identity;
		identity.joinType       = WorldEntry::kJoinTypeFirst;
		identity.gaeaId         = protocol.Redirect().gaeaId;
		identity.slotFieldAgent = protocol.Redirect().slotFieldAgent;
		identity.cryptKey       = RanWire::DefaultCryptKey();

		if (field.SendIdentity(identity).IsError())
		{
			outcome.failure = "send 2359";
			return outcome;
		}
		if (const Status status = field.PumpUntilSpawn(kDeadline); status.IsError())
		{
			outcome.failure = std::string("await 2333: ") + status.GetMessage();
			return outcome;
		}

		outcome.spawnSeen  = protocol.Spawn().received;
		outcome.spawnGaeaId = protocol.Spawn().gaeaId;

		// ---- movement round trip ----
		if (sendMove)
		{
			if (field.SendMoveState(MovementState::kActRun).IsError())
			{
				outcome.failure = "send 3032";
				return outcome;
			}
			if (const Status status = field.PumpUntilMoveCount(1, kDeadline);
			    status.IsError())
			{
				outcome.failure = std::string("await 3033: ") + status.GetMessage();
				return outcome;
			}
			outcome.moveSeen   = field.MoveState().received;
			outcome.moveState  = field.MoveState().actState;
			outcome.moveGaeaId = field.MoveState().gaeaId;
		}

		field.Disconnect();
		outcome.ok = true;
		return outcome;
	}

	// -----------------------------------------------------------------------
	// Tests.
	// -----------------------------------------------------------------------

	MODERN_TEST(FieldServer_RealProcessServesLoginToFieldMovement)
	{
		const std::string exe = ServerExePath();
		CHECK(!exe.empty());

		ServerProcess server;
		std::string   failure;
		if (!server.Start(exe, failure))
		{
			CHECK_EQ(failure, std::string());
			return;
		}

		const EndpointAddress agent = ParseEndpoint(server.Log(), "agent");
		const EndpointAddress field = ParseEndpoint(server.Log(), "field");

		// Both roles bound real, distinct loopback ports the server itself chose.
		CHECK_EQ(agent.ip, std::string("127.0.0.1"));
		CHECK_EQ(field.ip, std::string("127.0.0.1"));
		CHECK(agent.port != 0);
		CHECK(field.port != 0);
		CHECK_NE(agent.port, field.port);

		const FlowOutcome outcome = RunFullFlow(agent, kCharacterOne, true);

		if (!outcome.failure.empty())
		{
			CHECK_EQ(outcome.failure, std::string());
		}
		CHECK(outcome.ok);
		CHECK(outcome.spawnSeen);
		CHECK_EQ(outcome.spawnGaeaId, outcome.gaeaId);
		CHECK(outcome.gaeaId != 0);

		// ---- the acceptance: the authoritative movement answer ----
		CHECK(outcome.moveSeen);
		CHECK_EQ(outcome.moveGaeaId, outcome.gaeaId);
		CHECK_EQ(outcome.moveState, static_cast<WireU32>(MovementState::kActRun));

		server.Stop();
	}

	MODERN_TEST(FieldServer_RealProcessRefusesInvalidCredentials)
	{
		const std::string exe = ServerExePath();
		CHECK(!exe.empty());

		ServerProcess server;
		std::string   failure;
		if (!server.Start(exe, failure))
		{
			CHECK_EQ(failure, std::string());
			return;
		}

		const EndpointAddress agent = ParseEndpoint(server.Log(), "agent");

		MinLzo1xCodec    codec;
		WorldEntryClient protocol(codec);
		AgentConnection  connection(protocol);
		CHECK(connection.Connect(agent, kDeadline).IsOk());

		LoginRequestData login;
		login.userId   = kUserId;
		login.password = "wrong_password";

		std::vector<WireU8> request;
		CHECK(protocol.BuildLogin(login, request).IsOk());
		CHECK(connection.Send(request).IsOk());

		// The server answers LOGIN_FB with a failure code and closes; the client
		// reaches LoginRejected rather than Authenticated.
		const Status status = connection.PumpUntil(WorldEntryPhase::Authenticated, kDeadline);
		CHECK(status.IsError() || protocol.Phase() == WorldEntryPhase::LoginRejected);
		CHECK(protocol.Phase() != WorldEntryPhase::Authenticated);

		connection.Disconnect();
		server.Stop();
	}

	MODERN_TEST(FieldServer_RealProcessRefusesMovementBeforeEntry)
	{
		const std::string exe = ServerExePath();
		CHECK(!exe.empty());

		ServerProcess server;
		std::string   failure;
		if (!server.Start(exe, failure))
		{
			CHECK_EQ(failure, std::string());
			return;
		}

		const EndpointAddress agent = ParseEndpoint(server.Log(), "agent");
		const EndpointAddress field = ParseEndpoint(server.Log(), "field");

		// A raw Field connection that presents NO 2359 and sends a 3032. The
		// server must drop it rather than attribute a movement to nobody.
		TcpTransport transport;
		Endpoint     target;
		target.host = field.ip;
		target.port = field.port;
		CHECK(transport.Connect(target, kDeadline).IsOk());

		std::vector<WireU8> move;
		MoveStateRequest    request;
		request.actState = MovementState::kActRun;
		CHECK(MovementStateCodec::AppendMoveStateRequest(move, request).IsOk());
		CHECK(transport.Send(move.data(), move.size()).IsOk());

		// No broadcast ever arrives; the read times out or the peer closes.
		WireU8      buffer[64];
		std::size_t received = 0;
		const Status status  = transport.Receive(buffer, sizeof(buffer), received);

		// Either an orderly close (Ok with 0 bytes / error) is acceptable; what
		// must NOT happen is a 3033 coming back.
		(void) status;
		transport.Disconnect();

		// The server is still alive and still serving: a correct client completes
		// the whole flow afterwards.
		const FlowOutcome after = RunFullFlow(agent, kCharacterOne, true);
		CHECK_EQ(after.failure, std::string());
		CHECK(after.ok);
		CHECK(after.moveSeen);

		server.Stop();
	}
}

int main()
{
	std::setvbuf(stdout, nullptr, _IONBF, 0);

	const int failedCases = ModernTests::RunAll();

	if (failedCases == 0)
	{
		std::printf("\nAll %d test cases passed.\n",
		            static_cast<int>(ModernTests::Registry().size()));
		return 0;
	}

	std::printf("\n%d of %d test cases FAILED (%d checks).\n", failedCases,
	            static_cast<int>(ModernTests::Registry().size()),
	            ModernTests::FailureCount());
	return 1;
}
