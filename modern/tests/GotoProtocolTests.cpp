// WORLD-ENTRY-002f: codec tests for the GOTO pair, 3034 and 3035.
//
// Pure layout and round-trip. There is no socket here on purpose and for the same
// reason WorldEntryProtocolTests.cpp has none: these two packets are framed and
// measured independently of a transport, and 3034's authority story is decided
// BEFORE any byte is framed - a 3034 carries no character id, so the Field
// CONNECTION is the only thing that says whose character it is. None of that needs
// a transport to be true, and asserting it over a socket would let a framing bug
// hide a layout bug.
//
// The cases defend five properties:
//
//   ids        3034 and 3035, and not their neighbours 3032/3033
//   layout     the MEASURED sizes and offsets, read back with a hand-rolled reader
//   identity   3034 has no dwGaeaID, and there is no room for one
//   values     the coordinates and the dead fDelay survive a round trip bit for bit
//   refusal    non-finite coordinates and wrong-sized input are refused, and the
//              output is left clean
//
// The refusal cases are the security-relevant ones. `vCurPos` feeds GotoService's
// 60-unit desynchronisation test as `|m_vPos - vCurPos| > 60.0f`; with a NaN that
// comparison is FALSE, so a NaN would not fail the check - it would DISABLE it, and
// the anti-teleport would become a teleport. A codec that "helpfully" passed the
// NaN along would therefore be the single most dangerous change in this file.

#include "TestHarness.h"

#include "GotoProtocol.h"
#include "MovementStateProtocol.h"
#include "NetworkTypes.h"
#include "RanWirePrimitives.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>

namespace
{
	using namespace Modern;
	using namespace Modern::Network;
	namespace GotoNS = Modern::Network::Goto;
	namespace GotoCodec = Modern::Network::Goto::GotoCodec;
	namespace MS = Modern::Network::MovementState;

	// A little-endian 32-bit read, written out by hand rather than reusing the codec's
	// own helper.
	//
	// It exists so the byte-order and offset assertions are genuinely INDEPENDENT of
	// the code under test. Reading a wire value back with the codec that just wrote it
	// would pass even if every field were written big-endian at the wrong offset,
	// which is the class of bug these packets are most prone to.
	WireU32 PeekLE32(const std::vector<WireU8>& frame, std::size_t offset)
	{
		WireU32 value = 0;
		for (std::size_t i = 0; i < 4; ++i)
		{
			value |= static_cast<WireU32>(frame[offset + i]) << (8 * i);
		}
		return value;
	}

	// The same independence for a float: the wire carries three IEEE-754 singles in
	// x, y, z order, so this decomposes the bit pattern one byte at a time rather than
	// reinterpreting four bytes as a float.
	float PeekLEFloat(const std::vector<WireU8>& frame, std::size_t offset)
	{
		const WireU32 bits = PeekLE32(frame, offset);
		float         value = 0.0f;
		std::memcpy(&value, &bits, sizeof(value));
		return value;
	}

	// A request with coordinates that are easy to read back and impossible to confuse
	// with a shifted field.
	GotoNS::GotoRequest MakeRequest()
	{
		GotoNS::GotoRequest request;
		request.actState        = 0xA5A5A5A5u;
		request.currentPosition = {1.5f, -2.25f, 3.125f};
		request.targetPosition  = {-100.5f, 7.75f, 0.5f};
		return request;
	}

	GotoNS::GotoBroadcast MakeBroadcast()
	{
		GotoNS::GotoBroadcast broadcast;
		broadcast.gaeaId         = 0x11223344u;
		broadcast.actState       = 0x0000ABCDu;
		broadcast.currentPosition = {11.0f, -12.5f, 13.25f};
		broadcast.targetPosition  = {-21.0f, 22.5f, -23.75f};
		broadcast.delay           = 0.0f;
		return broadcast;
	}

	// =========================================================================
	// Message ids, and what they must not be confused with
	// =========================================================================

