// LOGIN-001: the Login Server game-server-list codec.
//
// Deterministic: no socket, no clock, no RNG, no database.
//
// The facts under test are the ones a wrong implementation would get wrong:
//
//   1. The ids are 1542 / 1552 / 1562, and they are LOGIN-base ids, not LOBBY-base.
//   2. REQ_GAME_SVR and SND_GAME_SVR_END are bare 8-byte NET_MSG_GENERIC frames
//      with NO body - in particular the terminator carries no count.
//   3. SND_GAME_SVR is exactly 56 bytes, and the PADDING at 29-31 and 53-55 is part
//      of the wire. A codec that packs fields tightly produces a different frame.
//   4. All five integers are SIGNED on the wire.
//   5. The list is a sparse (group, channel) GRID, not a sequence: out-of-range
//      entries are rejected, duplicates collapse, and order is grid order.

#include "TestHarness.h"

#include "GameServerListProtocol.h"
#include "NetworkCodec.h"
#include "NetworkConnection.h"
#include "NetworkTypes.h"
#include "NetCompressCodec.h"

namespace
{
	using namespace Modern;
	using namespace Modern::Network;

	GameServerInfo Sample()
	{
		GameServerInfo info;
		info.ip = "10.20.30.40";
		info.servicePort = 5101;
		info.serverGroup = 3;
		info.serverNumber = 7;
		info.currentClients = 120;
		info.maxClients = 800;
		info.pk = true;
		return info;
	}

	// Patches a little-endian u32 at a byte offset, so a test can corrupt exactly
	// one field without rebuilding the frame.
	void PutU32At(std::vector<WireU8>& bytes, std::size_t offset, WireU32 value)
	{
		for (int i = 0; i < 4; ++i)
		{
			bytes[offset + static_cast<std::size_t>(i)] =
			    static_cast<WireU8>((value >> (8 * i)) & 0xFFu);
		}
	}

	// The frame as the wire carries it, header included.
	std::vector<WireU8> BuildFrame(const GameServerInfo& info)
	{
		std::vector<WireU8> frame;
		(void)GameServerListCodec::AppendEntry(frame, info);
		return frame;
	}
}

// ---------------------------------------------------------------------------
// Identity
// ---------------------------------------------------------------------------

MODERN_TEST(GameServerList_MessageIdsAreTheLoginBaseOnes)
{
	CHECK_EQ(GameServerList::kRequestGameServersId, MessageId{ 1542 });
	CHECK_EQ(GameServerList::kGameServerInfoId, MessageId{ 1552 });
	CHECK_EQ(GameServerList::kGameServerListEndId, MessageId{ 1562 });

	// They are NET_MSG_LGIN-relative, and are NOT the Agent login ids. Conflating
	// the two conversations is the mistake this milestone exists to prevent.
	CHECK_NE(GameServerList::kRequestGameServersId, MessageId{ 2049 });
	CHECK_NE(GameServerList::kGameServerInfoId, MessageId{ 2050 });

	CHECK(GameServerListCodec::IsRequest(1542));
	CHECK(GameServerListCodec::IsEntry(1552));
	CHECK(GameServerListCodec::IsListEnd(1562));
	CHECK(!GameServerListCodec::IsEntry(1542));
	CHECK(!GameServerListCodec::IsListEnd(1552));
}

MODERN_TEST(GameServerList_GridLimitsComeFromLegacyConstants)
{
	CHECK_EQ(GameServerList::kMaxServerGroup, std::int32_t{ 20 });
	CHECK_EQ(GameServerList::kMaxChannelNumber, std::int32_t{ 10 });
	// MAX_SERVER_GROUP * MAX_CHANNEL_NUMBER - the most entries the grid can hold,
	// and therefore the most the server can ever send.
	CHECK_EQ(GameServerList::kMaxServers, std::size_t{ 200 });
}

// ---------------------------------------------------------------------------
// Request
// ---------------------------------------------------------------------------

