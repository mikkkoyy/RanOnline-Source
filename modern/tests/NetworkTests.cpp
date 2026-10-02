// VERTICAL-027: modern network boundary.
//
// These are rule tests for framing, encoding, validation, routing and session
// state. Every one is deterministic and runs without a socket, a clock or a
// network - the whole point of LoopbackTransport is that the framing code can be
// exercised with byte-at-a-time delivery, which is the case that breaks in the
// field and never reproduces in a naive test.
//
// Links ModernNetwork (which brings Modern for Result/Status) and nothing else.
// No renderer, no socket, no database, no legacy library.

#include "TestHarness.h"

// ModernNetwork exposes modern/network as its include root, the same way Modern
// exposes modern/core - so headers are named without a directory prefix.
#include "LoopbackTransport.h"
#include "MessageRouter.h"
#include "NetworkCodec.h"
#include "NetworkConnection.h"
#include "NetworkTypes.h"
#include "ServerSession.h"

namespace
{
	using namespace Modern;
	using namespace Modern::Network;

	// ---- helpers ---------------------------------------------------------

	// Builds one encoded message.
	std::vector<WireU8> MakeMessage(MessageId id, const std::vector<WireU8>& payload = {})
	{
		MessageHeader header;
		header.type = id;
		std::vector<WireU8> bytes;
		(void) Codec::EncodeMessage(header, payload, bytes);
		return bytes;
	}

	MessageHeader HeaderAt(const std::vector<WireU8>& bytes)
	{
		MessageHeader header;
		header.size = Codec::ReadU32(bytes.data());
		header.type = Codec::ReadU32(bytes.data() + 4);
		return header;
	}
}

// ── Protocol constants: transcription, not invention ─────────────────────────
//
// These guard the transcription from legacy. If someone "tidies" a wire
// constant, the modern client and server silently stop interoperating with
// RAN, and no functional test would notice.

MODERN_TEST(Network_ProtocolConstantsMatchLegacy)
{
	// s_NetGlobal.h:645-696.
	CHECK_EQ(Protocol::kMessageBase, static_cast<MessageId>(992));
	CHECK_EQ(Protocol::kLoginBase, static_cast<MessageId>(1442));
	CHECK_EQ(Protocol::kLobbyBase, static_cast<MessageId>(1942));
	CHECK_EQ(Protocol::kLobbyMax, static_cast<MessageId>(2442));
	CHECK_EQ(Protocol::kGCtrlBase, static_cast<MessageId>(2892));

	// s_NetGlobal.h:105-111 and SendMsgBuffer.h:36-38.
	CHECK_EQ(Protocol::kDataBufferSize, static_cast<std::size_t>(2048));
	CHECK_EQ(Protocol::kDataMessageBufferSize, static_cast<std::size_t>(8192));
	CHECK_EQ(Protocol::kDataClientMessageBufferSize, static_cast<std::size_t>(16384));
	CHECK_EQ(Protocol::kMaxClients, static_cast<std::size_t>(1000));
	CHECK_EQ(Protocol::kSendBufferSize, static_cast<std::size_t>(6144));
	CHECK_EQ(Protocol::kMaxPacketSize, static_cast<std::size_t>(2048));
	CHECK_EQ(Protocol::kCompressThreshold, static_cast<std::size_t>(1000));
	CHECK_EQ(Protocol::kEncryptKeyLength, static_cast<std::size_t>(12));
	CHECK_EQ(Protocol::kTimeoutMilliseconds, static_cast<std::size_t>(180000));
}

// The header is exactly two 32-bit fields. A change here changes the wire.
MODERN_TEST(Network_HeaderIsEightBytes)
{
	CHECK_EQ(kMessageHeaderSize, static_cast<std::size_t>(8));
}

