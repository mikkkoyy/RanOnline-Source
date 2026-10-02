#pragma once

// WORLD-001: the NET_MSG_LOGIN_2 client -> server protocol.
//
// ===========================================================================
// DIRECTION. THIS IS NOT COMPRESSED AND NOT ENVELOPED.
// ===========================================================================
//
// V028/V029 established that compression is server -> client only. The shipped
// client sends raw NET_MSG_GENERIC, one message per ::send
// (CNetClient::SendBuffer2, s_NetClient.cpp:815-831). Nothing in this file may
// reference NetCompress, Lzo1xCodec or ServerBatchEncoder, and Test
// Login_NeverRoutesThroughTheCompressionLayer enforces that the encoded login
// packet is not a NET_COMPRESS envelope.
//
// ===========================================================================
// WIRE LAYOUT
// ===========================================================================
//
// The logical message is NET_LOGIN_DATA, 76 bytes (compiler-verified in V029,
// x86 MSVC, MBCS). The frame on the wire is that message with a garbage token
// spliced in after the header, which inflates dwSize.
//
//   offset  size  field                     notes
//   ------  ----  ------------------------  --------------------------------
//        0     4  dwSize                    u32 LE, INCLUDES header AND garbage
//        4     4  nType                     u32 LE, always 2049 / 0x0801
//        8     g  garbage token             one of five, g in {6,7,9}
//     8+g     4  nChannel                   i32 LE
//   12+g     7  szRandomPassword           minTea
//   19+g    21  szPassword                 minTea
//   40+g    21  szUserid                   minTea
//   61+g    13  szEnCrypt                  minTea
//   74+g     2  struct padding             indeterminate in legacy, zero here
//        --    --  total dwSize             = 76 + g
//
// Legacy provenance:
//
//   NET_LOGIN_DATA      s_NetGlobal.h:2916-2945   (fields, ctor sets size+type)
//   client send         s_NetClientMsgLogin.cpp:28-68  CNetClient::SndLogin
//   garbage insertion   s_NetClient.cpp:893-922   SendMsgAddGarbageValue
//   token table         RcvMsgBuffer.cpp:11-12    GARBAGE_DATA[]
//   server detection    RcvMsgBuffer.cpp:187-211  SetGarbageNum()
//   server strip        RcvMsgBuffer.cpp:229-249  getOneMsg(bClient = true)
//   server size check   s_CAgentServerMsgLogin.cpp:618
//   server decrypt      s_CAgentServerMsgLogin.cpp:637-644
//
// ---------------------------------------------------------------------------
// ORDER OF OPERATIONS - the whole reason this decoder is shaped like it is
// ---------------------------------------------------------------------------
//
//   TCP receive
//       -> NET_MSG_GENERIC framing (dwSize says 76 + g)
//       -> NET_MSG_LOGIN_2?
//       -> STRIP the garbage token
//       -> validate the remaining body is exactly 76 bytes
//       -> minTea-decrypt the fields
//       -> validate credentials
//
// Validating "76 bytes" BEFORE stripping the garbage rejects every real login,
// because the received body is 76 + g, not 76. Legacy gets this right by accident
// of layering: the strip happens in CNetUser::GetMsg (one layer above the
// handler) and adjusts dwSize, so MsgLogIn's `sizeof(NET_LOGIN_DATA) != dwSize`
// check at :618 compares against an already-corrected 76.
//
// This decoder does the same thing in one place, in the right order, so the
// invariant is visible rather than implied.

#include "MinTeaCodec.h"
#include "NetworkTypes.h"

#include "types/Result.h"

#include <array>
#include <cstddef>
#include <string>
#include <vector>

namespace Modern::Network
{
	namespace LoginProtocol
	{
		// s_NetGlobal.h:770. NET_MSG_LOGIN_2 = NET_MSG_LOBBY + 107 = 2049.
		constexpr MessageId kLoginMessageId = 2049;

		// s_NetGlobal.h:170-237. Field capacities, all +1 for the NUL.
		constexpr std::size_t kUserIdLength        = 20;  // USR_ID_LENGTH
		constexpr std::size_t kPasswordLength      = 20;  // USR_PASS_LENGTH == USR_ID_LENGTH
		constexpr std::size_t kRandomPasswordLength = 6;  // USR_RAND_PASS_LENGTH
		constexpr std::size_t kEncryptKeyLength    = 12;  // ENCRYPT_KEY

		// On-wire field capacities: the +1 NUL byte is transmitted, because the
		// whole fixed field is TEA-processed.
		constexpr std::size_t kUserIdFieldSize        = kUserIdLength + 1;        // 21
		constexpr std::size_t kPasswordFieldSize      = kPasswordLength + 1;      // 21
		constexpr std::size_t kRandomPasswordFieldSize = kRandomPasswordLength + 1; // 7
		constexpr std::size_t kEncryptFieldSize       = kEncryptKeyLength + 1;    // 13