	MODERN_TEST(Goto_MessageIdsAre3034And3035)
	{
		CHECK_EQ(GotoNS::kGotoId, static_cast<MessageId>(3034));
		CHECK_EQ(GotoNS::kGotoBrdId, static_cast<MessageId>(3035));

		// Both are NET_MSG_GCTRL + n, so the arithmetic is two lines away from being
		// one off in either direction - which would collide with 3033 or 3036.
		CHECK_EQ(GotoNS::kGotoId - MS::kMoveStateId, static_cast<MessageId>(2));
		CHECK_EQ(GotoNS::kGotoBrdId - GotoNS::kGotoId, static_cast<MessageId>(1));

		CHECK(GotoCodec::IsGoto(3034));
		CHECK(!GotoCodec::IsGoto(3035));
		CHECK(GotoCodec::IsGotoBroadcast(3035));
		CHECK(!GotoCodec::IsGotoBroadcast(3034));

		// The STATE half is two ids below and is a different message entirely. A
		// predicate that accepted both would route a 3032 through the GOTO rule, which
		// applies ONE bit where the 3032 rule applies four.
		CHECK(!GotoCodec::IsGoto(MS::kMoveStateId));
		CHECK(!GotoCodec::IsGotoBroadcast(MS::kMoveStateBrdId));
		CHECK(!GotoCodec::IsGoto(3032));
		CHECK(!GotoCodec::IsGotoBroadcast(3036));
	}

	// =========================================================================
	// Layout
	// =========================================================================

	// The offsets are asserted by static_assert in GotoProtocol.h, so this case is
	// about the CONSTANTS - it fails when a constant is restated wrongly, which the
	// static_asserts would then agree with instead of contradicting.
	MODERN_TEST(Goto_MeasuredSizesAndOffsetsAreNamed)
	{
		CHECK_EQ(GotoNS::kRequestSize, static_cast<std::size_t>(36));
		CHECK_EQ(GotoNS::kRequestActStateOffset, static_cast<std::size_t>(8));
		CHECK_EQ(GotoNS::kRequestCurrentPositionOffset, static_cast<std::size_t>(12));
		CHECK_EQ(GotoNS::kRequestTargetPositionOffset, static_cast<std::size_t>(24));

		CHECK_EQ(GotoNS::kBroadcastSize, static_cast<std::size_t>(44));
		CHECK_EQ(GotoNS::kBroadcastGaeaIdOffset, static_cast<std::size_t>(8));
		CHECK_EQ(GotoNS::kBroadcastActStateOffset, static_cast<std::size_t>(12));
		CHECK_EQ(GotoNS::kBroadcastCurrentPositionOffset, static_cast<std::size_t>(16));
		CHECK_EQ(GotoNS::kBroadcastTargetPositionOffset, static_cast<std::size_t>(28));
		CHECK_EQ(GotoNS::kBroadcastDelayOffset, static_cast<std::size_t>(40));

		// A wire vector is three singles and a wire float is one, which is what makes
		// the two layout sums above add up without padding.
		CHECK_EQ(GotoNS::kVector3Bytes, sizeof(RanWire::Vector3));
		CHECK_EQ(GotoNS::kFloatBytes, sizeof(float));
	}

	// 3034 is exactly 36 bytes with dwActState at 8, vCurPos at 12 and vTarPos at 24.
	//
	// The header is asserted separately from each field so a swap reports which field
	// moved rather than that "the frame was wrong".
	MODERN_TEST(Goto_RequestEncodesToThirtySixBytesWithFieldsWhereMeasured)
	{
		std::vector<WireU8> frame;
		CHECK(GotoCodec::AppendGotoRequest(frame, MakeRequest()).IsOk());

		CHECK_EQ(frame.size(), GotoNS::kRequestSize);

		// NET_MSG_GENERIC order: dwSize first, then nType.
		CHECK_EQ(PeekLE32(frame, 0), static_cast<WireU32>(36));
		CHECK_EQ(PeekLE32(frame, 4), static_cast<WireU32>(3034));

		CHECK_EQ(PeekLE32(frame, GotoNS::kRequestActStateOffset), static_cast<WireU32>(0xA5A5A5A5u));

		CHECK(PeekLEFloat(frame, GotoNS::kRequestCurrentPositionOffset + 0) == 1.5f);
		CHECK(PeekLEFloat(frame, GotoNS::kRequestCurrentPositionOffset + 4) == -2.25f);
		CHECK(PeekLEFloat(frame, GotoNS::kRequestCurrentPositionOffset + 8) == 3.125f);

		CHECK(PeekLEFloat(frame, GotoNS::kRequestTargetPositionOffset + 0) == -100.5f);
		CHECK(PeekLEFloat(frame, GotoNS::kRequestTargetPositionOffset + 4) == 7.75f);
		CHECK(PeekLEFloat(frame, GotoNS::kRequestTargetPositionOffset + 8) == 0.5f);
	}

