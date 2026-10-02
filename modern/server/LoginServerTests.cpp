// LOGIN-001: the Login Server endpoint, the client, and the exchange between them.
//
// Deterministic: no socket, no clock, no RNG, no database. Bytes move directly
// from the responder into the client, which is the strongest statement available
// without a live Login Server process.
//
// What these cover that the codec tests cannot:
//   - the response SHAPE (entries in grid order, then an unconditional terminator),
//   - the client's PHASE progression,
//   - survival of arbitrary TCP fragmentation,
//   - the 0 / 1 / 2 / many / maximum entry cases,
//   - that the wire is RAW rather than NET_COMPRESS-enveloped,
//   - and that this stays a SEPARATE conversation from WORLD-002's Agent login.

#include "TestHarness.h"

#include "GameServerListProtocol.h"
#include "NetworkCodec.h"
#include "NetworkConnection.h"
#include "NetworkTypes.h"
#include "LoopbackTransport.h"
#include "NetCompressCodec.h"

#include "login/LoginServerClient.h"
#include "login/LoginServerResponder.h"

namespace
{
	using namespace Modern;
	using namespace Modern::Network;

	// The proven RANPARAM defaults (RANPARAM.cpp:144 and :146), used as a
	// realistic fixture rather than invented values.
	const char* const kDefaultLoginAddress = "211.172.252.50";
	std::uint16_t    kDefaultLoginPort    = 5001;

	GameServerInfo MakeEntry(std::int32_t group, std::int32_t number,
	                         const char* ip, std::int32_t maxClients)
	{
		GameServerInfo info;
		info.ip = ip;
		info.servicePort = 5000 + number;
		info.serverGroup = group;
		info.serverNumber = number;
		info.currentClients = maxClients / 4;
		info.maxClients = maxClients;
		info.pk = true;
		return info;
	}

	// A small, deterministic fixture.
	GameServerGrid MakeFixture()
	{
		GameServerGrid grid;
		(void)grid.Add(MakeEntry(0, 1, "10.0.0.1", 800));
		(void)grid.Add(MakeEntry(0, 3, "10.0.0.3", 800));
		(void)grid.Add(MakeEntry(1, 0, "10.0.1.0", 1500));
		return grid;
	}

	// Brings a client to the point where a response is expected.
	//
	// Returned by value: the client holds no socket or shared handle, only a framer
	// and a grid, so it is a plain movable value.
	Client::LoginServerClient ConnectedClient()
	{
		Client::LoginServerClient client;

		EndpointAddress endpoint;
		endpoint.ip = kDefaultLoginAddress;
		endpoint.port = kDefaultLoginPort;

		(void)client.BeginConnect(endpoint);
		(void)client.CompleteConnect();

		std::vector<WireU8> request;
		(void)client.RequestGameServers(request);
		return client;
	}
}

// ---------------------------------------------------------------------------
// Client phases
// ---------------------------------------------------------------------------

MODERN_TEST(LoginServer_ClientWalksTheLoginPhaseSequence)
{
	Client::LoginServerClient client;
	CHECK(client.Phase() == Client::LoginServerPhase::Disconnected);

	EndpointAddress endpoint;
	endpoint.ip = kDefaultLoginAddress;
	endpoint.port = kDefaultLoginPort;

	CHECK(client.BeginConnect(endpoint).IsOk());
	CHECK(client.Phase() == Client::LoginServerPhase::ConnectingLoginServer);

	CHECK(client.CompleteConnect().IsOk());
	CHECK(client.Phase() == Client::LoginServerPhase::LoginServerConnected);

	std::vector<WireU8> request;
	CHECK(client.RequestGameServers(request).IsOk());
	CHECK(client.Phase() == Client::LoginServerPhase::RequestingGameServers);

	Server::LoginServerResponder responder;
	std::vector<WireU8> response;
	CHECK(responder.BuildResponse(MakeFixture(), response).IsOk());

	std::size_t handled = 0;
	CHECK(client.Feed(response, handled).IsOk());
	CHECK(client.Phase() == Client::LoginServerPhase::GameServersReady);
	CHECK(client.IsComplete());
}

MODERN_TEST(LoginServer_PhaseNamesAreDistinct)
{
	// A log that cannot tell "still receiving" from "ready" is worse than no log.
	CHECK(std::string(ToString(Client::LoginServerPhase::RequestingGameServers)) !=
	      std::string(ToString(Client::LoginServerPhase::ReceivingGameServers)));
	CHECK(std::string(ToString(Client::LoginServerPhase::ReceivingGameServers)) !=
	      std::string(ToString(Client::LoginServerPhase::GameServersReady)));
	CHECK_EQ(std::string(ToString(Client::LoginServerPhase::Disconnected)),
	         std::string("Disconnected"));
}

MODERN_TEST(LoginServer_ThisPhaseIsNotTheAgentLoginPhase)
{
	// The two conversations must stay distinguishable. LoginServerClient has its own
	// phase type precisely so they cannot be confused; asserting the distinct
	// values keeps a future merge from looking harmless.
	CHECK_NE(static_cast<int>(Client::LoginServerPhase::Disconnected),
	         static_cast<int>(Client::LoginServerPhase::GameServersReady));

	// No Agent-login vocabulary leaked into the Login Server surface.
	const std::string gameReady = ToString(Client::LoginServerPhase::GameServersReady);
	CHECK(gameReady.find("LoginAccepted") == std::string::npos);
	CHECK(gameReady.find("LoginRejected") == std::string::npos);
}