MODERN_TEST(GameServerList_RequestIsABareEightByteMessage)
{
	std::vector<WireU8> request;
	CHECK(GameServerListCodec::AppendRequest(request).IsOk());

	// Exactly the header. SndReqServerInfo declares a NET_MSG_GENERIC, sets
	// dwSize = sizeof(NET_MSG_GENERIC) and sends it (s_NetClientMsg.cpp:328-333).
	CHECK_EQ(request.size(), std::size_t{ 8 });
	CHECK_EQ(request.size(), GameServerList::kBareMessageSize);
	CHECK_EQ(Codec::ReadU32(request.data()), WireU32{ 8 });
	CHECK_EQ(Codec::ReadU32(request.data() + 4), GameServerList::kRequestGameServersId);

	MessageId id = 0;
	CHECK(GameServerListCodec::DecodeBare(request, id).IsOk());
	CHECK(GameServerListCodec::IsRequest(id));
	CHECK(GameServerListCodec::ValidateRequest(request).IsOk());
}

MODERN_TEST(GameServerList_RequestIsNotCompressed)
{
	// The Login Server connection is raw. If the request were enveloped, its nType
	// would be NET_MSG_COMPRESS (170) rather than 1542.
	std::vector<WireU8> request;
	CHECK(GameServerListCodec::AppendRequest(request).IsOk());
	CHECK_NE(Codec::ReadU32(request.data() + 4), WireU32{ 170 });
	CHECK(!NetCompress::IsEnvelope(Codec::ReadU32(request.data() + 4)));
}

// ---------------------------------------------------------------------------
// Terminator
// ---------------------------------------------------------------------------

MODERN_TEST(GameServerList_TerminatorIsBareAndCarriesNoCount)
{
	std::vector<WireU8> end;
	CHECK(GameServerListCodec::AppendListEnd(end).IsOk());

	CHECK_EQ(end.size(), std::size_t{ 8 });
	CHECK_EQ(Codec::ReadU32(end.data()), WireU32{ 8 });
	CHECK_EQ(Codec::ReadU32(end.data() + 4), GameServerList::kGameServerListEndId);

	// The two remaining bytes groups are the header, so there is provably no room
	// for a count: the client must be told "done" by the id alone.
	MessageId id = 0;
	CHECK(GameServerListCodec::DecodeBare(end, id).IsOk());
	CHECK(GameServerListCodec::IsListEnd(id));
}

// ---------------------------------------------------------------------------
// Entry geometry
// ---------------------------------------------------------------------------

MODERN_TEST(GameServerList_EntryIsFiftySixBytesWithLegacyPadding)
{
	const std::vector<WireU8> frame = BuildFrame(Sample());

	CHECK_EQ(frame.size(), std::size_t{ 56 });
	CHECK_EQ(frame.size(), GameServerList::kEntrySize);
	CHECK_EQ(Codec::ReadU32(frame.data()), WireU32{ 56 });
	CHECK_EQ(Codec::ReadU32(frame.data() + 4), GameServerList::kGameServerInfoId);

	// The IP field is char[21] and ends at byte 28. The length is derived, not
	// hardcoded, so editing the sample address cannot silently mis-assert.
	const std::string expectedIp = "10.20.30.40";
	const std::size_t  ipLen      = expectedIp.size();
	const std::string ip(reinterpret_cast<const char*>(frame.data() + GameServerList::kOffsetServerIp), ipLen);
	CHECK_EQ(ip, expectedIp);

	// Terminator immediately after the address, then zero to the end of the field.
	CHECK_EQ(frame[GameServerList::kOffsetServerIp + ipLen], WireU8{ 0 });
	for (std::size_t i = GameServerList::kOffsetServerIp + ipLen + 1;
	     i < GameServerList::kOffsetServerIp + GameServerList::kServerIpFieldSize; ++i)
	{
		CHECK_EQ(frame[i], WireU8{ 0 });
	}

	// Alignment padding at 29-31: szServerIP ends at 28, the next int needs 4-byte
	// alignment, so the next field starts at 32. A tightly packed codec would put
	// nServicePort at 29 and produce a frame no client accepts.
	for (std::size_t i = 29; i < 32; ++i)
	{
		CHECK_EQ(frame[i], WireU8{ 0 });
	}

	CHECK_EQ(Codec::ReadU32(frame.data() + 32), WireU32{ 5101 });
	CHECK_EQ(Codec::ReadU32(frame.data() + 36), WireU32{ 3 });
	CHECK_EQ(Codec::ReadU32(frame.data() + 40), WireU32{ 7 });
	CHECK_EQ(Codec::ReadU32(frame.data() + 44), WireU32{ 120 });
	CHECK_EQ(Codec::ReadU32(frame.data() + 48), WireU32{ 800 });
	CHECK_EQ(frame[52], WireU8{ 1 }); // bPK

	// Trailing alignment padding at 53-55.
	for (std::size_t i = 53; i < 56; ++i)
	{
		CHECK_EQ(frame[i], WireU8{ 0 });
	}
}