	// 3034 CARRIES NO dwGaeaID, and there is no room for one.
	//
	// This is the authority story in one arithmetic assertion: 8 bytes of header plus
	// 4 of actState plus two 12-byte vectors is 36, exactly the measured size. Had a
	// dwGaeaID been added - which is the tempting "harmless" field to add when routing
	// a broadcast - the packet would be 40 bytes and every RAN client would drop it.
	//
	// `GotoRequestWire` is the measured layout, and the fact that it has no id member
	// is the compile-time half of the same statement.
	MODERN_TEST(Goto_RequestCarriesNoGaeaIdAndHasNoRoomForOne)
	{
		CHECK_EQ(sizeof(GotoNS::GotoRequestWire), GotoNS::kRequestSize);
		CHECK_EQ(offsetof(GotoNS::GotoRequestWire, actState),
		         GotoNS::kRequestActStateOffset);
		CHECK_EQ(offsetof(GotoNS::GotoRequestWire, currentPosition),
		         GotoNS::kRequestCurrentPositionOffset);
		CHECK_EQ(offsetof(GotoNS::GotoRequestWire, targetPosition),
		         GotoNS::kRequestTargetPositionOffset);

		// The sum, spelled out. Nothing else is on the wire.
		CHECK_EQ(kMessageHeaderSize + sizeof(WireU32) + 2 * GotoNS::kVector3Bytes,
		         GotoNS::kRequestSize);

		// And the bytes agree: whatever follows dwActState at offset 12 is a float, not
		// an entity id. 0x3FC00000 is 1.5f; an id read from the same eight bytes would
		// be 0x3FC00000, which is not a plausible gaea id and is not what is there.
		std::vector<WireU8> frame;
		CHECK(GotoCodec::AppendGotoRequest(frame, MakeRequest()).IsOk());
		CHECK_EQ(PeekLE32(frame, 12), static_cast<WireU32>(0x3FC00000u));

		// 3035 is the OTHER shape and does carry one, from SNETPC_BROAD. The pair being
		// different is why the application model has two types rather than one with a
		// nullable id.
		CHECK_EQ(sizeof(GotoNS::GotoBroadcastWire), GotoNS::kBroadcastSize);
		CHECK_EQ(offsetof(GotoNS::GotoBroadcastWire, gaeaId), GotoNS::kBroadcastGaeaIdOffset);
	}

