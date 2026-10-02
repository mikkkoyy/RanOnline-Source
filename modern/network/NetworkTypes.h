#pragma once

// VERTICAL-027: wire-level types for the modern network boundary.
//
// This header owns the protocol's numeric vocabulary and nothing else. It has
// no gameplay meaning: nothing here knows about Character, Stats, Damage, HP
// or Skills, and nothing here may be made to.
//
// Legacy provenance (all CONFIRMED by source inspection):
//
//   NET_MSG_GENERIC   Lib_Network/s_NetGlobal.h:2635
//       struct NET_MSG_GENERIC { DWORD dwSize; EMNET_MSG nType; };  // 8 bytes
//
//   NET_COMPRESS      Lib_Network/s_NetGlobal.h:2811
//       struct NET_COMPRESS { NET_MSG_GENERIC nmg; bool bCompress; };
//
//   Message ID space  Lib_Network/s_NetGlobal.h:645-696
//       NET_MSG_BASE = 992 for EVERY country macro, then
//       NET_MSG_LGIN        = BASE + 450
//       NET_MSG_LOBBY       = BASE + 950
//       NET_MSG_LOBBY_MAX   = BASE + 1450
//       NET_MSG_GCTRL       = BASE + 1900
//
//   Buffer sizes      Lib_Network/s_NetGlobal.h:105-111
//       NET_DATA_BUFSIZE          = 2048
//       NET_DATA_MSG_BUFSIZE      = 8192
//       NET_DATA_CLIENT_MSG_BUFSIZE= 16384
//       NET_MAX_CLIENT            = 1000
//       NET_TIME_OUT              = 180000 (ms, i.e. 3 minutes)
//
//   Send framing      Lib_Network/SendMsgBuffer.h:33-42
//       BUFFER_SIZE          = 6144
//       MAX_PACKET_SIZE      = 2048
//       COMPRESS_PACKET_SIZE = 1000
//
//   Server roles      Lib_Network/s_NetGlobal.h:82-86
//       1 Login, 2 Session, 3 Field, 4 Agent
//
// IMPORTANT - byte order and layout, and why this header does NOT memcpy.
//
// Legacy declares these as native structs and copies them wholesale. On x86
// that happens to be little-endian with 4-byte DWORDs and a 1-byte MSVC `bool`,
// so `sizeof(NET_MSG_GENERIC) == 8` on the wire. That is a property of one
// compiler on one CPU, not a protocol guarantee, and it silently breaks the
// moment anyone reads the bytes with a different alignment, a different
// compiler, or a debugger.
//
// So the modern codec ENCODES and DECODES fields explicitly, little-endian,
// using fixed-width types. The resulting bytes are identical to legacy's on
// x86 - which keeps wire compatibility - but they are produced by code rather
// than by struct layout, so the guarantee is now explicit and testable rather
// than accidental.
//
// See docs/reference/server/VERTICAL-027_NETWORK_BOUNDARY_FOUNDATION.md.

#include <cstddef>
#include <cstdint>

namespace Modern::Network
{
	// -------------------------------------------------------------------------
	// Wire primitives.
	//
	// Fixed-width and explicitly sized. `int`/`long` are never used for anything
	// that reaches the wire, because their width is platform-defined.
	// -------------------------------------------------------------------------
	using WireU8  = std::uint8_t;
	using WireU16 = std::uint16_t;
	using WireU32 = std::uint32_t;
	using WireU64 = std::uint64_t;
	using WireI8  = std::int8_t;
	using WireI16 = std::int16_t;
	using WireI32 = std::int32_t;
	using WireI64 = std::int64_t;

	// A protocol message id. Kept as a distinct type so a bare integer cannot be
	// passed where an id is expected - the single most likely source of a
	// silent framing bug in a system with ~800 message constants.
	using MessageId = WireU32;