MODERN_TEST(LoginServer_CannotCompleteAConnectThatWasNeverStarted)
{
	Client::LoginServerClient client;
	CHECK(client.CompleteConnect().IsError());
	CHECK(client.Phase() == Client::LoginServerPhase::Disconnected);
}

MODERN_TEST(LoginServer_CannotRequestBeforeConnecting)
{
	Client::LoginServerClient client;
	std::vector<WireU8> request;
	CHECK(client.RequestGameServers(request).IsError());
	CHECK(request.empty());
}

MODERN_TEST(LoginServer_CannotReceiveBeforeRequesting)
{
	// Legacy gates its dispatcher on network state (s_NetClientMsg.cpp:28-44), so
	// accepting a list nobody asked for would let a server drive the client.
	Client::LoginServerClient client;

	EndpointAddress endpoint;
	endpoint.ip = kDefaultLoginAddress;
	endpoint.port = kDefaultLoginPort;
	CHECK(client.BeginConnect(endpoint).IsOk());
	CHECK(client.CompleteConnect().IsOk());

	Server::LoginServerResponder responder;
	std::vector<WireU8> response;
	CHECK(responder.BuildResponse(MakeFixture(), response).IsOk());

	std::size_t handled = 0;
	CHECK(client.Feed(response, handled).IsError());
	CHECK(!client.IsComplete());
}

MODERN_TEST(LoginServer_DisconnectReturnsToDisconnected)
{
	Client::LoginServerClient client = ConnectedClient();
	CHECK(client.Disconnect().IsOk());
	CHECK(client.Phase() == Client::LoginServerPhase::Disconnected);
	CHECK(client.Servers().Count() == 0);
	CHECK(!client.IsComplete());
}

// ---------------------------------------------------------------------------
// Endpoint validation
// ---------------------------------------------------------------------------

MODERN_TEST(LoginServer_AHostnameEndpointIsRefused)
{
	// Legacy calls ::inet_addr directly with gethostbyname commented out
	// (s_NetClient.cpp:436-474), so a name cannot resolve.
	Client::LoginServerClient client;

	EndpointAddress endpoint;
	endpoint.ip = "login.ranonline.example";
	endpoint.port = kDefaultLoginPort;

	CHECK(client.BeginConnect(endpoint).IsError());
	CHECK(client.Phase() == Client::LoginServerPhase::Disconnected);
}

MODERN_TEST(LoginServer_EndpointValidationLeavesNoHalfConnectedState)
{
	Client::LoginServerClient client;

	EndpointAddress bad;
	bad.ip = "not.an.address";
	bad.port = kDefaultLoginPort;
	CHECK(client.BeginConnect(bad).IsError());

	// A refused endpoint must not have moved the phase, or CompleteConnect would
	// report success for a connection that was never attempted.
	CHECK(client.Phase() == Client::LoginServerPhase::Disconnected);
	CHECK(client.CompleteConnect().IsError());
}

MODERN_TEST(LoginServer_ZeroPortIsRefused)
{
	Client::LoginServerClient client;
	EndpointAddress endpoint;
	endpoint.ip = kDefaultLoginAddress;
	endpoint.port = 0;
	CHECK(client.BeginConnect(endpoint).IsError());
}

// ---------------------------------------------------------------------------
// Request
// ---------------------------------------------------------------------------

MODERN_TEST(LoginServer_RequestIsEightBytesAndSatisfiesTheServerValidator)
{
	Client::LoginServerClient client;
	EndpointAddress endpoint;
	endpoint.ip = kDefaultLoginAddress;
	endpoint.port = kDefaultLoginPort;
	CHECK(client.BeginConnect(endpoint).IsOk());
	CHECK(client.CompleteConnect().IsOk());

	std::vector<WireU8> request;
	CHECK(client.RequestGameServers(request).IsOk());

	CHECK_EQ(request.size(), std::size_t{ 8 });
	CHECK(GameServerListCodec::ValidateRequest(request).IsOk());
}

MODERN_TEST(LoginServer_RequestingTwiceDiscardsTheFirstList)
{
	// SndReqServerInfo zeroes the grid and clears the END flag in the same breath as
	// sending (s_NetClientMsg.cpp:316-326), so a second request must not be
	// answered by the previous list.
	Client::LoginServerClient client = ConnectedClient();

	Server::LoginServerResponder responder;
	std::vector<WireU8> response;
	CHECK(responder.BuildResponse(MakeFixture(), response).IsOk());

	std::size_t handled = 0;
	CHECK(client.Feed(response, handled).IsOk());
	CHECK_EQ(client.Servers().Count(), std::size_t{ 3 });

	// Re-request.
	std::vector<WireU8> request;
	CHECK(client.RequestGameServers(request).IsOk());

	// The old list is gone immediately, before any new bytes arrive.
	CHECK_EQ(client.Servers().Count(), std::size_t{ 0 });
	CHECK(!client.IsComplete());
	CHECK(client.Phase() == Client::LoginServerPhase::RequestingGameServers);
}

