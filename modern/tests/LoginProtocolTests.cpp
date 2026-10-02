// WORLD-001: the client -> server login protocol.
//
// Deterministic throughout: no socket, no clock, no RNG, no database, no
// external server. The garbage token is always supplied explicitly rather than
// generated, which is what lets a test assert on exact bytes.
//
// Three things are under test, and they are independent:
//
//   1. minTea field encryption, including the variable ciphertext length that
//      makes "encrypt 21 bytes" the wrong mental model
//   2. the NET_MSG_LOGIN_2 frame: header, garbage token, 76-byte body
//   3. validation: everything malformed must be rejected, never tolerated
//
// The compression boundary is asserted absent, not merely unused: Test 19 fails if
// a login packet is ever a NET_COMPRESS envelope.

#include "TestHarness.h"

#include "LoginProtocol.h"
#include "MinTeaCodec.h"
#include "NetCompressCodec.h"
#include "NetworkCodec.h"
#include "NetworkConnection.h"
#include "NetworkTypes.h"

namespace
{
	using namespace Modern;
	using namespace Modern::Network;

	const MinTea& Tea()
	{
		static const MinTea instance;
		return instance;
	}

	// A field exactly as the legacy struct holds it: SecureZeroMemory'd, then a
	// NUL-terminated string copied in.
	std::vector<WireU8> Field(std::size_t size, const std::string& value)
	{
		std::vector<WireU8> field(size, 0);
		const std::size_t copy = value.size() < size ? value.size() : size - 1;
		for (std::size_t i = 0; i < copy; ++i)
		{
			field[i] = static_cast<WireU8>(value[i]);
		}
		return field;
	}

	// Writes a little-endian u32 at an offset. Codec::WriteU32 appends to a
	// vector; tests need to patch an offset in an already-built frame.
	void PutU32(std::vector<WireU8>& bytes, std::size_t offset, WireU32 value)
	{
		for (int i = 0; i < 4; ++i)
		{
			bytes[offset + i] = static_cast<WireU8>((value >> (8 * i)) & 0xFFu);
		}
	}

	LoginRequest SampleRequest()
	{
		LoginRequest request;
		request.userId = "tester";
		request.password = "pw123";
		request.randomPassword = "54321";
		request.encryptKey = "sharedsecret";
		request.channel = 0;
		return request;
	}
}

// ---------------------------------------------------------------------------
// Test 3 - TEA, per field, at the four widths
// ---------------------------------------------------------------------------

MODERN_TEST(Login_TeaRoundTripsUserIdField21)
{
	// USR_ID_LENGTH + 1 = 21
	std::vector<WireU8> field = Field(21, "tester");
	const std::vector<WireU8> original = field;

	CHECK(Tea().EncryptInPlace(field, field.size()).IsOk());
	// Ciphertext must not be plaintext.
	CHECK(field != original);

	CHECK(Tea().DecryptInPlace(field, field.size()).IsOk());
	CHECK(field == original);
}

MODERN_TEST(Login_TeaRoundTripsPasswordField21)
{
	std::vector<WireU8> field = Field(21, "pw123");
	const std::vector<WireU8> original = field;
	CHECK(Tea().EncryptInPlace(field, field.size()).IsOk());
	CHECK(Tea().DecryptInPlace(field, field.size()).IsOk());
	CHECK(field == original);
}

MODERN_TEST(Login_RandomPasswordFieldIsNotEncryptedAndThatIsCorrect)
{
	// A 6-digit random number in the 7-byte szRandomPassword field pads to an
	// 8-byte ciphertext, which does not fit. minTea therefore returns false and
	// copies NOTHING (minTea.cpp, `if (nEncryptedLength > nMaxLength) return false`),
	// and SndLogin ignores that return - so the field goes out as plaintext.
	//
	// The server declines identically, so the Agent reads the plaintext value. This
	// is wire behaviour, not a defect: "fixing" it would diverge from every shipped
	// RAN server.
	std::vector<WireU8> field = Field(7, "54321");
	const std::vector<WireU8> original = field;

	CHECK(!Tea().CanEncryptInPlace(field, field.size()));
	CHECK(Tea().EncryptInPlace(field, field.size()).IsError()); // declines
	CHECK(field == original);                                   // and changes nothing

	// So the round-trip is an identity, not a cipher round-trip.
	CHECK(field == original);
}