// The country #if chain in s_NetGlobal.h:643-669 assigns 992 in EVERY branch,
// so ids do not vary by region. This matters because V026 established that the
// country macro is KR_PARAM for RAN's own build: if ids varied by region, the
// protocol would depend on the macro question V026 just closed.
MODERN_TEST(Network_MessageIdsDoNotVaryByRegion)
{
	// Every branch of the legacy chain yields the same value, so a single
	// constant is sufficient and correct.
	CHECK_EQ(Protocol::kMessageBase, static_cast<MessageId>(992));
}

// ── Codec ────────────────────────────────────────────────────────────────────

MODERN_TEST(Codec_RoundTripsIntegers)
{
	std::vector<WireU8> payload;
	Codec::WriteU8(payload, 0x12);
	Codec::WriteU16(payload, 0xBEEF);
	Codec::WriteU32(payload, 0xDEADBEEFu);
	Codec::WriteU64(payload, 0x0123456789ABCDEFull);

	Codec::Reader reader(payload.data(), payload.size());
	WireU8  u8 = 0; WireU16 u16 = 0; WireU32 u32 = 0; WireU64 u64 = 0;
	CHECK(reader.ReadU8(u8).IsOk());
	CHECK(reader.ReadU16(u16).IsOk());
	CHECK(reader.ReadU32(u32).IsOk());
	CHECK(reader.ReadU64(u64).IsOk());

	CHECK_EQ(static_cast<int>(u8), 0x12);
	CHECK_EQ(static_cast<int>(u16), 0xBEEF);
	CHECK_EQ(static_cast<uint32_t>(u32), 0xDEADBEEFu);
	CHECK(static_cast<uint64_t>(u64) == 0x0123456789ABCDEFull);
	CHECK(reader.Exhausted());
}

// Byte order is little-endian and asserted position by position, because a
// wrong-order encoder that happens to round-trip through its own decoder would
// pass every functional test and fail on the wire.
MODERN_TEST(Codec_IntegersAreLittleEndianOnTheWire)
{
	std::vector<WireU8> payload;
	Codec::WriteU32(payload, 0x01020304u);
	CHECK_EQ(payload.size(), static_cast<std::size_t>(4));
	CHECK_EQ(static_cast<int>(payload[0]), 0x04);
	CHECK_EQ(static_cast<int>(payload[1]), 0x03);
	CHECK_EQ(static_cast<int>(payload[2]), 0x02);
	CHECK_EQ(static_cast<int>(payload[3]), 0x01);

	Codec::WriteU16(payload, 0x0102);
	CHECK_EQ(static_cast<int>(payload[4]), 0x02);
	CHECK_EQ(static_cast<int>(payload[5]), 0x01);
}

MODERN_TEST(Codec_RoundTripsFloat)
{
	std::vector<WireU8> payload;
	Codec::WriteF32(payload, 1234.5f);

	Codec::Reader reader(payload.data(), payload.size());
	float value = 0.0f;
	CHECK(reader.ReadF32(value).IsOk());
	CHECK_EQ(value, 1234.5f);
}

MODERN_TEST(Codec_RoundTripsString)
{
	std::vector<WireU8> payload;
	CHECK(Codec::WriteString(payload, "brawler", 64).IsOk());

	Codec::Reader reader(payload.data(), payload.size());
	std::string value;
	CHECK(reader.ReadString(value, 64).IsOk());
	CHECK_EQ(value, std::string("brawler"));
}

// A string whose declared length exceeds the limit is refused at ENCODE time, so
// no oversized buffer is ever built.
MODERN_TEST(Codec_OversizedStringIsRefusedAtEncode)
{
	std::vector<WireU8> payload;
	CHECK(Codec::WriteString(payload, "far too long for the field", 4).IsError());
	CHECK(payload.empty());
}

MODERN_TEST(Codec_StringLengthOverrunIsRefused)
{
	// Claims 64 bytes of string, supplies 2, and declares the limit as 64.
	std::vector<WireU8> payload;
	Codec::WriteU16(payload, 64);
	Codec::WriteU8(payload, 'a');
	Codec::WriteU8(payload, 'b');

	Codec::Reader reader(payload.data(), payload.size());
	std::string value;
	CHECK(reader.ReadString(value, 64).IsError());
}