// ---------------------------------------------------------------------------
// End-to-end: entry counts
// ---------------------------------------------------------------------------

MODERN_TEST(LoginServer_EndToEndRoundTripsTheConfiguredFixture)
{
	Client::LoginServerClient client = ConnectedClient();

	const GameServerGrid fixture = MakeFixture();

	Server::LoginServerResponder responder;
	std::vector<WireU8> response;
	CHECK(responder.BuildResponse(fixture, response).IsOk());

	// 3 entries plus the terminator.
	CHECK_EQ(response.size(), GameServerList::kEntrySize * 3 + GameServerList::kBareMessageSize);
	CHECK_EQ(responder.LastEntryCount(), std::size_t{ 3 });

	std::size_t handled = 0;
	CHECK(client.Feed(response, handled).IsOk());
	CHECK_EQ(handled, std::size_t{ 4 }); // three entries and the terminator

	CHECK(client.IsComplete());
	CHECK_EQ(client.Servers().Count(), fixture.Count());

	// The received list must EQUAL the configured fixture, field for field.
	const std::vector<GameServerInfo> expected = fixture.Servers();
	const std::vector<GameServerInfo> actual   = client.Servers().Servers();
	CHECK_EQ(actual.size(), expected.size());
	for (std::size_t i = 0; i < expected.size() && i < actual.size(); ++i)
	{
		CHECK(actual[i] == expected[i]);
	}
}

MODERN_TEST(LoginServer_AnEmptyListIsTheTerminatorAlone)
{
	// The behaviour the source settles outright: when nothing is advertised legacy
	// logs an error and then sends SND_GAME_SVR_END anyway
	// (s_CLoginServerMsg.cpp:136-144). Silence would strand a client that
	// completes on the terminator.
	GameServerGrid empty;

	Server::LoginServerResponder responder;
	std::vector<WireU8> response;
	CHECK(responder.BuildResponse(empty, response).IsOk());

	CHECK_EQ(responder.LastEntryCount(), std::size_t{ 0 });
	CHECK_EQ(response.size(), GameServerList::kBareMessageSize);
	CHECK_EQ(Codec::ReadU32(response.data() + 4), GameServerList::kGameServerListEndId);

	Client::LoginServerClient client = ConnectedClient();
	std::size_t handled = 0;
	CHECK(client.Feed(response, handled).IsOk());

	// Complete, with nothing in it.
	CHECK(client.IsComplete());
	CHECK_EQ(client.Servers().Count(), std::size_t{ 0 });
}

MODERN_TEST(LoginServer_ASingleEntryRoundTrips)
{
	GameServerGrid grid;
	CHECK(grid.Add(MakeEntry(5, 5, "192.168.0.5", 900)).IsOk());

	Server::LoginServerResponder responder;
	std::vector<WireU8> response;
	CHECK(responder.BuildResponse(grid, response).IsOk());
	CHECK_EQ(responder.LastEntryCount(), std::size_t{ 1 });

	Client::LoginServerClient client = ConnectedClient();
	std::size_t handled = 0;
	CHECK(client.Feed(response, handled).IsOk());
	CHECK(client.IsComplete());
	CHECK_EQ(client.Servers().Count(), std::size_t{ 1 });
	CHECK(client.Servers().Servers()[0] == grid.Servers()[0]);
}

MODERN_TEST(LoginServer_TwoEntriesRoundTrip)
{
	GameServerGrid grid;
	CHECK(grid.Add(MakeEntry(0, 0, "10.0.0.0", 100)).IsOk());
	CHECK(grid.Add(MakeEntry(0, 1, "10.0.0.1", 100)).IsOk());

	Server::LoginServerResponder responder;
	std::vector<WireU8> response;
	CHECK(responder.BuildResponse(grid, response).IsOk());
	CHECK_EQ(responder.LastEntryCount(), std::size_t{ 2 });

	Client::LoginServerClient client = ConnectedClient();
	std::size_t handled = 0;
	CHECK(client.Feed(response, handled).IsOk());
	CHECK_EQ(client.Servers().Count(), std::size_t{ 2 });
}

MODERN_TEST(LoginServer_ManyEntriesRoundTrip)
{
	GameServerGrid grid;
	for (std::int32_t g = 0; g < 5; ++g)
	{
		for (std::int32_t n = 0; n < 4; ++n)
		{
			CHECK(grid.Add(MakeEntry(g, n, "172.16.0.1", 400 + g * 10 + n)).IsOk());
		}
	}
	CHECK_EQ(grid.Count(), std::size_t{ 20 });

	Server::LoginServerResponder responder;
	std::vector<WireU8> response;
	CHECK(responder.BuildResponse(grid, response).IsOk());
	CHECK_EQ(responder.LastEntryCount(), std::size_t{ 20 });

	Client::LoginServerClient client = ConnectedClient();
	std::size_t handled = 0;
	CHECK(client.Feed(response, handled).IsOk());

	CHECK(client.IsComplete());
	CHECK_EQ(client.Servers().Count(), std::size_t{ 20 });
	const std::vector<GameServerInfo> expected = grid.Servers();
	const std::vector<GameServerInfo> actual   = client.Servers().Servers();
	CHECK_EQ(actual.size(), expected.size());
	for (std::size_t i = 0; i < expected.size() && i < actual.size(); ++i)
	{
		CHECK(actual[i] == expected[i]);
	}
}