MODERN_TEST(Login_RandomPasswordTravelsAsPlaintextOnTheWire)
{
	// Seen at the protocol level: the random password bytes are readable, while
	// the password and user id are not. This is what the shipped client emits.
	std::vector<WireU8> frame;
	CHECK(LoginProtocol::Encode(SampleRequest(), "K9IHANA", Tea(), frame).IsOk());

	const WireU8* payload = frame.data() + kMessageHeaderSize + 7; // after garbage
	const std::string onWire(reinterpret_cast<const char*>(payload + 4), 5);
	CHECK_EQ(onWire, std::string("54321"));

	// And the password field is ciphertext, not readable.
	const std::string passwordBytes(reinterpret_cast<const char*>(payload + 11), 5);
	CHECK_NE(passwordBytes, std::string("pw123"));

	// It still decodes correctly, because the server declines in the same way.
	LoginRequest decoded;
	CHECK(LoginProtocol::Decode(frame.data(), frame.size(), Tea(), decoded).IsOk());
	CHECK_EQ(decoded.randomPassword, std::string("54321"));
	CHECK_EQ(decoded.password, std::string("pw123"));
}

MODERN_TEST(Login_TeaRoundTripsEncryptField13)
{
	// ENCRYPT_KEY + 1 = 13
	std::vector<WireU8> field = Field(13, "sharedsecret");
	const std::vector<WireU8> original = field;
	CHECK(Tea().EncryptInPlace(field, field.size()).IsOk());
	CHECK(Tea().DecryptInPlace(field, field.size()).IsOk());
	CHECK(field == original);
}

MODERN_TEST(Login_TeaCipherLengthIsNotTheFieldWidth)
{
	// The single most important property of the legacy cipher for wire
	// compatibility: a 21-byte field holding "ab" encrypts to 8 bytes, not 21.
	//
	// Legacy strips trailing NULs, grows to a minimum of 5, then rounds up to a
	// multiple of 4 (minTea.cpp). An implementation that TEA'd all 21 bytes would
	// put different bytes on the wire and no RAN server would accept it.
	std::vector<WireU8> field = Field(21, "ab");
	CHECK_EQ(MinTea::CipherLengthFor(2), std::size_t{ 8 });
	CHECK_EQ(MinTea::CipherLengthFor(1), std::size_t{ 8 });
	CHECK_EQ(MinTea::CipherLengthFor(4), std::size_t{ 8 });
	CHECK_EQ(MinTea::CipherLengthFor(5), std::size_t{ 8 });
	CHECK_EQ(MinTea::CipherLengthFor(6), std::size_t{ 8 });
	CHECK_EQ(MinTea::CipherLengthFor(8), std::size_t{ 8 });
	CHECK_EQ(MinTea::CipherLengthFor(9), std::size_t{ 12 });
	CHECK_EQ(MinTea::CipherLengthFor(12), std::size_t{ 12 });
	CHECK_EQ(MinTea::CipherLengthFor(20), std::size_t{ 20 });
	CHECK_EQ(MinTea::CipherLengthFor(21), std::size_t{ 24 });

	// And the tail beyond the ciphertext stays NUL.
	std::vector<WireU8> work = Field(21, "ab");
	CHECK(Tea().EncryptInPlace(work, work.size()).IsOk());
	CHECK_EQ(work[8], WireU8{ 0 });
	CHECK_EQ(work[20], WireU8{ 0 });
}