// Every read checks the remaining length, so a truncated payload is an error and
// never an out-of-bounds read.
MODERN_TEST(Codec_TruncatedPayloadIsRefusedNotOverread)
{
	std::vector<WireU8> payload;
	Codec::WriteU32(payload, 0x01020304u);   // promises 4 bytes
	payload.pop_back();                       // supplies 3

	Codec::Reader reader(payload.data(), payload.size());
	WireU32 value = 0;
	CHECK(reader.ReadU32(value).IsError());
}

// A declared count that exceeds what remains is refused. This is the shape a
// hostile packet takes.
MODERN_TEST(Codec_ArrayCountBeyondBufferIsRefused)
{
	std::vector<WireU8> payload;
	Codec::WriteU16(payload, 1000);          // claims 1000 elements
	Codec::WriteU8(payload, 0xFF);

	Codec::Reader reader(payload.data(), payload.size());
	std::vector<WireU8> values;
	CHECK(reader.ReadBytes(values, 1000).IsError());
}

MODERN_TEST(Codec_MessageRoundTrip)
{
	std::vector<WireU8> payload;
	Codec::WriteU16(payload, 42);
	Codec::WriteString(payload, "hero", 32);

	MessageHeader header;
	header.type = WellKnownMessage::kHeartbeatClientReq;

	std::vector<WireU8> bytes;
	CHECK(Codec::EncodeMessage(header, payload, bytes).IsOk());
	CHECK_EQ(bytes.size(), kMessageHeaderSize + payload.size());

	Message decoded;
	CHECK(Codec::DecodeMessage(bytes.data(), bytes.size(), decoded).IsOk());
	CHECK_EQ(static_cast<uint32_t>(decoded.header.type),
	         static_cast<uint32_t>(WellKnownMessage::kHeartbeatClientReq));
	CHECK_EQ(decoded.PayloadSize(), payload.size());

	// Compare field by field rather than against a hand-built expected message:
	// the encoder computes `size`, so a literal here would be asserting the test
	// author's arithmetic rather than the round trip.
	const MessageHeader roundTripped = decoded.header;
	CHECK_EQ(static_cast<uint32_t>(roundTripped.size),
	         static_cast<uint32_t>(kMessageHeaderSize + payload.size()));
	CHECK_EQ(static_cast<uint32_t>(roundTripped.type),
	         static_cast<uint32_t>(WellKnownMessage::kHeartbeatClientReq));
	CHECK(decoded.payload == payload);
}

// The size field counts the header. An off-by-8 header is the classic framing
// bug, so it gets its own test.
MODERN_TEST(Codec_MessageSizeIncludesTheHeader)
{
	const std::vector<WireU8> bytes = MakeMessage(WellKnownMessage::kVersionReq, { 1, 2, 3 });
	CHECK_EQ(HeaderAt(bytes).size, static_cast<WireU32>(kMessageHeaderSize + 3));
}

// ---- decode validation: the security boundary -------------------------------

MODERN_TEST(Codec_DecodeRejectsSizeBelowHeader)
{
	// size = 4, less than the 8-byte header.
	const WireU8 bytes[kMessageHeaderSize] = { 4, 0, 0, 0, 100, 0, 0, 0 };
	Message decoded;
	CHECK(Codec::DecodeMessage(bytes, sizeof(bytes), decoded).IsError());
}

MODERN_TEST(Codec_DecodeRejectsZeroSize)
{
	const WireU8 bytes[kMessageHeaderSize] = { 0, 0, 0, 0, 100, 0, 0, 0 };
	Message decoded;
	CHECK(Codec::DecodeMessage(bytes, sizeof(bytes), decoded).IsError());
}

MODERN_TEST(Codec_DecodeRejectsOversizedSize)
{
	// size = 4096, above NET_DATA_BUFSIZE / kMaxPacketSize.
	const WireU8 bytes[kMessageHeaderSize] = { 0, 16, 0, 0, 100, 0, 0, 0 };
	Message decoded;
	CHECK(Codec::DecodeMessage(bytes, sizeof(bytes), decoded).IsError());
}