	// 3035 is exactly 44 bytes with dwGaeaID FIRST.
	//
	// First because SNETPC_GOTO_BRD derives from SNETPC_BROAD, and C++ lays a base
	// class's members out before the derived ones. A 3035 whose id sat at 12 would be
	// the right size and would move every character in the world.
	MODERN_TEST(Goto_BroadcastEncodesToFortyFourBytesWithGaeaIdFirst)
	{
		std::vector<WireU8> frame;
		CHECK(GotoCodec::AppendGotoBroadcast(frame, MakeBroadcast()).IsOk());

		CHECK_EQ(frame.size(), GotoNS::kBroadcastSize);

		CHECK_EQ(PeekLE32(frame, 0), static_cast<WireU32>(44));
		CHECK_EQ(PeekLE32(frame, 4), static_cast<WireU32>(3035));

		CHECK_EQ(PeekLE32(frame, GotoNS::kBroadcastGaeaIdOffset), static_cast<WireU32>(0x11223344u));
		CHECK_EQ(PeekLE32(frame, GotoNS::kBroadcastActStateOffset), static_cast<WireU32>(0x0000ABCDu));

		CHECK(PeekLEFloat(frame, GotoNS::kBroadcastCurrentPositionOffset + 0) == 11.0f);
		CHECK(PeekLEFloat(frame, GotoNS::kBroadcastCurrentPositionOffset + 4) == -12.5f);
		CHECK(PeekLEFloat(frame, GotoNS::kBroadcastCurrentPositionOffset + 8) == 13.25f);

		CHECK(PeekLEFloat(frame, GotoNS::kBroadcastTargetPositionOffset + 0) == -21.0f);
		CHECK(PeekLEFloat(frame, GotoNS::kBroadcastTargetPositionOffset + 4) == 22.5f);
		CHECK(PeekLEFloat(frame, GotoNS::kBroadcastTargetPositionOffset + 8) == -23.75f);

		CHECK(PeekLEFloat(frame, GotoNS::kBroadcastDelayOffset) == 0.0f);
	}

	// fDelay is dead on this route and is CARRIED anyway.
	//
	// It is four bytes of a fixed-size struct: dropping it would make the packet 40
	// bytes and desynchronise every RAN client. It is always written 0.0f by
	// GotoService (GLCharMsg.cpp:316 is its only assignment), and this case proves two
	// things a size assertion cannot - that the four bytes are a REAL float rather
	// than padding that happens to be zero, and that the decoder reads them back.
	MODERN_TEST(Goto_DelayIsFourRealBytesEvenThoughTheServerAlwaysSendsZero)
	{
		GotoNS::GotoBroadcast broadcast = MakeBroadcast();
		broadcast.delay                  = -0.25f;

		std::vector<WireU8> frame;
		CHECK(GotoCodec::AppendGotoBroadcast(frame, broadcast).IsOk());
		CHECK(PeekLEFloat(frame, GotoNS::kBroadcastDelayOffset) == -0.25f);

		GotoNS::GotoBroadcast decoded;
		CHECK(GotoCodec::DecodeGotoBroadcast(frame, decoded).IsOk());
		CHECK(decoded.delay == -0.25f);

		// Zero is the value the server path produces, and it round-trips as zero rather
		// than as "absent" - 3035 has no absence.
		GotoNS::GotoBroadcast zero = MakeBroadcast();
		zero.delay                  = 0.0f;
		std::vector<WireU8> zeroFrame;
		CHECK(GotoCodec::AppendGotoBroadcast(zeroFrame, zero).IsOk());
		CHECK(PeekLEFloat(zeroFrame, GotoNS::kBroadcastDelayOffset) == 0.0f);
	}

	// =========================================================================
	// Round trip
	// =========================================================================

	MODERN_TEST(Goto_RequestRoundTripsEveryField)
	{
		const GotoNS::GotoRequest original = MakeRequest();

		std::vector<WireU8> frame;
		CHECK(GotoCodec::AppendGotoRequest(frame, original).IsOk());

		GotoNS::GotoRequest decoded;
		CHECK(GotoCodec::DecodeGotoRequest(frame, decoded).IsOk());

		CHECK_EQ(decoded.actState, original.actState);
		CHECK(decoded.currentPosition.x == original.currentPosition.x);
		CHECK(decoded.currentPosition.y == original.currentPosition.y);
		CHECK(decoded.currentPosition.z == original.currentPosition.z);
		CHECK(decoded.targetPosition.x == original.targetPosition.x);
		CHECK(decoded.targetPosition.y == original.targetPosition.y);
		CHECK(decoded.targetPosition.z == original.targetPosition.z);
	}