MODERN_TEST(GameServerList_EntryPaddingIsDeterministic)
{
	// Legacy leaves the padding indeterminate - ncil is an uninitialised stack
	// local. Zero is chosen deliberately so the frame is reproducible and
	// testable; asserted here so that choice cannot be quietly broken.
	const std::vector<WireU8> a = BuildFrame(Sample());
	const std::vector<WireU8> b = BuildFrame(Sample());
	CHECK(a == b);
}

// ---------------------------------------------------------------------------
// Field round-trips
// ---------------------------------------------------------------------------

MODERN_TEST(GameServerList_EntryRoundTripsEveryField)
{
	GameServerInfo decoded;
	CHECK(GameServerListCodec::DecodeEntry(BuildFrame(Sample()), decoded).IsOk());

	CHECK_EQ(decoded.ip, Sample().ip);
	CHECK_EQ(decoded.servicePort, Sample().servicePort);
	CHECK_EQ(decoded.serverGroup, Sample().serverGroup);
	CHECK_EQ(decoded.serverNumber, Sample().serverNumber);
	CHECK_EQ(decoded.currentClients, Sample().currentClients);
	CHECK_EQ(decoded.maxClients, Sample().maxClients);
	CHECK_EQ(decoded.pk, Sample().pk);
}

MODERN_TEST(GameServerList_AllFiveIntegersAreSigned)
{
	// Native `int` on the wire. Reading them unsigned would turn a negative load
	// figure into ~4 billion.
	GameServerInfo info;
	info.ip = "1.2.3.4";
	info.servicePort = -1;
	info.serverGroup = 0;
	info.serverNumber = 0;
	info.currentClients = -250;
	info.maxClients = -1;

	GameServerInfo decoded;
	CHECK(GameServerListCodec::DecodeEntry(BuildFrame(info), decoded).IsOk());

	CHECK_EQ(decoded.servicePort, WireI32{ -1 });
	CHECK_EQ(decoded.currentClients, WireI32{ -250 });
	CHECK_EQ(decoded.maxClients, WireI32{ -1 });
}

MODERN_TEST(GameServerList_PkFlagRoundTripsBothWays)
{
	GameServerInfo off = Sample();
	off.pk = false;
	GameServerInfo decoded;
	CHECK(GameServerListCodec::DecodeEntry(BuildFrame(off), decoded).IsOk());
	CHECK(!decoded.pk);

	GameServerInfo on = Sample();
	on.pk = true;
	CHECK(GameServerListCodec::DecodeEntry(BuildFrame(on), decoded).IsOk());
	CHECK(decoded.pk);
}