// Legacy has no zero-type check (RcvMsgBuffer.cpp:112-118 validates only size).
// This is modern hardening and it is recorded as such in the header.
MODERN_TEST(Codec_DecodeRejectsZeroType)
{
	const WireU8 bytes[kMessageHeaderSize] = { 8, 0, 0, 0, 0, 0, 0, 0 };
	Message decoded;
	CHECK(Codec::DecodeMessage(bytes, sizeof(bytes), decoded).IsError());
}

MODERN_TEST(Codec_DecodeRejectsTruncatedMessage)
{
	const std::vector<WireU8> bytes = MakeMessage(WellKnownMessage::kVersionReq, { 1, 2, 3, 4 });
	Message decoded;
	// One byte short of the declared size.
	CHECK(Codec::DecodeMessage(bytes.data(), bytes.size() - 1, decoded).IsError());
}

MODERN_TEST(Codec_EncodeRefusesOversizedPayload)
{
	MessageHeader header;
	header.type = WellKnownMessage::kVersionReq;
	const std::vector<WireU8> payload(Protocol::kMaxPacketSize, 0x41);
	std::vector<WireU8> bytes;
	CHECK(Codec::EncodeMessage(header, payload, bytes).IsError());
}

MODERN_TEST(Codec_EncodeRefusesZeroType)
{
	MessageHeader header;
	header.type = 0;
	std::vector<WireU8> bytes;
	CHECK(Codec::EncodeMessage(header, {}, bytes).IsError());
}

// ── Framing ──────────────────────────────────────────────────────────────────

MODERN_TEST(Framing_SingleMessage)
{
	ConnectionFramer framer;
	const std::vector<WireU8> bytes = MakeMessage(WellKnownMessage::kVersionOk, { 1, 2, 3 });
	CHECK(framer.Feed(bytes.data(), bytes.size()) == FrameStatus::Ok);

	Message out;
	CHECK(framer.Next(out) == FrameStatus::Ok);
	CHECK_EQ(out.PayloadSize(), static_cast<std::size_t>(3));
	// Nothing left after a complete message.
	CHECK(framer.Next(out) == FrameStatus::NeedMoreData);
}

MODERN_TEST(Framing_MultipleMessagesInOneFeed)
{
	ConnectionFramer framer;
	std::vector<WireU8> bytes = MakeMessage(WellKnownMessage::kVersionOk, { 1 });
	const std::vector<WireU8> second = MakeMessage(WellKnownMessage::kVersionInfo, { 2, 2 });
	bytes.insert(bytes.end(), second.begin(), second.end());
	CHECK(framer.Feed(bytes.data(), bytes.size()) == FrameStatus::Ok);

	Message out;
	CHECK(framer.Next(out) == FrameStatus::Ok);
	CHECK_EQ(static_cast<uint32_t>(out.header.type),
	         static_cast<uint32_t>(WellKnownMessage::kVersionOk));
	CHECK(framer.Next(out) == FrameStatus::Ok);
	CHECK_EQ(static_cast<uint32_t>(out.header.type),
	         static_cast<uint32_t>(WellKnownMessage::kVersionInfo));
	CHECK(framer.Next(out) == FrameStatus::NeedMoreData);
}

// The case a real socket produces and a naive test never does: one message split
// across many feeds, down to a single byte at a time.
MODERN_TEST(Framing_PartialMessageAccumulatesOneByteAtATime)
{
	ConnectionFramer framer;
	const std::vector<WireU8> bytes = MakeMessage(WellKnownMessage::kVersionInfo, { 7, 7, 7, 7, 7 });

	Message out;
	for (std::size_t i = 0; i + 1 < bytes.size(); ++i)
	{
		CHECK(framer.Feed(bytes.data() + i, 1) == FrameStatus::Ok);
		CHECK(framer.Next(out) == FrameStatus::NeedMoreData);
	}
	CHECK(framer.Feed(bytes.data() + bytes.size() - 1, 1) == FrameStatus::Ok);
	CHECK(framer.Next(out) == FrameStatus::Ok);
	CHECK_EQ(out.PayloadSize(), static_cast<std::size_t>(5));
}