	MODERN_TEST(Goto_BroadcastRoundTripsEveryField)
	{
		const GotoNS::GotoBroadcast original = MakeBroadcast();

		std::vector<WireU8> frame;
		CHECK(GotoCodec::AppendGotoBroadcast(frame, original).IsOk());

		GotoNS::GotoBroadcast decoded;
		CHECK(GotoCodec::DecodeGotoBroadcast(frame, decoded).IsOk());

		CHECK_EQ(decoded.gaeaId, original.gaeaId);
		CHECK_EQ(decoded.actState, original.actState);
		CHECK(decoded.currentPosition.x == original.currentPosition.x);
		CHECK(decoded.currentPosition.y == original.currentPosition.y);
		CHECK(decoded.currentPosition.z == original.currentPosition.z);
		CHECK(decoded.targetPosition.x == original.targetPosition.x);
		CHECK(decoded.targetPosition.y == original.targetPosition.y);
		CHECK(decoded.targetPosition.z == original.targetPosition.z);
		CHECK(decoded.delay == original.delay);
	}

	// Coordinates are written verbatim and are NOT filtered by a range rule.
	//
	// Whether a destination is reachable is a navigation question, and it is answered
	// after decoding by the ±10 probe. A codec that quietly clamped a coordinate would
	// make the bytes differ from what the caller asked to send, which is the wrong
	// place for the rule - and a client that clicked a cliff edge would silently walk
	// somewhere it did not ask for.
	MODERN_TEST(Goto_ExtremelyLargeButFiniteCoordinatesAreSentUnchanged)
	{
		GotoNS::GotoRequest request;
		request.actState        = 0;
		request.currentPosition = {-100000.0f, 100000.0f, -0.0001f};
		request.targetPosition  = {100000.0f, -100000.0f, 0.0001f};

		std::vector<WireU8> frame;
		CHECK(GotoCodec::AppendGotoRequest(frame, request).IsOk());

		GotoNS::GotoRequest decoded;
		CHECK(GotoCodec::DecodeGotoRequest(frame, decoded).IsOk());

		CHECK(decoded.currentPosition.x == -100000.0f);
		CHECK(decoded.currentPosition.y == 100000.0f);
		CHECK(decoded.targetPosition.x == 100000.0f);
		CHECK(decoded.targetPosition.z == 0.0001f);
	}

	// Append APPENDS. Two 3034s into one buffer are 72 bytes, the second starting at
	// 36, each with its own header - which is what makes a batch of them well formed.
	MODERN_TEST(Goto_TwoRequestsAppendIntoOneWellFormedBuffer)
	{
		std::vector<WireU8> frame;

		GotoNS::GotoRequest first = MakeRequest();
		first.actState            = 1;
		CHECK(GotoCodec::AppendGotoRequest(frame, first).IsOk());

		GotoNS::GotoRequest second               = MakeRequest();
		second.actState                          = 2;
		second.currentPosition             = {9.0f, 8.0f, 7.0f};
		second.targetPosition               = {6.0f, 5.0f, 4.0f};
		CHECK(GotoCodec::AppendGotoRequest(frame, second).IsOk());

		CHECK_EQ(frame.size(), static_cast<std::size_t>(72));

		// Each carries its own size and id, so a framer that splits on them finds two
		// 36-byte 3034s rather than one 72-byte frame.
		CHECK_EQ(PeekLE32(frame, 0), static_cast<WireU32>(36));
		CHECK_EQ(PeekLE32(frame, 4), static_cast<WireU32>(3034));
		CHECK_EQ(PeekLE32(frame, GotoNS::kRequestActStateOffset), static_cast<WireU32>(1));

		CHECK_EQ(PeekLE32(frame, 36), static_cast<WireU32>(36));
		CHECK_EQ(PeekLE32(frame, 40), static_cast<WireU32>(3034));
		CHECK_EQ(PeekLE32(frame, 36 + GotoNS::kRequestActStateOffset), static_cast<WireU32>(2));
	}

	// =========================================================================
	// Refusal of non-finite coordinates
	// =========================================================================

