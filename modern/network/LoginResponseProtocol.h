#pragma once

// WORLD-002: NET_MSG_LOGIN_FB, the server -> client login response.
//
// ===========================================================================
// DIRECTION. This one IS enveloped and compressed.
// ===========================================================================
//
// The V030 boundary applies here and only here. The login REQUEST (WORLD-001)
// is raw; the RESPONSE is not:
//
//   client -> server   raw NET_MSG_GENERIC, one per send, no envelope
//   server -> client   batch -> LZO 1X -> 12-byte NET_COMPRESS envelope
//
// Legacy: CAgentServer::MsgLogInBack (s_CAgentServerMsgLogin.cpp:1087-1324) calls
// SendClient, which queues into CSendMsgBuffer; the flush at
// CClientManager::SendClientFinal (s_CClientManager.cpp:420-433) wraps it. The
// client unwraps in CRcvMsgBuffer::getMsg (RcvMsgBuffer.cpp:112).
//
// ===========================================================================
// THE TWO-STRUCT TRAP
// ===========================================================================
//
// Message id 2050 is referred to by TWO different structs in the legacy tree:
//
//   NET_LOGIN_FEEDBACK_DATA    <- the ONLY one that reaches the client
//   NET_LOGIN_FEEDBACK_DATA2   <- internal, in-process, never transmitted
//
// They are not two versions of one message; they are two different stages.
// CAgentUserCheck::Execute builds a DATA2 from the database result and passes it
// BY POINTER to MsgLogInBack (s_CDbActionUser.cpp:510) - a plain function call,
// not a send. MsgLogInBack then copies selected DATA2 fields into a DATA and
// sends THAT to the client (s_CAgentServerMsgLogin.cpp:1089+).
//
// A modern implementation that transmitted DATA2 would be incompatible. This
// codec encodes DATA.
//
// ===========================================================================
// WIRE LAYOUT - 120 bytes, compiler-verified (x86 MSVC, MBCS)
// ===========================================================================
//
//   offset  size  field               type
//   ------  ----  ------------------  -----------------------------------------
//        0     4  dwSize              u32 LE, always 120
//        4     4  nType               u32 LE, always 2050
//        8    21  szDaumGID           char[21], NUL-padded, MBCS so 1 byte
//       29     1  <padding>           aligns nResult to 30
//       30     2  nResult             u16 LE
//       32     2  uChaRemain          u16 LE
//       34     2  <padding>           aligns nExtremeM to 36
//       36     4  nExtremeM           i32 LE
//       40     4  nExtremeW           i32 LE
//       44     4  nCheckFlag          i32 LE
//       48     4  nPatchProgramVer    i32 LE
//       52     4  nGameProgramVer     i32 LE
//       56     4  dwGameTime          u32 LE
//       60     4  dwPremiumPoint      u32 LE
//       64     4  dwCombatPoint       u32 LE
//       68    51  szEmail             char[51], NUL-padded
//      119     1  <tail padding>
//      ---
//      120        total
//
// Legacy provenance:
//   struct      s_NetGlobal.h:2971-3000 (NET_LOGIN_FEEDBACK_DATA)
//   ids         s_NetGlobal.h:771-772
//   results     s_NetGlobal.h:375-400 (EM_LOGIN_FB_SUB, 0..23)
//   server fill s_CAgentServerMsgLogin.cpp:1087-1324 (MsgLogInBack)
//   client read Lib_ClientUI/Interface/OuterInterfaceMsg.cpp:31-70
//
// ===========================================================================
// ENCRYPTION: NONE. THIS IS THE ASYMMETRY THAT MATTERS MOST.
// ===========================================================================
//
// The REQUEST fields are minTea-encrypted. The RESPONSE is not: MsgLogInBack
// contains no m_Tea call at all, and the client handler casts the received buffer
// straight to NET_LOGIN_FEEDBACK_DATA and reads fields.
//
// This is deliberate asymmetry, not an oversight, and it is why this codec has no
// cipher parameter. The response is CONFIDENTIALITY-FREE by design: it carries
// only a result code and account metadata. Note it DOES contain szEmail, so a
// deployment that considers that sensitive must add transport protection of its
// own - but minTea is not that mechanism, and adding one here would break the
// client.
//
// There is also NO random-password field. The request's plaintext random password
// is validated server-side against the value the server itself issued; it is not
// echoed back. §7's question answered: the response does not use it at all.