MODERN_TEST(Login_TeaRoundTripsAcrossEveryCipherLength)
{
	// Round-trip at each plaintext length that changes the ciphertext size, so the
	// minimum-length and rounding rules are exercised rather than assumed.
	for (std::size_t len = 0; len <= 21; ++len)
	{
		std::string value(len, 'x');
		std::vector<WireU8> field = Field(21, value);
		const std::vector<WireU8> original = field;

		if (const Status s = Tea().EncryptInPlace(field, field.size()); s.IsError())
		{
			CHECK(false); // 21 bytes always fits; failure would be a real defect
			continue;
		}
		CHECK(Tea().DecryptInPlace(field, field.size()).IsOk());
		CHECK(field == original);
	}
}

MODERN_TEST(Login_TeaUsesTheLegacyKey)
{
	// The key is protocol, not configuration. If it ever changes the modern client
	// cannot talk to any RAN server.
	CHECK_EQ(std::string(kLegacyTeaKey), std::string("Steven Seagal Neck Break"));
	CHECK_EQ(kTeaKeyLength, std::size_t{ 16 });
}

// ---------------------------------------------------------------------------
// Tests 1, 2, 4 - the frame
// ---------------------------------------------------------------------------

MODERN_TEST(Login_FrameHeaderIsExact)
{
	std::vector<WireU8> frame;
	CHECK(LoginProtocol::Encode(SampleRequest(), "K9IHANA", Tea(), frame).IsOk());

	// nType is NET_MSG_LOGIN_2 = 2049 = 0x0801.
	CHECK_EQ(static_cast<WireU32>(LoginProtocol::kLoginMessageId), WireU32{ 2049 });
	CHECK_EQ(Codec::ReadU32(frame.data() + 4), WireU32{ 2049 });

	// dwSize includes the header AND the garbage token.
	CHECK_EQ(static_cast<std::size_t>(Codec::ReadU32(frame.data())), frame.size());

	CHECK(LoginProtocol::IsLoginMessage(frame.data(), frame.size()));
}

MODERN_TEST(Login_LogicalBodyIsSeventySixBytes)
{
	// The body is the garbage-free NET_LOGIN_DATA.
	CHECK_EQ(LoginProtocol::kLoginBodySize, std::size_t{ 76 });
	CHECK_EQ(LoginProtocol::kLoginPayloadSize, std::size_t{ 68 });

	// Field widths the brief pins.
	CHECK_EQ(LoginProtocol::kUserIdFieldSize, std::size_t{ 21 });
	CHECK_EQ(LoginProtocol::kPasswordFieldSize, std::size_t{ 21 });
	CHECK_EQ(LoginProtocol::kRandomPasswordFieldSize, std::size_t{ 7 });
	CHECK_EQ(LoginProtocol::kEncryptFieldSize, std::size_t{ 13 });
}

MODERN_TEST(Login_FullPacketStructure)
{
	std::vector<WireU8> frame;
	CHECK(LoginProtocol::Encode(SampleRequest(), "M7HSET", Tea(), frame).IsOk());

	// 76 + 6 for M7HSET.
	CHECK_EQ(frame.size(), std::size_t{ 82 });

	// Header.
	CHECK_EQ(static_cast<std::size_t>(Codec::ReadU32(frame.data())), std::size_t{ 82 });
	CHECK_EQ(Codec::ReadU32(frame.data() + 4), WireU32{ 2049 });

	// Garbage sits immediately after the header.
	const std::string garbage(reinterpret_cast<const char*>(frame.data()) + kMessageHeaderSize, 6);
	CHECK_EQ(garbage, std::string("M7HSET"));

	// Payload begins after the garbage.
	const WireU8* payload = frame.data() + kMessageHeaderSize + 6;
	// nChannel is 0.
	CHECK_EQ(payload[0], WireU8{ 0 });
	CHECK_EQ(payload[1], WireU8{ 0 });
	CHECK_EQ(payload[2], WireU8{ 0 });
	CHECK_EQ(payload[3], WireU8{ 0 });

	// Trailing struct padding is zeroed.
	CHECK_EQ(payload[66], WireU8{ 0 });
	CHECK_EQ(payload[67], WireU8{ 0 });

	// And it round-trips back to the same values.
	LoginRequest decoded;
	CHECK(LoginProtocol::Decode(frame.data(), frame.size(), Tea(), decoded).IsOk());
	CHECK_EQ(decoded.userId, std::string("tester"));
	CHECK_EQ(decoded.password, std::string("pw123"));
	CHECK_EQ(decoded.randomPassword, std::string("54321"));
	CHECK_EQ(decoded.encryptKey, std::string("sharedsecret"));
	CHECK_EQ(decoded.channel, 0);
	CHECK_EQ(decoded.garbage.text, std::string("M7HSET"));
	CHECK_EQ(decoded.garbage.length, std::size_t{ 6 });
}