MODERN_TEST(LoginServer_TheMaximumTwoHundredEntriesRoundTrip)
{
	// The maximum is not invented: it is MAX_SERVER_GROUP * MAX_CHANNEL_NUMBER,
	// the size of the grid the server walks (s_CLoginServerMsg.cpp:121-134).
	GameServerGrid grid;
	for (std::int32_t g = 0; g < GameServerList::kMaxServerGroup; ++g)
	{
		for (std::int32_t n = 0; n < GameServerList::kMaxChannelNumber; ++n)
		{
			CHECK(grid.Add(MakeEntry(g, n, "10.1.2.3", 1000)).IsOk());
		}
	}
	CHECK_EQ(grid.Count(), GameServerList::kMaxServers);

	Server::LoginServerResponder responder;
	std::vector<WireU8> response;
	CHECK(responder.BuildResponse(grid, response).IsOk());
	CHECK_EQ(responder.LastEntryCount(), GameServerList::kMaxServers);

	// 11208 bytes in one burst, well past the framer's 2048-byte partial-data
	// capacity. Proves the client slices rather than refusing.
	CHECK_EQ(response.size(), GameServerList::kEntrySize * 200 + GameServerList::kBareMessageSize);
	CHECK(response.size() > 2048);

	Client::LoginServerClient client = ConnectedClient();
	std::size_t handled = 0;
	CHECK(client.Feed(response, handled).IsOk());

	CHECK(client.IsComplete());
	CHECK(!client.IsFailed());
	CHECK_EQ(client.Servers().Count(), GameServerList::kMaxServers);

	const std::vector<GameServerInfo> expected = grid.Servers();
	const std::vector<GameServerInfo> actual   = client.Servers().Servers();
	CHECK_EQ(actual.size(), std::size_t{ 200 });
	for (std::size_t i = 0; i < expected.size() && i < actual.size(); ++i)
	{
		CHECK(actual[i] == expected[i]);
	}
}

MODERN_TEST(LoginServer_EntriesWithNoAdvertisedCapacityAreSkipped)
{
	// nServerMaxClient > 0 is the server's own filter (s_CLoginServerMsg.cpp:125).
	GameServerGrid grid;
	CHECK(grid.Add(MakeEntry(0, 0, "10.0.0.0", 500)).IsOk());
	CHECK(grid.Add(MakeEntry(0, 1, "10.0.0.1", 0)).IsOk());   // no capacity
	CHECK(grid.Add(MakeEntry(0, 2, "10.0.0.2", 500)).IsOk());
	CHECK(grid.Add(MakeEntry(0, 3, "10.0.0.3", -1)).IsOk());  // negative capacity
	CHECK_EQ(grid.Count(), std::size_t{ 4 });

	Server::LoginServerResponder responder;
	std::vector<WireU8> response;
	CHECK(responder.BuildResponse(grid, response).IsOk());

	CHECK_EQ(responder.LastEntryCount(), std::size_t{ 2 });
	CHECK_EQ(responder.LastSkippedCount(), std::size_t{ 2 });

	Client::LoginServerClient client = ConnectedClient();
	std::size_t handled = 0;
	CHECK(client.Feed(response, handled).IsOk());
	CHECK_EQ(client.Servers().Count(), std::size_t{ 2 });
}

// ---------------------------------------------------------------------------
// Ordering
// ---------------------------------------------------------------------------

MODERN_TEST(LoginServer_ResponsePreservesGridOrderNotInsertionOrder)
{
	// Insertion order is deliberately shuffled; the response must come back in
	// group-then-channel order, because that is the loop the server walks.
	GameServerGrid grid;
	CHECK(grid.Add(MakeEntry(2, 1, "10.0.2.1", 100)).IsOk());
	CHECK(grid.Add(MakeEntry(0, 4, "10.0.0.4", 100)).IsOk());
	CHECK(grid.Add(MakeEntry(2, 0, "10.0.2.0", 100)).IsOk());
	CHECK(grid.Add(MakeEntry(0, 1, "10.0.0.1", 100)).IsOk());
	CHECK(grid.Add(MakeEntry(1, 2, "10.0.1.2", 100)).IsOk());

	Server::LoginServerResponder responder;
	std::vector<WireU8> response;
	CHECK(responder.BuildResponse(grid, response).IsOk());

	Client::LoginServerClient client = ConnectedClient();
	std::size_t handled = 0;
	CHECK(client.Feed(response, handled).IsOk());

	const std::vector<GameServerInfo> received = client.Servers().Servers();
	CHECK_EQ(received.size(), std::size_t{ 5 });

	const std::pair<std::int32_t, std::int32_t> expected[] = {
	    { 0, 1 }, { 0, 4 }, { 1, 2 }, { 2, 0 }, { 2, 1 },
	};
	for (std::size_t i = 0; i < received.size() && i < 5; ++i)
	{
		CHECK_EQ(received[i].serverGroup, WireI32{ expected[i].first });
		CHECK_EQ(received[i].serverNumber, WireI32{ expected[i].second });
	}
}