#include "NetworkTypes.h"

#include "types/Result.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace Modern::Network
{
	namespace LoginResponse
	{
		// s_NetGlobal.h:771. NET_MSG_LOGIN_FB = NET_MSG_LOBBY + 108.
		constexpr MessageId kLoginFeedbackMessageId = 2050;

		// s_NetGlobal.h:772. Sent when the login succeeded and the client should enter
		// the field stage. Declared so the id is recorded and testable, but NOT sent
		// by this milestone: WORLD-002 implements the 2050 response only, and
		// emitting 2051 from here would be inventing a follow-up handshake.
		constexpr MessageId kLoginFeedbackOkField = 2051;

		// s_NetGlobal.h:237, :170. Field capacities. TCHAR is 1 byte because every
		// original .vcproj sets CharacterSet="2" (MBCS) - V029.
		constexpr std::size_t kDaumGidFieldSize     = 21;  // DAUM_MAX_GID_LENGTH + 1
		constexpr std::size_t kEmailFieldSize       = 51;  // USR_INFOMAIL_LENGTH + 1

		// Maximum CHARACTERS each field carries, which is NOT uniformly fieldSize - 1.
		//
		// StringCchCopy's second argument is the destination BUFFER SIZE IN CHARACTERS,
		// terminator INCLUDED - not the field capacity - so the two call sites in
		// MsgLogInBack cap differently even though the two fields are nearly the
		// same shape:
		//
		//   szEmail[USR_INFOMAIL_LENGTH+1] = 51 bytes
		//       StringCchCopy(szEmail, USR_INFOMAIL_LENGTH, src)      // cch = 50
		//       -> at most 49 characters, terminator at byte 49, byte 50 left as memset
		//       -> s_CAgentServerMsgLogin.cpp:460 and :1161
		//
		//   szDaumGID[DAUM_MAX_GID_LENGTH+1] = 21 bytes
		//       StringCchCopy(szDaumGID, DAUM_MAX_GID_LENGTH+1, src) // cch = 21
		//       -> at most 20 characters, terminator at byte 20
		//       -> s_CAgentServerMsgLogin.cpp:1681, :1870 and :2378
		//
		// Conflating the two would emit an email byte the legacy server never sends.
		constexpr std::size_t kEmailMaxChars   = kEmailFieldSize - 2;    // 49
		constexpr std::size_t kDaumGidMaxChars = kDaumGidFieldSize - 1; // 20

		constexpr std::size_t kMaxChaRemain         = 100; // MAX_CHAR_LENGTH

		// sizeof(NET_LOGIN_FEEDBACK_DATA) on x86, compiler-verified.
		constexpr std::size_t kFeedbackBodySize = 120;

		// Fixed offsets inside the message, header included. These are the padding
		// artefacts of native struct layout and are part of the wire format, so they
		// are stated rather than recomputed.
		constexpr std::size_t kOffsetDaumGid          = 8;
		constexpr std::size_t kOffsetResult           = 30;
		constexpr std::size_t kOffsetChaRemain        = 32;
		constexpr std::size_t kOffsetExtremeM         = 36;
		constexpr std::size_t kOffsetExtremeW         = 40;
		constexpr std::size_t kOffsetCheckFlag        = 44;
		constexpr std::size_t kOffsetPatchProgramVer  = 48;
		constexpr std::size_t kOffsetGameProgramVer   = 52;
		constexpr std::size_t kOffsetGameTime         = 56;
		constexpr std::size_t kOffsetPremiumPoint     = 60;
		constexpr std::size_t kOffsetCombatPoint      = 64;
		constexpr std::size_t kOffsetEmail            = 68;

		static_assert(kOffsetResult == kOffsetDaumGid + kDaumGidFieldSize + 1,
		              "one padding byte must sit between szDaumGID and nResult");
		static_assert(kOffsetExtremeM == kOffsetChaRemain + 2 + 2,
		              "two padding bytes must sit between uChaRemain and nExtremeM");
		static_assert(kFeedbackBodySize == kOffsetEmail + kEmailFieldSize + 1,
		              "one tail padding byte must close the struct at 120");
	}

	// EM_LOGIN_FB_SUB, s_NetGlobal.h:375-400. The full 0..23 set is modelled even
	// though the modern server only emits a few, because the CLIENT must be able to
	// name whatever a RAN server sends it.
	enum class LoginFeedbackResult : std::uint16_t
	{
		Ok             = 0,
		Fail           = 1,
		System          = 2,
		Usage           = 3,
		Duplicate       = 4,
		Incorrect       = 5,   // wrong id or password
		IpBan           = 6,
		Block           = 7,
		Uncon           = 8,   // Daum: not authenticated
		Expired         = 9,   // Daum: subscription expired
		GidError        = 10,
		UidError        = 11,
		Unknown         = 12,
		SsnHead         = 13,
		Adult           = 14,
		ChannelFull     = 15,
		ThaiUnder18Time = 16,
		ThaiUnder18ThreeHour = 17,
		ThaiOver18Time  = 18,
		RandomPass      = 19,  // random-password mismatch
		PassOk          = 20,
		AlreadyOffline  = 21,
		SecIdAlready    = 22,
		RequireTime     = 23,  // login delay
	};

	// Human-readable name, for logs and test failure output. Stable.
	const char* ToString(LoginFeedbackResult result) noexcept;

	// The decoded login response. A value type; the wire form belongs to the codec.
	struct LoginFeedback
	{
		LoginFeedbackResult result = LoginFeedbackResult::Fail;

		// Daum GID. Always zero on the standard KR/TW path - MsgLogInBack never
		// assigns it. Kept because the field exists on the wire and a peer may send it.
		std::string daumGid;

		std::uint16_t chaRemain      = 0;
		std::int32_t  extremeM        = 0;
		std::int32_t  extremeW        = 0;
		std::int32_t  checkFlag       = 0;
		std::int32_t  patchProgramVer = 0;
		std::int32_t  gameProgramVer  = 0;
		std::uint32_t gameTime        = 0;
		std::uint32_t premiumPoint    = 0;
		std::uint32_t combatPoint     = 0;
		std::string   email;

		bool IsSuccess() const noexcept { return result == LoginFeedbackResult::Ok; }
	};

	namespace LoginResponse
	{
		// SERVER. Serializes a response into one complete NET_MSG_GENERIC.
		//
		// The result is written into a caller-supplied buffer at a fixed offset, the
		// because ServerBatchEncoder appends to a batch and knows nothing about
		// message internals. Keeping the append here is what lets the envelope layer
		// stay ignorant of payload layout.
		//
		// Legacy fields MsgLogInBack leaves at zero are left at zero here:
		// szDaumGID and nPatchProgramVer/nGameProgramVer are only populated on the
		// Daum/GSP/Terra paths (s_CAgentServerMsgLogin.cpp:1667, :1858, :2097), never
		// on the KR/TW path.
		Status Append(std::vector<WireU8>& batch, const LoginFeedback& feedback);

		// CLIENT. Parses one complete NET_MSG_LOGIN_FB message.
		//
		// Rejects a wrong type, a wrong size, and a truncated buffer without
		// allocating. Does NOT decrypt: the response is not encrypted.
		Status Decode(const WireU8* frame, std::size_t frameSize, LoginFeedback& out);

		// True when the frame is a LOGIN_FB at all, for dispatch before decoding.
		bool IsLoginFeedback(const WireU8* frame, std::size_t frameSize) noexcept;
	}
}