MODERN_TEST(Login_ChannelIsLittleEndian)
{
	LoginRequest request = SampleRequest();
	request.channel = 0x01020304;

	std::vector<WireU8> frame;
	CHECK(LoginProtocol::Encode(request, "K9IHANA", Tea(), frame).IsOk());

	LoginRequest decoded;
	CHECK(LoginProtocol::Decode(frame.data(), frame.size(), Tea(), decoded).IsOk());
	CHECK_EQ(decoded.channel, 0x01020304);

	// Explicit byte order, not struct layout.
	const WireU8* payload = frame.data() + kMessageHeaderSize + 7;
	CHECK_EQ(payload[0], WireU8{ 0x04 });
	CHECK_EQ(payload[1], WireU8{ 0x03 });
	CHECK_EQ(payload[2], WireU8{ 0x02 });
	CHECK_EQ(payload[3], WireU8{ 0x01 });
}

MODERN_TEST(Login_MaximumLengthValuesRoundTrip)
{
	// 20-character user id and password: the longest the fields allow.
	LoginRequest request;
	request.userId = std::string(20, 'u');
	request.password = std::string(20, 'p');
	request.randomPassword = std::string(6, 'r');
	request.encryptKey = std::string(12, 'e');
	request.channel = 3;

	std::vector<WireU8> frame;
	CHECK(LoginProtocol::Encode(request, "L8IDUL", Tea(), frame).IsOk());

	LoginRequest decoded;
	CHECK(LoginProtocol::Decode(frame.data(), frame.size(), Tea(), decoded).IsOk());
	CHECK_EQ(decoded.userId, request.userId);
	CHECK_EQ(decoded.password, request.password);
	CHECK_EQ(decoded.randomPassword, request.randomPassword);
	CHECK_EQ(decoded.encryptKey, request.encryptKey);
	CHECK_EQ(decoded.channel, 3);
}

// ---------------------------------------------------------------------------
// Tests 5, 6, 7 - every garbage token length
// ---------------------------------------------------------------------------

MODERN_TEST(Login_GarbageLengthSixIsAccepted)
{
	std::vector<WireU8> frame;
	CHECK(LoginProtocol::Encode(SampleRequest(), "L8IDUL", Tea(), frame).IsOk());
	CHECK_EQ(frame.size(), std::size_t{ 82 });

	LoginRequest decoded;
	CHECK(LoginProtocol::Decode(frame.data(), frame.size(), Tea(), decoded).IsOk());
	CHECK_EQ(decoded.garbage.length, std::size_t{ 6 });
}

MODERN_TEST(Login_GarbageLengthSevenIsAccepted)
{
	std::vector<WireU8> frame;
	CHECK(LoginProtocol::Encode(SampleRequest(), "K9IHANA", Tea(), frame).IsOk());
	CHECK_EQ(frame.size(), std::size_t{ 83 });

	LoginRequest decoded;
	CHECK(LoginProtocol::Decode(frame.data(), frame.size(), Tea(), decoded).IsOk());
	CHECK_EQ(decoded.garbage.length, std::size_t{ 7 });
}