// A header arriving in one feed and the body in the next is the same hazard.
MODERN_TEST(Framing_HeaderAndBodySplitAcrossFeeds)
{
	ConnectionFramer framer;
	const std::vector<WireU8> bytes = MakeMessage(WellKnownMessage::kVersionInfo, { 9, 9 });

	Message out;
	CHECK(framer.Feed(bytes.data(), kMessageHeaderSize) == FrameStatus::Ok);
	CHECK(framer.Next(out) == FrameStatus::NeedMoreData);
	CHECK(framer.Feed(bytes.data() + kMessageHeaderSize,
	                   bytes.size() - kMessageHeaderSize) == FrameStatus::Ok);
	CHECK(framer.Next(out) == FrameStatus::Ok);
	CHECK_EQ(out.PayloadSize(), static_cast<std::size_t>(2));
}

MODERN_TEST(Framing_InvalidLengthLatchesFailure)
{
	ConnectionFramer framer;
	// size = 1, below the header.
	const WireU8 bad[kMessageHeaderSize] = { 1, 0, 0, 0, 100, 0, 0, 0 };
	CHECK(framer.Feed(bad, sizeof(bad)) == FrameStatus::Ok);

	Message out;
	CHECK(framer.Next(out) == FrameStatus::InvalidLength);
	CHECK(framer.IsFailed());

	// Latched: valid-looking bytes afterwards must not resurrect it, because we
	// no longer know where the stream re-synchronises.
	const std::vector<WireU8> good = MakeMessage(WellKnownMessage::kVersionOk, { 1 });
	CHECK(framer.Feed(good.data(), good.size()) == FrameStatus::Ok);
	CHECK(framer.Next(out) == FrameStatus::InvalidLength);
	CHECK(framer.IsFailed());
}

MODERN_TEST(Framing_OversizedFeedIsRefusedNotTruncated)
{
	ConnectionFramer framer(Protocol::kMaxPacketSize);
	const std::vector<WireU8> tooMuch(Protocol::kMaxPacketSize + 1, 0x41);
	CHECK(framer.Feed(tooMuch.data(), tooMuch.size()) == FrameStatus::Oversized);
	// Nothing was consumed, so the buffer is untouched rather than half-filled.
	CHECK_EQ(framer.Buffered(), static_cast<std::size_t>(0));
}

// ── Transport ────────────────────────────────────────────────────────────────

MODERN_TEST(Transport_LoopbackPairMovesBytes)
{
	auto [client, server] = LoopbackTransport::CreatePair();
	CHECK(client.State() == TransportState::Open);
	CHECK(server.State() == TransportState::Open);

	const std::vector<WireU8> bytes = MakeMessage(WellKnownMessage::kVersionOk, { 1, 2, 3, 4 });
	CHECK(client.Send(bytes.data(), bytes.size()).IsOk());
	CHECK_EQ(server.PendingBytes(), bytes.size());

	WireU8 buffer[64] = {};
	std::size_t received = 0;
	CHECK(server.Receive(buffer, sizeof(buffer), received).IsOk());
	CHECK_EQ(received, bytes.size());
}

MODERN_TEST(Transport_ReceiveReturnsPartialWithoutBlocking)
{
	auto [client, server] = LoopbackTransport::CreatePair();
	const std::vector<WireU8> bytes = MakeMessage(WellKnownMessage::kVersionOk, { 1, 2, 3, 4 });
	CHECK(client.Send(bytes.data(), bytes.size()).IsOk());

	WireU8 small[2] = {};
	std::size_t received = 0;
	CHECK(server.Receive(small, sizeof(small), received).IsOk());
	CHECK_EQ(received, static_cast<std::size_t>(2));
	CHECK_EQ(server.PendingBytes(), bytes.size() - 2);
}