MODERN_TEST(GameServerList_DefaultPkIsTrue)
{
	// G_SERVER_CUR_INFO_LOGIN's constructor initialises bPK to true
	// (s_NetGlobal.h:577-587), and the server copies whole structs.
	CHECK(GameServerList::kDefaultPk);
	CHECK(Sample().pk);
}

MODERN_TEST(GameServerList_IpFieldIsZeroPaddedAndNulTerminated)
{
	GameServerInfo info = Sample();
	info.ip = "1.2.3.4"; // shorter than the field

	const std::vector<WireU8> frame = BuildFrame(info);

	// Byte 7 of the field is the terminator and everything after it is zero, so a
	// C-string reader on the far side stops in the right place.
	CHECK_EQ(frame[GameServerList::kOffsetServerIp + 7], WireU8{ 0 });
	for (std::size_t i = GameServerList::kOffsetServerIp + 8;
	     i < GameServerList::kOffsetServerIp + GameServerList::kServerIpFieldSize; ++i)
	{
		CHECK_EQ(frame[i], WireU8{ 0 });
	}
}

MODERN_TEST(GameServerList_OverlongIpIsRefusedNotTruncated)
{
	// char[21] holds 20 characters plus a terminator. Refusing is what legacy's
	// std::string assignment into the array does, and truncating would hand a
	// client a different address than the server meant.
	GameServerInfo info = Sample();
	info.ip = std::string(GameServerList::kServerIpFieldSize, 'a'); // 21, no room to terminate

	std::vector<WireU8> frame;
	CHECK(GameServerListCodec::AppendEntry(frame, info).IsError());
	CHECK(frame.empty());

	// One shorter is the longest that fits.
	info.ip = std::string(GameServerList::kServerIpFieldSize - 1, 'a'); // 20
	CHECK(GameServerListCodec::AppendEntry(frame, info).IsOk());
	CHECK_EQ(frame.size(), GameServerList::kEntrySize);
}

MODERN_TEST(GameServerList_AnUnterminatedIpFieldIsRejectedOnDecode)
{
	// Legacy reads this field with C-string functions and would run past the end.
	// The modern boundary refuses it instead.
	std::vector<WireU8> frame = BuildFrame(Sample());
	for (std::size_t i = GameServerList::kOffsetServerIp;
	     i < GameServerList::kOffsetServerIp + GameServerList::kServerIpFieldSize; ++i)
	{
		frame[i] = '9';
	}

	GameServerInfo decoded;
	CHECK(GameServerListCodec::DecodeEntry(frame, decoded).IsError());
}

// ---------------------------------------------------------------------------
// Malformed input
// ---------------------------------------------------------------------------

MODERN_TEST(GameServerList_DecodeEntryRejectsAWrongDeclaredSize)
{
	std::vector<WireU8> frame = BuildFrame(Sample());
	PutU32At(frame, GameServerList::kOffsetSize, 55);

	GameServerInfo decoded;
	CHECK(GameServerListCodec::DecodeEntry(frame, decoded).IsError());
}

MODERN_TEST(GameServerList_DecodeEntryRejectsAWrongMessageId)
{
	std::vector<WireU8> frame = BuildFrame(Sample());
	PutU32At(frame, GameServerList::kOffsetType, GameServerList::kGameServerListEndId);

	GameServerInfo decoded;
	CHECK(GameServerListCodec::DecodeEntry(frame, decoded).IsError());
}

MODERN_TEST(GameServerList_DecodeEntryRejectsWrongLengthFrames)
{
	const std::vector<WireU8> full = BuildFrame(Sample());

	GameServerInfo decoded;

	std::vector<WireU8> shortFrame(full.begin(), full.end() - 1);
	CHECK(GameServerListCodec::DecodeEntry(shortFrame, decoded).IsError());

	std::vector<WireU8> longFrame = full;
	longFrame.push_back(0);
	CHECK(GameServerListCodec::DecodeEntry(longFrame, decoded).IsError());

	CHECK(GameServerListCodec::DecodeEntry({}, decoded).IsError());
}