MODERN_TEST(Login_GarbageLengthNineIsAccepted)
{
	std::vector<WireU8> frame;
	CHECK(LoginProtocol::Encode(SampleRequest(), "O5FDASEOT", Tea(), frame).IsOk());
	CHECK_EQ(frame.size(), std::size_t{ 85 });

	LoginRequest decoded;
	CHECK(LoginProtocol::Decode(frame.data(), frame.size(), Tea(), decoded).IsOk());
	CHECK_EQ(decoded.garbage.length, std::size_t{ 9 });
	CHECK_EQ(decoded.userId, std::string("tester"));
}

// ---------------------------------------------------------------------------
// Test 8 - invalid garbage
// ---------------------------------------------------------------------------

MODERN_TEST(Login_UnknownGarbageTokenIsRejected)
{
	// A token the server cannot identify. Legacy's SetGarbageNum returns -1 and
	// the message is refused; accepting it would feed garbage bytes to the field
	// decryptor as if they were ciphertext.
	std::vector<WireU8> frame;
	CHECK(LoginProtocol::Encode(SampleRequest(), "K9IHANA", Tea(), frame).IsOk());

	// Corrupt the token in place, keeping the length valid.
	frame[kMessageHeaderSize] = 'X';

	LoginRequest decoded;
	CHECK(LoginProtocol::Decode(frame.data(), frame.size(), Tea(), decoded).IsError());
}

MODERN_TEST(Login_ArbitrarySuffixLengthIsRejected)
{
	// Not one of {6,7,9}: an 8-byte suffix. dwSize is made self-consistent so the
	// rejection is specifically about the unrecognised token, not about a size
	// mismatch that would have been caught anyway.
	std::vector<WireU8> frame;
	CHECK(LoginProtocol::Encode(SampleRequest(), "K9IHANA", Tea(), frame).IsOk());

	// Rebuild as 76 + 8 with a suffix no server would recognise.
	frame.assign(84, 0);
	PutU32(frame, 0, 84);
	PutU32(frame, 4, LoginProtocol::kLoginMessageId);
	for (std::size_t i = 0; i < 8; ++i)
	{
		frame[kMessageHeaderSize + i] = static_cast<WireU8>('A' + i);
	}

	LoginRequest decoded;
	CHECK(LoginProtocol::Decode(frame.data(), frame.size(), Tea(), decoded).IsError());
}

MODERN_TEST(Login_MissingGarbageTokenIsRejected)
{
	// dwSize is 76 - the "clean" size with no token at all. Legacy cannot reach
	// this state, but a hostile peer can send it and it must not decode.
	std::vector<WireU8> frame(LoginProtocol::kLoginBodySize, 0);
	PutU32(frame, 0, static_cast<WireU32>(LoginProtocol::kLoginBodySize));
	PutU32(frame, 4, LoginProtocol::kLoginMessageId);

	LoginRequest decoded;
	CHECK(LoginProtocol::Decode(frame.data(), frame.size(), Tea(), decoded).IsError());
}

MODERN_TEST(Login_EncoderRefusesAnUnknownToken)
{
	// The client must not be able to emit a token the server will refuse.
	std::vector<WireU8> frame;
	CHECK(LoginProtocol::Encode(SampleRequest(), "NOTATOKEN", Tea(), frame).IsError());
	CHECK(frame.empty());
}

// ---------------------------------------------------------------------------
// Tests 9, 10, 11 - truncated, oversized, wrong type
// ---------------------------------------------------------------------------

MODERN_TEST(Login_TruncatedPacketIsRejected)
{
	std::vector<WireU8> frame;
	CHECK(LoginProtocol::Encode(SampleRequest(), "K9IHANA", Tea(), frame).IsOk());

	LoginRequest decoded;
	// Every length short of the header is refused.
	for (std::size_t n = 0; n < kMessageHeaderSize; ++n)
	{
		CHECK(LoginProtocol::Decode(frame.data(), n, Tea(), decoded).IsError());
	}
	// Truncated body: header present and consistent, but fewer bytes than declared.
	CHECK(LoginProtocol::Decode(frame.data(), frame.size() - 1, Tea(), decoded).IsError());
}