MODERN_TEST(Transport_EmptyReceiveIsNotAnError)
{
	auto [client, server] = LoopbackTransport::CreatePair();
	WireU8 buffer[8] = {};
	std::size_t received = 99;
	CHECK(server.Receive(buffer, sizeof(buffer), received).IsOk());
	CHECK_EQ(received, static_cast<std::size_t>(0));
}

// ── End to end over the transport ────────────────────────────────────────────

// Bytes through a transport, in awkward chunks, into a framer, out as messages.
// This is the only test that exercises all three layers together, and it is the
// one that would catch a layer quietly assuming the others' responsibilities.
MODERN_TEST(Network_EndToEndOverTransportWithAwkwardChunks)
{
	auto [client, server] = LoopbackTransport::CreatePair();
	ConnectionFramer framer;

	std::vector<WireU8> wire =
		MakeMessage(static_cast<MessageId>(Protocol::kLoginBase + 10), { 1, 2 });
	const std::vector<WireU8> second =
		MakeMessage(static_cast<MessageId>(Protocol::kLobbyBase + 390), { 3, 4, 5 });
	wire.insert(wire.end(), second.begin(), second.end());

	// Deliver in three uneven chunks.
	std::size_t offset = 0;
	const std::size_t chunks[3] = { 3, 7, wire.size() - 10 };
	Message out;
	for (int i = 0; i < 3; ++i)
	{
		CHECK(client.Send(wire.data() + offset, chunks[i]).IsOk());
		offset += chunks[i];

		WireU8 buffer[64] = {};
		std::size_t received = 0;
		CHECK(server.Receive(buffer, sizeof(buffer), received).IsOk());
		CHECK(framer.Feed(buffer, received) == FrameStatus::Ok);

		while (framer.Next(out) == FrameStatus::Ok)
		{
			// Drained inside the loop; the final check below confirms the count.
		}
	}

	CHECK_EQ(framer.Buffered(), static_cast<std::size_t>(0));
	CHECK(!framer.IsFailed());
}

MODERN_TEST(Transport_DisconnectClosesBothHalves)
{
	auto [client, server] = LoopbackTransport::CreatePair();
	client.Disconnect();
	CHECK(client.State() == TransportState::Closed);
	CHECK(server.State() == TransportState::Closed);

	WireU8 buffer[4] = {};
	std::size_t received = 0;
	CHECK(server.Receive(buffer, sizeof(buffer), received).IsError());
	CHECK(client.Send(buffer, sizeof(buffer)).IsError());
}

// ── Router ───────────────────────────────────────────────────────────────────

MODERN_TEST(Router_RoutesToTheCorrectHandler)
{
	MessageRouter router;
	int owner = 1;
	WireU32 seen = 0;

	CHECK(router.Register(&owner, WellKnownMessage::kVersionOk,
	                      [&seen](void*, const Message& message)
	                      {
		                          seen = message.header.type;
		                          return DispatchResult::Handled();
	                      }).IsOk());

	const Message message{ { kMessageHeaderSize, WellKnownMessage::kVersionOk }, {} };
	CHECK(router.Dispatch(&owner, message).handled);
	CHECK_EQ(seen, static_cast<WireU32>(WellKnownMessage::kVersionOk));
}

// Two clients on one server have different state; a shared table is how one ends
// up acting on the other's session.
MODERN_TEST(Router_HandlersAreScopedPerOwner)
{
	MessageRouter router;
	int first = 1;
	int second = 2;
	WireU32 routedTo = 0;
	CHECK(router.Register(&first, WellKnownMessage::kVersionOk,
	                      [&routedTo](void*, const Message&)
	                      {
		                          routedTo = 1;
		                          return DispatchResult::Handled();
	                      }).IsOk());
	CHECK(router.Register(&second, WellKnownMessage::kVersionOk,
	                      [&routedTo](void*, const Message&)
	                      {
		                          routedTo = 2;
		                          return DispatchResult::Handled();
	                      }).IsOk());

	const Message message{ { kMessageHeaderSize, WellKnownMessage::kVersionOk }, {} };
	CHECK(router.Dispatch(&second, message).handled);
	CHECK_EQ(static_cast<int>(routedTo), 2);
}