// ---------------------------------------------------------------------------
// Compression and framing on the wire
// ---------------------------------------------------------------------------

MODERN_TEST(LoginServer_TheResponseIsRawNotNetCompressEnveloped)
{
	// The load-bearing finding of this milestone.
	//
	// The Agent's LOGIN_FB IS enveloped (CAgentServer::SendClient ->
	// CClientManager::SendClient -> addSendMsg -> CSendMsgBuffer), but the Login
	// Server's list is NOT: CLoginServer::SendClient -> CClientManager::SendClient2
	// copies dwSize bytes straight out with no batching and no envelope
	// (s_CClientManager.cpp:489-527), and addSendMsg is reached only from
	// CClientManager::SendClient.
	Server::LoginServerResponder responder;
	std::vector<WireU8> response;
	CHECK(responder.BuildResponse(MakeFixture(), response).IsOk());

	// The very first frame on the wire is SND_GAME_SVR, not NET_MSG_COMPRESS.
	CHECK(!NetCompress::IsEnvelope(Codec::ReadU32(response.data() + 4)));
	CHECK_EQ(Codec::ReadU32(response.data() + 4), GameServerList::kGameServerInfoId);
}

MODERN_TEST(LoginServer_TheResponseIsAPlainSequenceOfNetMsgGenericFrames)
{
	Server::LoginServerResponder responder;
	std::vector<WireU8> response;
	CHECK(responder.BuildResponse(MakeFixture(), response).IsOk());

	// Parses with the SAME framer the WORLD-001 request uses - one framing parser,
	// reused, because the wire is raw in both directions.
	ConnectionFramer framer;
	CHECK(framer.Feed(response.data(), response.size()) == FrameStatus::Ok);

	Message message;
	std::vector<MessageId> types;
	while (framer.Next(message) == FrameStatus::Ok)
	{
		types.push_back(message.header.type);
	}
	CHECK(!framer.IsFailed());
	CHECK_EQ(types.size(), std::size_t{ 4 });
	for (std::size_t i = 0; i < 3; ++i)
	{
		CHECK_EQ(types[i], GameServerList::kGameServerInfoId);
	}
	CHECK_EQ(types[3], GameServerList::kGameServerListEndId);

	// Every frame's declared size matches what it actually carried.
	ConnectionFramer verifier;
	CHECK(verifier.Feed(response.data(), response.size()) == FrameStatus::Ok);
	Message check;
	std::size_t total = 0;
	while (verifier.Next(check) == FrameStatus::Ok)
	{
		total += check.header.size;
	}
	CHECK_EQ(total, response.size());
}

// ---------------------------------------------------------------------------
// Fragmented TCP input
// ---------------------------------------------------------------------------

MODERN_TEST(LoginServer_SurvivesDeliveryOneByteAtATime)
{
	// The case that breaks in the field and never in a naive test.
	Server::LoginServerResponder responder;
	std::vector<WireU8> response;
	CHECK(responder.BuildResponse(MakeFixture(), response).IsOk());

	Client::LoginServerClient client = ConnectedClient();
	std::size_t handled = 0;
	bool ready = false;

	for (std::size_t i = 0; i < response.size(); ++i)
	{
		CHECK(client.Feed(response.data() + i, 1, handled).IsOk());
		if (client.IsComplete())
		{
			ready = true;
		}
		CHECK(!client.IsFailed());
	}

	CHECK(ready);
	CHECK_EQ(client.Servers().Count(), std::size_t{ 3 });
}

MODERN_TEST(LoginServer_SurvivesDeliveryTwoBytesAtATime)
{
	Server::LoginServerResponder responder;
	std::vector<WireU8> response;
	CHECK(responder.BuildResponse(MakeFixture(), response).IsOk());

	Client::LoginServerClient client = ConnectedClient();
	std::size_t handled = 0;
	for (std::size_t i = 0; i < response.size(); i += 2)
	{
		const std::size_t chunk = (response.size() - i) < 2 ? (response.size() - i) : 2;
		CHECK(client.Feed(response.data() + i, chunk, handled).IsOk());
	}

	CHECK(client.IsComplete());
	CHECK_EQ(client.Servers().Count(), std::size_t{ 3 });
}

MODERN_TEST(LoginServer_SurvivesTheHeaderBeingSplitAcrossReads)
{
	Server::LoginServerResponder responder;
	std::vector<WireU8> response;
	CHECK(responder.BuildResponse(MakeFixture(), response).IsOk());

	Client::LoginServerClient client = ConnectedClient();
	std::size_t handled = 0;

	// Four bytes of an 8-byte header, then the rest: the framer must not act on a
	// partial size field.
	CHECK(client.Feed(response.data(), 4, handled).IsOk());
	CHECK_EQ(client.Servers().Count(), std::size_t{ 0 });
	CHECK(!client.IsComplete());

	CHECK(client.Feed(response.data() + 4, response.size() - 4, handled).IsOk());
	CHECK(client.IsComplete());
	CHECK_EQ(client.Servers().Count(), std::size_t{ 3 });
}