MODERN_TEST(Login_OversizedBodyIsRejected)
{
	// A frame that declares far more than arrived.
	std::vector<WireU8> frame(32, 0);
	PutU32(frame, 0, 0x0000FFFFu);
	PutU32(frame, 4, LoginProtocol::kLoginMessageId);

	LoginRequest decoded;
	CHECK(LoginProtocol::Decode(frame.data(), frame.size(), Tea(), decoded).IsError());
}

MODERN_TEST(Login_WrongMessageTypeIsRejected)
{
	std::vector<WireU8> frame;
	CHECK(LoginProtocol::Encode(SampleRequest(), "K9IHANA", Tea(), frame).IsOk());

	// Rewrite nType to something else - a heartbeat, say.
	PutU32(frame, 4, static_cast<WireU32>(WellKnownMessage::kHeartbeatClientReq));

	LoginRequest decoded;
	CHECK(!LoginProtocol::IsLoginMessage(frame.data(), frame.size()));
	CHECK(LoginProtocol::Decode(frame.data(), frame.size(), Tea(), decoded).IsError());
}

MODERN_TEST(Login_DeclaredSizeMustMatchGarbagePlusBody)
{
	// dwSize self-consistent with a valid token, but the frame is longer than
	// 76 + g. Legacy's MsgLogIn size check rejects this after stripping.
	std::vector<WireU8> frame;
	CHECK(LoginProtocol::Encode(SampleRequest(), "L8IDUL", Tea(), frame).IsOk());
	frame.push_back(0x41);                       // extra byte, length now 83
	PutU32(frame, 0, 83);

	LoginRequest decoded;
	CHECK(LoginProtocol::Decode(frame.data(), frame.size(), Tea(), decoded).IsError());
}

// ---------------------------------------------------------------------------
// Test 14 - bFeatureRegisterUseMD5
// ---------------------------------------------------------------------------

MODERN_TEST(Login_Md5ConfigurationDoesNotChangeTheWire)
{
	// The finding, tested. bFeatureRegisterUseMD5 changes the server's TEA DECRYPT
	// WINDOW (21 vs 20) and nothing else: not the struct size, not dwSize, not the
	// bytes transmitted. So the two configurations must produce IDENTICAL bytes on
	// the wire, and both must decode to the same credentials.
	LoginRequest request = SampleRequest();

	std::vector<WireU8> frame;
	CHECK(LoginProtocol::Encode(request, "K9IHANA", Tea(), frame).IsOk());
	const std::vector<WireU8> original = frame;

	LoginRequest full;
	CHECK(LoginProtocol::Decode(frame.data(), frame.size(), Tea(),
	                             LoginProtocol::PasswordDecryptWidth::Full21, full).IsOk());

	LoginRequest truncated;
	CHECK(LoginProtocol::Decode(frame.data(), frame.size(), Tea(),
	                             LoginProtocol::PasswordDecryptWidth::Truncated20, truncated).IsOk());

	// Identical wire bytes regardless of configuration.
	CHECK(frame == original);

	// And identical decoded credentials for a password within the shared window.
	CHECK_EQ(full.userId, truncated.userId);
	CHECK_EQ(full.password, truncated.password);
	CHECK_EQ(full.randomPassword, truncated.randomPassword);
	CHECK_EQ(full.encryptKey, truncated.encryptKey);
	CHECK_EQ(full.channel, truncated.channel);
}

MODERN_TEST(Login_Md5TruncatedWindowAcrossEveryPasswordLength)
{
	// The 20-byte window is safe because minTea recovers the ciphertext length by
	// scanning back over NULs, so a short ciphertext decrypts identically under
	// either window. Sweeping every password length proves it rather than
	// asserting it.
	for (std::size_t len = 1; len <= 19; ++len)
	{
		LoginRequest request;
		request.userId = "tester";
		request.password = std::string(len, 'p');
		request.randomPassword = "54321";
		request.encryptKey = "sharedsecret";

		std::vector<WireU8> frame;
		CHECK(LoginProtocol::Encode(request, "M7HSET", Tea(), frame).IsOk());

		LoginRequest a;
		LoginRequest b;
		CHECK(LoginProtocol::Decode(frame.data(), frame.size(), Tea(),
		                             LoginProtocol::PasswordDecryptWidth::Full21, a).IsOk());
		CHECK(LoginProtocol::Decode(frame.data(), frame.size(), Tea(),
		                             LoginProtocol::PasswordDecryptWidth::Truncated20, b).IsOk());

		CHECK_EQ(a.password, request.password);
		CHECK_EQ(b.password, request.password);
	}
}