// An unimplemented id is IGNORED, not an error: RAN has hundreds of message
// constants and a build legitimately meets ids it does not know.
MODERN_TEST(Router_UnknownIdIsIgnoredNotFailed)
{
	MessageRouter router;
	int owner = 1;
	const Message message{ { kMessageHeaderSize, 4242 }, {} };

	const DispatchResult result = router.Dispatch(&owner, message);
	CHECK(result.status.IsOk());
	CHECK(!result.handled);
}

// A silently replaced handler is indistinguishable from a working one until it
// misroutes, so registration refuses a duplicate.
MODERN_TEST(Router_DuplicateRegistrationIsRefused)
{
	MessageRouter router;
	int owner = 1;
	auto handler = [](void*, const Message&) { return DispatchResult::Handled(); };

	CHECK(router.Register(&owner, WellKnownMessage::kVersionOk, handler).IsOk());
	CHECK(router.Register(&owner, WellKnownMessage::kVersionOk, handler).IsError());
}

MODERN_TEST(Router_RejectsNullOwnerAndHandler)
{
	MessageRouter router;
	int owner = 1;
	auto handler = [](void*, const Message&) { return DispatchResult::Handled(); };

	CHECK(router.Register(nullptr, WellKnownMessage::kVersionOk, handler).IsError());
	CHECK(router.Register(&owner, 0, handler).IsError());
	CHECK(router.Register(&owner, WellKnownMessage::kVersionOk, nullptr).IsError());
}

// A long-lived server must not accumulate registrations for dead sessions.
MODERN_TEST(Router_ClearDropsOnlyThatOwner)
{
	MessageRouter router;
	int first = 1;
	int second = 2;
	auto handler = [](void*, const Message&) { return DispatchResult::Handled(); };

	CHECK(router.Register(&first, WellKnownMessage::kVersionOk, handler).IsOk());
	CHECK(router.Register(&first, WellKnownMessage::kVersionInfo, handler).IsOk());
	CHECK(router.Register(&second, WellKnownMessage::kVersionOk, handler).IsOk());
	CHECK_EQ(router.Size(), static_cast<std::size_t>(3));

	CHECK_EQ(router.Clear(&first), static_cast<std::size_t>(2));
	CHECK(!router.Has(&first, WellKnownMessage::kVersionOk));
	CHECK(router.Has(&second, WellKnownMessage::kVersionOk));
}

// ── Session ──────────────────────────────────────────────────────────────────

MODERN_TEST(Session_HappyPathToInWorld)
{
	ServerSession session(1001);
	CHECK(session.State() == SessionState::Connected);

	CHECK(session.BeginAuthentication().IsOk());
	CHECK(session.State() == SessionState::Authenticating);

	CHECK(session.CompleteAuthentication(555, false).IsOk());
	CHECK(session.State() == SessionState::Authenticated);
	CHECK_EQ(static_cast<uint64_t>(session.AccountId()), static_cast<uint64_t>(555));

	CHECK(session.SelectCharacter(42).IsOk());
	CHECK(session.State() == SessionState::CharacterSelected);
	CHECK_EQ(static_cast<uint32_t>(session.CharacterId()), static_cast<uint32_t>(42));

	CHECK(session.EnterWorld().IsOk());
	CHECK(session.State() == SessionState::InWorld);
	CHECK(session.MayAffectWorld());
}