MODERN_TEST(GameServerList_DecodeEntryRejectsAnImpossiblePkByte)
{
	// MSVC's bool can only be 0 or 1. Any other byte is not a value this protocol
	// can express, so it is refused rather than coerced to true.
	std::vector<WireU8> frame = BuildFrame(Sample());
	frame[GameServerList::kOffsetPk] = 2;

	GameServerInfo decoded;
	CHECK(GameServerListCodec::DecodeEntry(frame, decoded).IsError());
}

MODERN_TEST(GameServerList_DecodeBareRejectsExtraBytes)
{
	// A longer frame with the terminator id is not something legacy could have
	// sent, so accepting it would let a peer invent a body with no meaning.
	std::vector<WireU8> end;
	CHECK(GameServerListCodec::AppendListEnd(end).IsOk());
	end.push_back(0);

	MessageId id = 0;
	CHECK(GameServerListCodec::DecodeBare(end, id).IsError());
}

MODERN_TEST(GameServerList_DecodeBareRejectsAnUnknownId)
{
	std::vector<WireU8> frame(8, 0);
	PutU32At(frame, GameServerList::kOffsetSize, 8);
	PutU32At(frame, GameServerList::kOffsetType, 999999);

	MessageId id = 0;
	CHECK(GameServerListCodec::DecodeBare(frame, id).IsError());
}

MODERN_TEST(GameServerList_ValidateRequestRejectsAnythingButTheRequest)
{
	// A bare terminator is a well-formed message but not a request.
	std::vector<WireU8> end;
	CHECK(GameServerListCodec::AppendListEnd(end).IsOk());
	CHECK(GameServerListCodec::ValidateRequest(end).IsError());

	// Trailing bytes on a request are refused, not ignored. Legacy answers any
	// frame carrying this id regardless of dwSize (s_CLoginServerMsg.cpp:36-38);
	// that check is modern hardening.
	std::vector<WireU8> request;
	CHECK(GameServerListCodec::AppendRequest(request).IsOk());
	request.push_back(0);
	CHECK(GameServerListCodec::ValidateRequest(request).IsError());
}

MODERN_TEST(GameServerList_AppendEntryRefusesAnOutOfRangeEntryBeforeWriting)
{
	// Legacy's server can only ever emit in-range entries, because it walks the
	// grid. The check keeps a hand-built fixture from producing an entry the
	// client would silently drop.
	GameServerInfo info = Sample();
	info.serverGroup = GameServerList::kMaxServerGroup; // 20, one past the end

	std::vector<WireU8> frame;
	CHECK(GameServerListCodec::AppendEntry(frame, info).IsError());
	CHECK(frame.empty());
}

// ---------------------------------------------------------------------------
// The grid
// ---------------------------------------------------------------------------

MODERN_TEST(GameServerList_GridRejectsOutOfRangeIndices)
{
	GameServerGrid grid;

	GameServerInfo good = Sample();

	CHECK(grid.Add(good).IsOk());

	GameServerInfo highGroup = good;
	highGroup.serverGroup = GameServerList::kMaxServerGroup;
	CHECK(grid.Add(highGroup).IsError());

	GameServerInfo highNumber = good;
	highNumber.serverNumber = GameServerList::kMaxChannelNumber;
	CHECK(grid.Add(highNumber).IsError());

	GameServerInfo negative = good;
	negative.serverGroup = -1;
	CHECK(grid.Add(negative).IsError());

	// Only the one valid entry survived.
	CHECK_EQ(grid.Count(), std::size_t{ 1 });
}

MODERN_TEST(GameServerList_GridAcceptsTheExtremeIndices)
{
	GameServerGrid grid;

	GameServerInfo last = Sample();
	last.serverGroup = GameServerList::kMaxServerGroup - 1;    // 19
	last.serverNumber = GameServerList::kMaxChannelNumber - 1; // 9
	CHECK(grid.Add(last).IsOk());
	CHECK(grid.Contains(19, 9));
	CHECK_EQ(grid.Count(), std::size_t{ 1 });
}