MODERN_TEST(LoginServer_SurvivesTheEntryBodyBeingSplitAcrossReads)
{
	Server::LoginServerResponder responder;
	std::vector<WireU8> response;
	CHECK(responder.BuildResponse(MakeFixture(), response).IsOk());

	const std::size_t cut = GameServerList::kEntrySize / 2;

	Client::LoginServerClient client = ConnectedClient();
	std::size_t handled = 0;

	// The first entry's header is complete but its body is not.
	CHECK(client.Feed(response.data(), cut, handled).IsOk());
	CHECK_EQ(client.Servers().Count(), std::size_t{ 0 });

	CHECK(client.Feed(response.data() + cut, response.size() - cut, handled).IsOk());
	CHECK(client.IsComplete());
	CHECK_EQ(client.Servers().Count(), std::size_t{ 3 });
}

MODERN_TEST(LoginServer_SurvivesTheTerminatorArrivingAlone)
{
	// The final frame, alone in its own read.
	Server::LoginServerResponder responder;
	std::vector<WireU8> response;
	CHECK(responder.BuildResponse(MakeFixture(), response).IsOk());

	Client::LoginServerClient client = ConnectedClient();
	std::size_t handled = 0;

	const std::size_t entriesBytes = GameServerList::kEntrySize * 3;
	CHECK(client.Feed(response.data(), entriesBytes, handled).IsOk());

	// Entries arrived but the list is NOT complete: there is no count on the wire,
	// so only the terminator can finish it. This is the whole reason the
	// terminator is mandatory even for an empty list.
	CHECK(!client.IsComplete());
	CHECK_EQ(client.Servers().Count(), std::size_t{ 3 });

	CHECK(client.Feed(response.data() + entriesBytes,
	                  response.size() - entriesBytes, handled).IsOk());
	CHECK(client.IsComplete());
}

// ---------------------------------------------------------------------------
// Malformed input
// ---------------------------------------------------------------------------

MODERN_TEST(LoginServer_RejectsAZeroSizedFrame)
{
	Client::LoginServerClient client = ConnectedClient();

	std::vector<WireU8> bad(8, 0); // dwSize = 0, nType = 0
	std::size_t handled = 0;
	CHECK(client.Feed(bad, handled).IsError());
}

MODERN_TEST(LoginServer_RejectsAnImpossibleDeclaredSize)
{
	Client::LoginServerClient client = ConnectedClient();

	std::vector<WireU8> bad(8, 0xFF);
	std::size_t handled = 0;
	CHECK(client.Feed(bad, handled).IsError());
	CHECK(client.IsFailed());
}

MODERN_TEST(LoginServer_RejectsAnEntryWithATruncatedBody)
{
	Client::LoginServerClient client = ConnectedClient();

	// Declares a full entry but only part of it ever arrives, followed by a frame
	// that cannot line up. The framer must not treat the remainder as a message.
	GameServerInfo info = MakeEntry(0, 0, "10.0.0.0", 100);
	std::vector<WireU8> entry;
	CHECK(GameServerListCodec::AppendEntry(entry, info).IsOk());

	std::vector<WireU8> truncated(entry.begin(), entry.end() - 4);
	truncated.insert(truncated.end(), 4, 0xAA);

	std::size_t handled = 0;
	CHECK(client.Feed(truncated, handled).IsError());
}

MODERN_TEST(LoginServer_RejectsAnEntryWhoseBodyIsTheWrongLength)
{
	Client::LoginServerClient client = ConnectedClient();

	GameServerInfo info = MakeEntry(0, 0, "10.0.0.0", 100);
	std::vector<WireU8> entry;
	CHECK(GameServerListCodec::AppendEntry(entry, info).IsOk());

	// An entry declaring 60 bytes instead of 56. The codec refuses it rather than
	// reinterpreting the extra bytes as the start of another field.
	for (int i = 0; i < 4; ++i)
	{
		entry[static_cast<std::size_t>(i)] = 60;
	}
	entry.resize(60);

	std::size_t handled = 0;
	CHECK(client.Feed(entry, handled).IsError());
}

MODERN_TEST(LoginServer_RejectsATerminatorDeclaringABody)
{
	// A frame that CLAIMS nine bytes under the terminator id. The terminator has no
	// body in this protocol, so a peer inventing one is refused rather than read.
	Client::LoginServerClient client = ConnectedClient();

	std::vector<WireU8> bad(9, 0);
	bad[0] = 9; // dwSize = 9
	for (int i = 0; i < 4; ++i)
	{
		bad[4 + static_cast<std::size_t>(i)] =
		    static_cast<WireU8>(GameServerList::kGameServerListEndId >> (8 * i));
	}

	std::size_t handled = 0;
	CHECK(client.Feed(bad, handled).IsError());
	CHECK(!client.IsComplete());
}