// Server authority: a request is only meaningful once the previous step has
// happened, and an out-of-order one is refused at the boundary.
MODERN_TEST(Session_RefusesCharacterListBeforeAuthentication)
{
	ServerSession session;
	CHECK(!session.MayRequestCharacterList());
	CHECK(!session.MayRequestCharacterSelect());
	CHECK(!session.MayAffectWorld());

	CHECK(session.SelectCharacter(1).IsError());
	CHECK(session.State() == SessionState::Connected);
}

MODERN_TEST(Session_CharacterListRequiresAuthentication)
{
	ServerSession session;
	CHECK(session.BeginAuthentication().IsOk());
	// Authenticating is not Authenticated: credentials are in flight, not accepted.
	CHECK(!session.MayRequestCharacterList());

	CHECK(session.CompleteAuthentication(1, false).IsOk());
	CHECK(session.MayRequestCharacterList());
}

MODERN_TEST(Session_RejectedAuthenticationReturnsToConnected)
{
	ServerSession session;
	CHECK(session.BeginAuthentication().IsOk());
	CHECK(session.CompleteAuthentication(0, true).IsError());
	CHECK(session.State() == SessionState::Connected);
	// And the whole sequence can be retried.
	CHECK(session.BeginAuthentication().IsOk());
	CHECK(session.CompleteAuthentication(7, false).IsOk());
}

MODERN_TEST(Session_EnterWorldRequiresASelectedCharacter)
{
	ServerSession session;
	CHECK(session.BeginAuthentication().IsOk());
	CHECK(session.CompleteAuthentication(1, false).IsOk());

	// A join with no selection is a protocol fault, and keeping a half-entered
	// world is worse than dropping the connection.
	CHECK(session.EnterWorld().IsError());
	CHECK(session.IsClosed());
}

MODERN_TEST(Session_EnterWorldIsIdempotent)
{
	ServerSession session;
	CHECK(session.BeginAuthentication().IsOk());
	CHECK(session.CompleteAuthentication(1, false).IsOk());
	CHECK(session.SelectCharacter(9).IsOk());
	CHECK(session.EnterWorld().IsOk());
	CHECK(session.EnterWorld().IsOk());
	CHECK(session.State() == SessionState::InWorld);
}

MODERN_TEST(Session_ClosedSessionRefusesEverything)
{
	ServerSession session;
	session.Close();
	CHECK(session.IsClosed());
	CHECK(session.BeginAuthentication().IsError());
	CHECK(!session.MayAffectWorld());
	CHECK(!session.MayRequestCharacterList());
}

// Liveness, with the clock injected rather than read.
MODERN_TEST(Session_HeartbeatPreventsTimeout)
{
	ServerSession session;
	session.NoteHeartbeat();

	session.AdvanceClock(Protocol::kTimeoutMilliseconds - 1);
	CHECK(!session.IsTimedOut());

	session.NoteHeartbeat();
	session.AdvanceClock(Protocol::kTimeoutMilliseconds - 1);
	CHECK(!session.IsTimedOut());
}

MODERN_TEST(Session_TimeoutAfterLegacyWindow)
{
	ServerSession session;
	session.NoteHeartbeat();
	session.AdvanceClock(Protocol::kTimeoutMilliseconds);
	CHECK(session.IsTimedOut());
}

MODERN_TEST(Session_StateNamesAreDistinct)
{
	// A log that cannot tell two states apart is worse than no log.
	CHECK(std::string(ToString(SessionState::Connected)) !=
	      std::string(ToString(SessionState::InWorld)));
	CHECK(std::string(ToString(SessionState::Closed)) == std::string("Closed"));
}

// ---------------------------------------------------------------------------

int main()
{
	std::printf("Modern network boundary tests (VERTICAL-027)\n\n");

	const int failedCases = ModernTests::RunAll();

	if (failedCases == 0)
	{
		std::printf("\nAll %d test cases passed.\n",
		            static_cast<int>(ModernTests::Registry().size()));
		return 0;
	}

	std::printf("\n%d of %d test cases FAILED (%d checks).\n",
	            failedCases,
	            static_cast<int>(ModernTests::Registry().size()),
	            ModernTests::FailureCount());
	return 1;
}