	// -------------------------------------------------------------------------
	// Protocol constants, transcribed from legacy.
	//
	// These are INHERRED wire values. Changing one changes the protocol, so each
	// carries its legacy location and none may be "tidyed".
	// -------------------------------------------------------------------------
	namespace Protocol
	{
		// s_NetGlobal.h:645-669. The country #if chain assigns 992 in EVERY
		// branch - CH_PARAM, HK_PARAM, ID_PARAM, JP_PARAM, KR_PARAM, KRT_PARAM,
		// MY_PARAM, MYE_PARAM, PH_PARAM, VN_PARAM, TW_PARAM, TH_PARAM, GS_PARAM
		// and the #else. Message ids therefore do NOT vary by region, which is a
		// deliberate property worth keeping: it means the V026 country-macro
		// question does not reach the protocol.
		constexpr MessageId kMessageBase = 992;

		// s_NetGlobal.h:692-696. Three id ranges layered on kMessageBase.
		constexpr MessageId kLoginBase  = kMessageBase + 450;   // 1442
		constexpr MessageId kLobbyBase  = kMessageBase + 950;   // 1942
		constexpr MessageId kLobbyMax  = kMessageBase + 1450;  // 2442
		constexpr MessageId kGCtrlBase  = kMessageBase + 1900;  // 2892

		// s_NetGlobal.h:105-111.
		constexpr std::size_t kDataBufferSize           = 2048;
		constexpr std::size_t kDataMessageBufferSize    = 8192;
		constexpr std::size_t kDataClientMessageBufferSize = 16384;
		constexpr std::size_t kMaxClients               = 1000;
		constexpr std::size_t kTimeoutMilliseconds      = 180000;

		// SendMsgBuffer.h:36-38.
		constexpr std::size_t kSendBufferSize    = 6144;
		constexpr std::size_t kMaxPacketSize     = 2048;
		constexpr std::size_t kCompressThreshold = 1000;

		// s_NetGlobal.h:205. The length of the symmetric crypt key material,
		// excluding its terminator.
		constexpr std::size_t kEncryptKeyLength = 12;
	}

	// -------------------------------------------------------------------------
	// The wire header. Exactly 8 bytes: dwSize (u32), nType (u32).
	//
	// `size` is the TOTAL message size INCLUDING this header - legacy sets it
	// from `sizeof(StructName)` in every constructor (s_NetGlobal.h:2645 and
	// throughout), and RcvMsgBuffer.cpp compares `m_nRcvSize < pNmg->dwSize`
	// to decide whether a whole message has arrived. Getting that off-by-header
	// wrong is the classic framing bug, so it is stated rather than implied.
	// -------------------------------------------------------------------------
	struct MessageHeader
	{
		WireU32      size = 0;  // total bytes, header included
		MessageId    type = 0;
	};

	constexpr std::size_t kMessageHeaderSize = 8;

	static_assert(kMessageHeaderSize == sizeof(WireU32) + sizeof(MessageId),
	              "The RAN header is two 32-bit fields and nothing else");

	// -------------------------------------------------------------------------
	// Legacy server roles (s_NetGlobal.h:82-86).
	//
	// Recorded so the modern layer can talk about "which server" without
	// inventing a numbering of its own.
	// -------------------------------------------------------------------------
	enum class LegacyServerRole : WireU32
	{
		Login   = 1,
		Session = 2,
		Field   = 3,
		Agent   = 4,
	};

	// The handshake ids the connection layer needs before anything else
	// (s_NetGlobal.h:703-723).
	namespace WellKnownMessage
	{
		constexpr MessageId kVersionOk             = 100;
		constexpr MessageId kVersionInfo           = 110;
		constexpr MessageId kVersionReq            = 120;
		constexpr MessageId kRequestCryptKey       = 130;
		constexpr MessageId kSendCryptKey          = 140;
		constexpr MessageId kHeartbeatServerReq    = 153;
		constexpr MessageId kHeartbeatServerAns    = 154;
		constexpr MessageId kServerDown            = 155;
		constexpr MessageId kServerCloseClient     = 156;
		constexpr MessageId kHeartbeatClientReq    = 160;
		constexpr MessageId kHeartbeatClientAns    = 161;
		constexpr MessageId kCompress              = 170;
	}
}