MODERN_TEST(LoginServer_AStrayByteAfterTheTerminatorIsLeftPending)
{
	// Eight terminator bytes followed by one stray byte. The framer honours the
	// declared size, so the terminator IS a complete message and the list completes;
	// the stray byte stays buffered awaiting a header that never comes. That is
	// legacy's behaviour too - it reads the sized message, then waits for the next.
	//
	// Asserted rather than left implicit, because "the extra byte was rejected" and
	// "the extra byte is pending" are very different claims and only one is true.
	Client::LoginServerClient client = ConnectedClient();

	std::vector<WireU8> stream;
	CHECK(GameServerListCodec::AppendListEnd(stream).IsOk());
	stream.push_back(0x5A);

	std::size_t handled = 0;
	CHECK(client.Feed(stream, handled).IsOk());
	CHECK_EQ(handled, std::size_t{ 1 });
	CHECK(client.IsComplete());
}

MODERN_TEST(LoginServer_RejectsATruncatedTerminator)
{
	Client::LoginServerClient client = ConnectedClient();

	// Four bytes: half a header. Not an error by itself - it is simply incomplete -
	// but it must never be mistaken for a completed exchange.
	std::vector<WireU8> partial{ 8, 0, 0, 0 };
	std::size_t handled = 0;
	CHECK(client.Feed(partial, handled).IsOk());
	CHECK(!client.IsComplete());
	CHECK_EQ(client.Servers().Count(), std::size_t{ 0 });
}

MODERN_TEST(LoginServer_RejectsAnEntryWithAnUnterminatedString)
{
	Client::LoginServerClient client = ConnectedClient();

	GameServerInfo info = MakeEntry(0, 0, "10.0.0.0", 100);
	std::vector<WireU8> entry;
	CHECK(GameServerListCodec::AppendEntry(entry, info).IsOk());

	// Fill the IP field with non-NUL bytes: a C-string reader would run past it.
	for (std::size_t i = GameServerList::kOffsetServerIp;
	     i < GameServerList::kOffsetServerIp + GameServerList::kServerIpFieldSize; ++i)
	{
		entry[i] = '9';
	}

	std::size_t handled = 0;
	CHECK(client.Feed(entry, handled).IsError());
}

MODERN_TEST(LoginServer_AnOutOfRangeEntryIsSkippedButTheListStillCompletes)
{
	// The asymmetry that matters and that legacy draws explicitly: a WELL-FORMED
	// entry that simply cannot be placed is dropped, while corruption is fatal.
	// MsgGameSvrInfo returns without storing (s_NetClientMsg.cpp:199-203).
	Client::LoginServerClient client = ConnectedClient();

	// Well-formed bytes describing group 99, which the grid cannot hold.
	GameServerInfo stray;
	stray.ip = "10.9.9.9";
	stray.serverGroup = 99;
	stray.serverNumber = 0;
	stray.maxClients = 500;

	std::vector<WireU8> stream;
	// Appended directly, bypassing AppendEntry's own range check, so the CLIENT is
	// the thing under test.
	const std::size_t start = stream.size();
	stream.resize(start + GameServerList::kEntrySize, 0);
	for (int i = 0; i < 4; ++i)
	{
		stream[start + static_cast<std::size_t>(i)] =
		    static_cast<WireU8>(GameServerList::kEntrySize >> (8 * i));
	}
	for (int i = 0; i < 4; ++i)
	{
		stream[start + 4 + static_cast<std::size_t>(i)] =
		    static_cast<WireU8>(GameServerList::kGameServerInfoId >> (8 * i));
	}
	for (std::size_t i = 0; i < stray.ip.size(); ++i)
	{
		stream[start + GameServerList::kOffsetServerIp + i] =
		    static_cast<WireU8>(stray.ip[i]);
	}
	for (int i = 0; i < 4; ++i)
	{
		stream[start + GameServerList::kOffsetServerGroup + static_cast<std::size_t>(i)] =
		    static_cast<WireU8>(static_cast<WireU32>(stray.serverGroup) >> (8 * i));
	}
	for (int i = 0; i < 4; ++i)
	{
		stream[start + GameServerList::kOffsetServerNumber + static_cast<std::size_t>(i)] =
		    static_cast<WireU8>(static_cast<WireU32>(stray.serverNumber) >> (8 * i));
	}

	// A good entry and the terminator follow it.
	CHECK(GameServerListCodec::AppendEntry(stream, MakeEntry(0, 0, "10.0.0.0", 100)).IsOk());
	CHECK(GameServerListCodec::AppendListEnd(stream).IsOk());

	std::size_t handled = 0;
	CHECK(client.Feed(stream, handled).IsOk()); // not an error

	CHECK_EQ(client.DroppedEntryCount(), std::size_t{ 1 });
	CHECK(client.IsComplete());
	CHECK_EQ(client.Servers().Count(), std::size_t{ 1 }); // only the good one
}