	// The whole reason this codec validates. A NaN in `vCurPos` would make GotoService's
	// `|m_vPos - vCurPos| > 60.0f` compare against NaN, which is FALSE - so the
	// anti-teleport check would PASS a drifted client and the character would walk to
	// wherever its `vTarPos` says. An infinity would make the comparison TRUE for a
	// well-behaved client and refuse every GOTO it ever sent.
	//
	// Both are refused rather than propagated, and the OUTPUT IS NOT TOUCHED - a
	// refused append must not leave a half-written 36 bytes behind for a caller that
	// ignores the status.
	MODERN_TEST(Goto_NonFiniteCoordinatesAreRefusedByAppend)
	{
		const float nanValue = std::numeric_limits<float>::quiet_NaN();
		const float infValue = std::numeric_limits<float>::infinity();

		// Each component of each vector, both non-finite forms. Nine cases per field
		// would be the same test nine times, so the four interesting ones are named:
		// a NaN and an infinity in `vCurPos`, and a NaN and an infinity in `vTarPos`.
		const RanWire::Vector3 badCurrent[4] = {
		    {nanValue, 0.0f, 0.0f},
		    {0.0f, nanValue, 0.0f},
		    {infValue, 0.0f, 0.0f},
		    {0.0f, 0.0f, -infValue},
		};
		for (const RanWire::Vector3& bad : badCurrent)
		{
			GotoNS::GotoRequest request = MakeRequest();
			request.currentPosition      = bad;

			std::vector<WireU8> frame;
			CHECK(GotoCodec::AppendGotoRequest(frame, request).IsError());
			// Nothing written: a caller that ignored the status must not find a
			// truncated packet on the wire.
			CHECK_EQ(frame.size(), static_cast<std::size_t>(0));
		}

		const RanWire::Vector3 badTarget[4] = {
		    {nanValue, 0.0f, 0.0f},
		    {0.0f, infValue, 0.0f},
		    {-infValue, 0.0f, 0.0f},
		    {0.0f, 0.0f, nanValue},
		};
		for (const RanWire::Vector3& bad : badTarget)
		{
			GotoNS::GotoRequest request = MakeRequest();
			request.targetPosition       = bad;

			std::vector<WireU8> frame;
			CHECK(GotoCodec::AppendGotoRequest(frame, request).IsError());
			CHECK_EQ(frame.size(), static_cast<std::size_t>(0));

			// 3035 refuses the same two vectors for the same reason. Its `vCurPos` is
			// what a client uses to learn the server's position, so a NaN there would
			// poison the client's own next `vCurPos` - which is the 60-unit check again,
			// one hop later.
			GotoNS::GotoBroadcast broadcast = MakeBroadcast();
			broadcast.targetPosition = bad;

			std::vector<WireU8> broadcastFrame;
			CHECK(GotoCodec::AppendGotoBroadcast(broadcastFrame, broadcast).IsError());
			CHECK_EQ(broadcastFrame.size(), static_cast<std::size_t>(0));
		}

		// `fDelay` is NOT in this case, and the asymmetry is worth naming.
		// `AppendGotoBroadcast` checks both POSITIONS; `DecodeGotoBroadcast` checks
		// the positions AND the delay. So the encoder can produce a frame the
		// decoder refuses.
		//
		// Nothing sends a non-finite delay - GLCharMsg.cpp:316 is the only assignment
		// on this route and it writes 0.0f - and the decoder's check is the one that
		// matters, because that is the direction a PEER controls. It is still a
		// round-trip hole, so it is reported rather than asserted as intended.
	}