		// sizeof(NET_LOGIN_DATA) on x86: 8 header + 4 nChannel + 7 + 21 + 21 + 13
		// = 74, padded to the struct's 4-byte alignment. The 2 padding bytes are
		// inside dwSize. Legacy leaves them indeterminate; zero is used here.
		constexpr std::size_t kLoginBodySize = 76;

		// Bytes of NET_LOGIN_DATA after the 8-byte header.
		constexpr std::size_t kLoginPayloadSize = kLoginBodySize - kMessageHeaderSize; // 68

		// The garbage tokens, RcvMsgBuffer.cpp:11-12. Lengths 7, 6, 6, 6, 9 - so
		// the set of possible g is {6, 7, 9}, NOT a contiguous range.
		extern const std::array<const char*, 5>& GarbageTokens();
	}

	// One of the five legacy garbage tokens. Chosen by the client, recovered by
	// the server by matching the bytes at offset 8.
	struct GarbageToken
	{
		std::string text;
		std::size_t length = 0;
	};

	// The decoded login request: plaintext credentials plus the values the
	// server needs to answer.
	//
	// Deliberately NOT a wire struct. This is a value type; the wire form is built
	// and taken apart by LoginProtocol only.
	struct LoginRequest
	{
		std::string userId;
		std::string password;
		std::string randomPassword;
		std::string encryptKey;

		WireI32 channel = 0;

		// Which garbage token the client used. Diagnostic, and proves the strip
		// happened rather than being assumed.
		GarbageToken garbage{};
	};

	namespace LoginProtocol
	{
		// ---- client side -----------------------------------------------------

		// Encodes a login request as the exact bytes CNetClient::SndLogin +
		// SendMsgAddGarbageValue would produce.
		//
		// `garbageText` must be one of the five legacy tokens; anything else is
		// rejected, because a token the server cannot recognise produces a packet
		// no RAN server will accept. This models the client's GetGarbageMsg, which
		// only ever picks from the table.
		//
		// `encryptKey` is the pre-shared passphrase the server pushed over the
		// backbone (V028 B.4). It is encrypted like any other field.
		Status Encode(const LoginRequest& request,
		              const std::string& garbageText,
		              const MinTea& tea,
		              std::vector<WireU8>& out);

		// ---- server side -----------------------------------------------------

		// Decodes and validates a complete login frame that has already been
		// framed by ConnectionFramer (i.e. `frame` holds exactly one message and
		// its declared size).
		//
		// Performs, in order: type check -> garbage identify -> strip -> size check
		// -> TEA decrypt. Never allocates on an unvalidated length.
		//
		// `tea` is passed in rather than constructed so the caller owns the key.
		Status Decode(const WireU8* frame, std::size_t frameSize, const MinTea& tea,
		              LoginRequest& out);

		// How many bytes of `password` the server TEA-decrypts.
		//
		// This is the ONE place bFeatureRegisterUseMD5 shows up, and it is worth
		// being precise about what it does. Legacy:
		//
		//   int wPassLength = 0;                              // md5 login = 0
		//   if ( !RANPARAM::bFeatureRegisterUseMD5 ) wPassLength = 1;
		//   m_Tea.decrypt (pNml->szPassword, USR_PASS_LENGTH+wPassLength );
		//
		// (s_CAgentServerMsgLogin.cpp:637-642). It changes the DECRYPT WINDOW
		// 21 -> 20. It does NOT change the struct size, dwSize, the number of
		// bytes transmitted, or the database comparison. The client always
		// encrypts 21 bytes (s_NetClientMsgLogin.cpp:60).
		//
		// It is also harmless: minTea recovers the ciphertext length by scanning
		// back over NULs, so decrypting with a 20-byte window on a field whose
		// ciphertext is 8 bytes behaves identically to a 21-byte window.
		enum class PasswordDecryptWidth : uint8_t
		{
			// USR_PASS_LENGTH + 1 = 21. The legacy default, because
			// bFeatureRegisterUseMD5 defaults to FALSE (RANPARAM_FEATURE.cpp:42).
			Full21 = 0,
			// USR_PASS_LENGTH = 20. Used when bFeatureRegisterUseMD5 is TRUE.
			Truncated20,
		};

		// Decodes with an explicit password decrypt window. See the enum for why
		// this exists and why it does not change the wire format.
		Status Decode(const WireU8* frame, std::size_t frameSize, const MinTea& tea,
		              PasswordDecryptWidth width, LoginRequest& out);

		// True when `frame` is a NET_MSG_LOGIN_2 rather than some other message.
		// Used to dispatch before decoding, mirroring the Agent's switch.
		bool IsLoginMessage(const WireU8* frame, std::size_t frameSize) noexcept;
	}
}