MODERN_TEST(LoginServer_AClientControlledCountCannotCauseAnAllocation)
{
	// The response carries no count, so a peer cannot ask for an allocation by
	// claiming one. Feeding a frame whose size field claims far more than the
	// framer's capacity is refused outright.
	Client::LoginServerClient client = ConnectedClient();

	std::vector<WireU8> hostile(8, 0);
	hostile[0] = 0xFF; hostile[1] = 0xFF; hostile[2] = 0xFF; hostile[3] = 0x7F;
	for (int i = 0; i < 4; ++i)
	{
		hostile[4 + static_cast<std::size_t>(i)] =
		    static_cast<WireU8>(GameServerList::kGameServerInfoId >> (8 * i));
	}

	std::size_t handled = 0;
	CHECK(client.Feed(hostile, handled).IsError());
	CHECK(!client.IsComplete());
}

// ---------------------------------------------------------------------------
// Reset
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Integration through the transport abstraction
// ---------------------------------------------------------------------------

MODERN_TEST(LoginServer_ExchangeOverTheTransportAbstraction)
{
	// The strongest integration available in this repository, and it is an honest
	// one: both halves talk through INetworkTransport rather than by handing bytes
	// across a function call.
	//
	// It is NOT a TCP test. modern contains no socket code at all - no winsock, no
	// <winsock2.h>, no socket() call - so a real loopback-socket integration cannot
	// be written until a socket transport exists. LoopbackTransport is an in-process
	// pair of byte queues. See the report's "Live integration" section: BLOCKED, and
	// for this reason among others.
	auto channel = LoopbackTransport::CreatePair();
	LoopbackTransport clientSide(channel.first);
	LoopbackTransport serverSide(channel.second);

	// ---- client connects and requests ----
	Client::LoginServerClient client;
	EndpointAddress endpoint;
	endpoint.ip = kDefaultLoginAddress;
	endpoint.port = kDefaultLoginPort;
	CHECK(client.BeginConnect(endpoint).IsOk());
	CHECK(client.CompleteConnect().IsOk());

	std::vector<WireU8> request;
	CHECK(client.RequestGameServers(request).IsOk());
	CHECK(clientSide.Send(request.data(), request.size()).IsOk());

	// ---- server receives the request over the transport ----
	WireU8 buffer[64];
	std::size_t received = 0;
	CHECK(serverSide.Receive(buffer, sizeof(buffer), received).IsOk());
	CHECK_EQ(received, request.size());
	CHECK(GameServerListCodec::ValidateRequest(
	          std::vector<WireU8>(buffer, buffer + received)).IsOk());

	// ---- server answers ----
	const GameServerGrid fixture = MakeFixture();
	Server::LoginServerResponder responder;
	std::vector<WireU8> response;
	CHECK(responder.BuildResponse(fixture, response).IsOk());
	CHECK(serverSide.Send(response.data(), response.size()).IsOk());

	// ---- client drains the transport in transport-sized chunks ----
	// 64 bytes is smaller than one 56-byte entry plus what follows it, so this also
	// exercises the client's slicing rather than handing it a whole response.
	bool complete = false;
	while (clientSide.PendingBytes() > 0)
	{
		std::size_t got = 0;
		CHECK(clientSide.Receive(buffer, sizeof(buffer), got).IsOk());
		CHECK(got > 0);

		std::size_t handled = 0;
		CHECK(client.Feed(buffer, got, handled).IsOk());
		if (client.IsComplete())
		{
			complete = true;
		}
	}
	CHECK(complete);
	CHECK(client.IsComplete());

	const std::vector<GameServerInfo> expected = fixture.Servers();
	const std::vector<GameServerInfo> actual   = client.Servers().Servers();
	CHECK_EQ(actual.size(), expected.size());
	for (std::size_t i = 0; i < expected.size() && i < actual.size(); ++i)
	{
		CHECK(actual[i] == expected[i]);
	}

	// ---- disconnect ----
	clientSide.Disconnect();
	serverSide.Disconnect();
	CHECK(clientSide.State() == TransportState::Closed);
	CHECK(client.Disconnect().IsOk());
	CHECK(client.Phase() == Client::LoginServerPhase::Disconnected);
}

MODERN_TEST(LoginServer_ResetClearsBufferedBytesAndState)
{
	// A reconnect must not inherit the previous connection's partial frame.
	Server::LoginServerResponder responder;
	std::vector<WireU8> response;
	CHECK(responder.BuildResponse(MakeFixture(), response).IsOk());

	Client::LoginServerClient client = ConnectedClient();
	std::size_t handled = 0;

	// Leave a partial entry buffered.
	CHECK(client.Feed(response.data(), GameServerList::kEntrySize - 4, handled).IsOk());
	client.Reset();

	CHECK(client.Phase() == Client::LoginServerPhase::Disconnected);
	CHECK_EQ(client.Servers().Count(), std::size_t{ 0 });

	// Reconnect and receive cleanly.
	EndpointAddress endpoint;
	endpoint.ip = kDefaultLoginAddress;
	endpoint.port = kDefaultLoginPort;
	CHECK(client.BeginConnect(endpoint).IsOk());
	CHECK(client.CompleteConnect().IsOk());
	std::vector<WireU8> request;
	CHECK(client.RequestGameServers(request).IsOk());
	CHECK(client.Feed(response, handled).IsOk());

	CHECK(client.IsComplete());
	CHECK_EQ(client.Servers().Count(), std::size_t{ 3 });
}