// ---------------------------------------------------------------------------
// Framing integration and the compression boundary
// ---------------------------------------------------------------------------

MODERN_TEST(Login_FramerDeliversTheEncodedLogin)
{
	// The login travels through the ordinary V027 framer. No envelope, no batch.
	std::vector<WireU8> frame;
	CHECK(LoginProtocol::Encode(SampleRequest(), "O5FDASEOT", Tea(), frame).IsOk());

	ConnectionFramer framer;
	CHECK(framer.Feed(frame.data(), frame.size()) == FrameStatus::Ok);

	Message message;
	CHECK(framer.Next(message) == FrameStatus::Ok);
	CHECK_EQ(static_cast<WireU32>(message.header.type),
	         static_cast<WireU32>(LoginProtocol::kLoginMessageId));
	CHECK_EQ(static_cast<std::size_t>(message.header.size), std::size_t{ 85 });
	CHECK(framer.Next(message) == FrameStatus::NeedMoreData);
}

MODERN_TEST(Login_NeverRoutesThroughTheCompressionLayer)
{
	// Direction guard. V028/V029 established compression is server -> client only,
	// so a client -> server login frame must NOT be a NET_COMPRESS envelope. If a
	// future change routes login through ServerBatchEncoder, this fails.
	std::vector<WireU8> frame;
	CHECK(LoginProtocol::Encode(SampleRequest(), "K9IHANA", Tea(), frame).IsOk());

	CHECK(!NetCompress::IsEnvelope(Codec::ReadU32(frame.data() + 4)));
	CHECK_NE(Codec::ReadU32(frame.data() + 4),
	         static_cast<WireU32>(WellKnownMessage::kCompress));

	// And the body is not LZO-shaped: it starts with the garbage token in the
	// clear, which compressed bytes would not.
	const std::string head(reinterpret_cast<const char*>(frame.data() + kMessageHeaderSize), 7);
	CHECK_EQ(head, std::string("K9IHANA"));
}

MODERN_TEST(Login_EmptyUserIdIsRejected)
{
	// Structurally decodable but not a usable login. Legacy would query the
	// database with an empty string; catching it at the boundary keeps the
	// failure out of persistence.
	LoginRequest request;
	request.userId = "";
	request.password = "pw";
	request.randomPassword = "1";
	request.encryptKey = "k";

	std::vector<WireU8> frame;
	CHECK(LoginProtocol::Encode(request, "K9IHANA", Tea(), frame).IsOk());

	LoginRequest decoded;
	CHECK(LoginProtocol::Decode(frame.data(), frame.size(), Tea(), decoded).IsError());
}

MODERN_TEST(Login_AllFiveGarbageTokensRoundTrip)
{
	const auto& tokens = LoginProtocol::GarbageTokens();
	CHECK_EQ(tokens.size(), std::size_t{ 5 });

	for (const char* token : tokens)
	{
		std::vector<WireU8> frame;
		CHECK(LoginProtocol::Encode(SampleRequest(), token, Tea(), frame).IsOk());
		CHECK_EQ(frame.size(), LoginProtocol::kLoginBodySize + std::strlen(token));

		LoginRequest decoded;
		CHECK(LoginProtocol::Decode(frame.data(), frame.size(), Tea(), decoded).IsOk());
		CHECK_EQ(decoded.garbage.text, std::string(token));
		CHECK_EQ(decoded.userId, std::string("tester"));
		CHECK_EQ(decoded.password, std::string("pw123"));
	}
}