MODERN_TEST(GameServerList_GridIsFullAtTwoHundredEntries)
{
	GameServerGrid grid;
	for (std::int32_t g = 0; g < GameServerList::kMaxServerGroup; ++g)
	{
		for (std::int32_t n = 0; n < GameServerList::kMaxChannelNumber; ++n)
		{
			GameServerInfo info;
			info.ip = "10.0.0.1";
			info.serverGroup = g;
			info.serverNumber = n;
			info.maxClients = 100;
			CHECK(grid.Add(info).IsOk());
		}
	}

	CHECK_EQ(grid.Count(), GameServerList::kMaxServers);
	CHECK_EQ(grid.Count(), std::size_t{ 200 });
	CHECK_EQ(grid.Servers().size(), std::size_t{ 200 });

	// One more is impossible: the grid is full, not merely full by convention.
	GameServerInfo overflow;
	overflow.serverGroup = GameServerList::kMaxServerGroup;
	CHECK(grid.Add(overflow).IsError());
}

MODERN_TEST(GameServerList_DuplicateCoordinatesCollapseAndTheLastWins)
{
	// m_sGame[group][number] = gscil overwrites. A duplicate is not two servers.
	GameServerGrid grid;

	GameServerInfo first = Sample();
	first.maxClients = 100;
	CHECK(grid.Add(first).IsOk());

	GameServerInfo second = first;
	second.maxClients = 250;
	CHECK(grid.Add(second).IsOk());

	CHECK_EQ(grid.Count(), std::size_t{ 1 });
	CHECK_EQ(grid.Servers().size(), std::size_t{ 1 });
	CHECK_EQ(grid.Servers()[0].maxClients, WireI32{ 250 });
}

MODERN_TEST(GameServerList_OrderIsGroupThenChannelNotInsertionOrder)
{
	// The server walks nested loops, group outer and channel inner
	// (s_CLoginServerMsg.cpp:121-134), so the client must observe that same order
	// however the entries arrived.
	GameServerGrid grid;

	const std::pair<std::int32_t, std::int32_t> insertion[] = {
	    { 2, 3 }, { 0, 5 }, { 2, 1 }, { 1, 0 }, { 0, 1 },
	};

	for (const auto& cell : insertion)
	{
		GameServerInfo info;
		info.ip = "10.0.0.1";
		info.serverGroup = cell.first;
		info.serverNumber = cell.second;
		info.maxClients = 10;
		CHECK(grid.Add(info).IsOk());
	}

	const std::vector<GameServerInfo> ordered = grid.Servers();
	CHECK_EQ(ordered.size(), std::size_t{ 5 });

	// Expected canonical order: (0,1) (0,5) (1,0) (2,1) (2,3)
	CHECK_EQ(ordered[0].serverGroup, WireI32{ 0 });
	CHECK_EQ(ordered[0].serverNumber, WireI32{ 1 });
	CHECK_EQ(ordered[1].serverGroup, WireI32{ 0 });
	CHECK_EQ(ordered[1].serverNumber, WireI32{ 5 });
	CHECK_EQ(ordered[2].serverGroup, WireI32{ 1 });
	CHECK_EQ(ordered[2].serverNumber, WireI32{ 0 });
	CHECK_EQ(ordered[3].serverGroup, WireI32{ 2 });
	CHECK_EQ(ordered[3].serverNumber, WireI32{ 1 });
	CHECK_EQ(ordered[4].serverGroup, WireI32{ 2 });
	CHECK_EQ(ordered[4].serverNumber, WireI32{ 3 });
}

MODERN_TEST(GameServerList_ClearEmptiesTheGrid)
{
	GameServerGrid grid;
	CHECK(grid.Add(Sample()).IsOk());
	CHECK_EQ(grid.Count(), std::size_t{ 1 });

	grid.Clear();

	CHECK_EQ(grid.Count(), std::size_t{ 0 });
	CHECK(grid.Servers().empty());
	CHECK(!grid.Contains(3, 7));
	CHECK(grid.Find(3, 7) == nullptr);
}