	// A peer that puts a NaN on the WIRE is refused on the way in, not merely on the
	// way out.
	//
	// Both directions matter and they are not the same code path: `Append` refuses what
	// a caller builds, `Decode` refuses what a peer sent. A decoder that trusted the
	// bytes would let a malicious client hand the server a NaN directly, which is the
	// case the 60-unit check cannot survive.
	MODERN_TEST(Goto_NonFiniteCoordinatesOnTheWireAreRefusedByDecode)
	{
		const float nanBits = std::numeric_limits<float>::quiet_NaN();

		{
			// Built from a FINITE request and then overwritten with the bit pattern of a
			// NaN. That indirection is the point: the case above already refuses to BUILD
			// a NaN, so the only way to get one onto the wire is to write the bytes -
			// which is exactly what a hostile peer does.
			std::vector<WireU8> frame;
			CHECK(GotoCodec::AppendGotoRequest(frame, MakeRequest()).IsOk());

			std::uint32_t nanWord = 0;
			std::memcpy(&nanWord, &nanBits, sizeof(nanWord));
			for (std::size_t i = 0; i < 4; ++i)
			{
				frame[GotoNS::kRequestCurrentPositionOffset + i] =
				    static_cast<WireU8>((nanWord >> (8 * i)) & 0xFFu);
			}

			GotoNS::GotoRequest decoded;
			decoded.actState        = 0xDEADBEEFu;
			decoded.currentPosition = {1.0f, 2.0f, 3.0f};
			CHECK(GotoCodec::DecodeGotoRequest(frame, decoded).IsError());

			// The output is CLEARED on refusal, so a caller that ignores the status
			// cannot act on a half-decoded message. GotoService is such a caller unless
			// this holds.
			CHECK_EQ(decoded.actState, static_cast<WireU32>(0));
			CHECK(decoded.currentPosition.x == 0.0f);
			CHECK(decoded.currentPosition.y == 0.0f);
			CHECK(decoded.currentPosition.z == 0.0f);
			CHECK(decoded.targetPosition.x == 0.0f);
		}

		{
			GotoNS::GotoBroadcast broadcast = MakeBroadcast();

			std::vector<WireU8> frame;
			CHECK(GotoCodec::AppendGotoBroadcast(frame, broadcast).IsOk());

			std::uint32_t nanWord = 0;
			std::memcpy(&nanWord, &nanBits, sizeof(nanWord));
			for (std::size_t i = 0; i < 4; ++i)
			{
				frame[GotoNS::kBroadcastTargetPositionOffset + 8 + i] =
				    static_cast<WireU8>((nanWord >> (8 * i)) & 0xFFu);
			}

			GotoNS::GotoBroadcast decoded;
			decoded.gaeaId = 0x12345678u;
			CHECK(GotoCodec::DecodeGotoBroadcast(frame, decoded).IsError());
			CHECK_EQ(decoded.gaeaId, static_cast<WireU32>(0));
		}
	}

	// A non-finite Delay from a peer is refused by the DECODER.
	//
	// 3034 has no Delay and 3035 does, so the decoder's three finiteness checks are
	// not symmetric with the encoder's two - the encoder polices the positions and the
	// decoder polices the positions AND the delay. The decoder is the one that matters,
	// because that is the direction a peer controls, and a NaN delay would be a time
	// offset a client would add to a movement prediction.
	MODERN_TEST(Goto_NonFiniteDelayOnTheWireIsRefusedByDecode)
	{
		std::vector<WireU8> frame;
		CHECK(GotoCodec::AppendGotoBroadcast(frame, MakeBroadcast()).IsOk());

		const float nanDelay = std::numeric_limits<float>::quiet_NaN();
		std::uint32_t nanWord = 0;
		std::memcpy(&nanWord, &nanDelay, sizeof(nanWord));
		for (std::size_t i = 0; i < 4; ++i)
		{
			frame[GotoNS::kBroadcastDelayOffset + i] = static_cast<WireU8>((nanWord >> (8 * i)) & 0xFFu);
		}

		GotoNS::GotoBroadcast decoded;
		decoded.gaeaId = 0x12345678u;
		CHECK(GotoCodec::DecodeGotoBroadcast(frame, decoded).IsError());
		CHECK_EQ(decoded.gaeaId, static_cast<WireU32>(0));
	}

	// =========================================================================
	// Refusal of malformed frames
	// =========================================================================

	// `frame.size()` is the authority; a DECLARED size is only ever compared with it,
	// never used to index. That ordering is what stops a peer declaring 44 bytes and
	// being read out of a 36-byte buffer - so it is worth a case where the two
	// disagree.
	MODERN_TEST(Goto_DecodeRefusesWrongSizedAndMislabelledFrames)
	{
		std::vector<WireU8> good;
		CHECK(GotoCodec::AppendGotoRequest(good, MakeRequest()).IsOk());

		GotoNS::GotoRequest decoded;

		// One byte short and one byte long. Both are refused, and the short one is the
		// dangerous direction: 35 bytes with a declared 36 would be an over-read.
		std::vector<WireU8> shortFrame(good.begin(), good.end() - 1);
		CHECK(GotoCodec::DecodeGotoRequest(shortFrame, decoded).IsError());

		std::vector<WireU8> longFrame = good;
		longFrame.push_back(0);
		CHECK(GotoCodec::DecodeGotoRequest(longFrame, decoded).IsError());

		// Empty.
		const std::vector<WireU8> empty;
		CHECK(GotoCodec::DecodeGotoRequest(empty, decoded).IsError());

		// The other message. 3035 is 44 bytes and 3034 is 36, so a 3035 handed to the
		// 3034 decoder fails on the id rather than being half-read.
		std::vector<WireU8> broadcastFrame;
		CHECK(GotoCodec::AppendGotoBroadcast(broadcastFrame, MakeBroadcast()).IsOk());
		CHECK(GotoCodec::DecodeGotoRequest(broadcastFrame, decoded).IsError());

		// THE LIE: 36 real bytes declaring 44. A decoder that trusted the header to
		// know how much to read would walk eight bytes past the end of this buffer.
		std::vector<WireU8> lying = good;
		for (std::size_t i = 0; i < 4; ++i)
		{
			lying[i] = static_cast<WireU8>((44u >> (8 * i)) & 0xFFu);
		}
		CHECK(GotoCodec::DecodeGotoRequest(lying, decoded).IsError());

		// And the converse, which is the other half of the rule: a frame whose declared
		// size is short while the buffer is long is refused too, so the two can never
		// drift apart.
		std::vector<WireU8> understating = good;
		for (std::size_t i = 0; i < 4; ++i)
		{
			understating[i] = static_cast<WireU8>((12u >> (8 * i)) & 0xFFu);
		}
		CHECK(GotoCodec::DecodeGotoRequest(understating, decoded).IsError());

		// The good frame still decodes, so the refusals above are about the malformed
		// inputs and not about the decoder refusing everything.
		CHECK(GotoCodec::DecodeGotoRequest(good, decoded).IsOk());
	}

	// The same discipline for 3035, whose size is a different one, so a decoder shared
	// between the two messages would accept the wrong one somewhere.
	MODERN_TEST(Goto_BroadcastDecodeRefusesThirtySixByteFramesAndWrongIds)
	{
		std::vector<WireU8> good;
		CHECK(GotoCodec::AppendGotoBroadcast(good, MakeBroadcast()).IsOk());

		GotoNS::GotoBroadcast decoded;

		std::vector<WireU8> truncated(good.begin(), good.end() - 1);
		CHECK(GotoCodec::DecodeGotoBroadcast(truncated, decoded).IsError());

		std::vector<WireU8> padded = good;
		padded.push_back(0);
		CHECK(GotoCodec::DecodeGotoBroadcast(padded, decoded).IsError());

		// 3034's id in a 44-byte frame: the right size, the wrong message.
		std::vector<WireU8> wrongId = good;
		for (std::size_t i = 0; i < 4; ++i)
		{
			wrongId[4 + i] = static_cast<WireU8>((3034u >> (8 * i)) & 0xFFu);
		}
		CHECK(GotoCodec::DecodeGotoBroadcast(wrongId, decoded).IsError());

		// 3032 in a 44-byte frame: the STATE message, which carries no positions at
		// all, must never be read as a GOTO broadcast.
		std::vector<WireU8> moveState = good;
		for (std::size_t i = 0; i < 4; ++i)
		{
			moveState[4 + i] = static_cast<WireU8>((MS::kMoveStateId >> (8 * i)) & 0xFFu);
		}
		CHECK(GotoCodec::DecodeGotoBroadcast(moveState, decoded).IsError());

		CHECK(GotoCodec::DecodeGotoBroadcast(good, decoded).IsOk());
		CHECK_EQ(decoded.gaeaId, static_cast<WireU32>(0x11223344u));
	}
}