MODERN_TEST(GameServerList_AdvertisableOnlyWhenMaxClientsIsPositive)
{
	// CLoginServer::MsgSndGameSvrInfo emits an entry only when
	// nServerMaxClient > 0 (s_CLoginServerMsg.cpp:125).
	GameServerInfo info = Sample();

	info.maxClients = 1;
	CHECK(GameServerListCodec::IsAdvertisable(info));

	info.maxClients = 0;
	CHECK(!GameServerListCodec::IsAdvertisable(info));

	info.maxClients = -5;
	CHECK(!GameServerListCodec::IsAdvertisable(info));
}

// ---------------------------------------------------------------------------
// Endpoint
// ---------------------------------------------------------------------------

MODERN_TEST(GameServerList_OnlyNumericDottedQuadAddressesAreAccepted)
{
	// legacy calls ::inet_addr directly (s_NetClient.cpp:474); the gethostbyname
	// branch above it is commented out (lines 436-465). A hostname would not
	// resolve - inet_addr would return INADDR_NONE and the connect would fail
	// silently.
	CHECK(EndpointAddress::IsNumericIPv4("211.172.252.50"));
	CHECK(EndpointAddress::IsNumericIPv4("0.0.0.0"));
	CHECK(EndpointAddress::IsNumericIPv4("255.255.255.255"));

	CHECK(!EndpointAddress::IsNumericIPv4("localhost"));
	CHECK(!EndpointAddress::IsNumericIPv4("example.com"));
	CHECK(!EndpointAddress::IsNumericIPv4("211.172.252"));
	CHECK(!EndpointAddress::IsNumericIPv4("211.172.252.50.7"));
	CHECK(!EndpointAddress::IsNumericIPv4("256.1.1.1"));
	CHECK(!EndpointAddress::IsNumericIPv4("211.172.252."));
	CHECK(!EndpointAddress::IsNumericIPv4(".1.1.1"));
	CHECK(!EndpointAddress::IsNumericIPv4(""));
}

// ---------------------------------------------------------------------------
// Streaming
// ---------------------------------------------------------------------------

MODERN_TEST(GameServerList_EntriesAndTerminatorConcatenateIntoOneRawStream)
{
	// Raw, not enveloped. The Login Server sends each message with SendClient2,
	// which never batches (s_CClientManager.cpp:489-527), so the stream is a plain
	// sequence of NET_MSG_GENERIC frames that ConnectionFramer reads directly.
	std::vector<WireU8> stream;
	CHECK(GameServerListCodec::AppendEntry(stream, Sample()).IsOk());
	CHECK(GameServerListCodec::AppendEntry(stream, Sample()).IsOk());
	CHECK(GameServerListCodec::AppendListEnd(stream).IsOk());

	CHECK_EQ(stream.size(), GameServerList::kEntrySize * 2 + GameServerList::kBareMessageSize);

	// The first frame's type is SND_GAME_SVR, not NET_MSG_COMPRESS.
	CHECK_EQ(Codec::ReadU32(stream.data() + 4), GameServerList::kGameServerInfoId);

	ConnectionFramer framer;
	CHECK(framer.Feed(stream.data(), stream.size()) == FrameStatus::Ok);

	Message message;
	int entries = 0;
	int ends    = 0;
	while (framer.Next(message) == FrameStatus::Ok)
	{
		if (message.header.type == GameServerList::kGameServerInfoId)
		{
			++entries;
		}
		else if (message.header.type == GameServerList::kGameServerListEndId)
		{
			++ends;
		}
	}
	CHECK_EQ(entries, 2);
	CHECK_EQ(ends, 1);
	CHECK(!framer.IsFailed());
}
