#pragma once

// LOGIN-001: the Login Server game-server-list protocol.
//
// This is the PRE-LOGIN phase and it is a different conversation from the Agent
// login implemented by WORLD-001/WORLD-002. Legacy makes the distinction physical:
// the client dispatches on m_nClientNetState, and NET_STATE_LOGIN (1) routes to
// MessageProcessLogin while NET_STATE_AGENT (2) routes to MessageProcessGame
// (s_NetClientMsg.cpp:19-40). The two never share a handler.
//
//   Login Server                              Agent Server
//   ------------------------------            --------------------------
//   REQ_GAME_SVR      1542  (bare 8 bytes)   LOGIN_2   2049
//   SND_GAME_SVR      1552  (56 bytes each)  LOGIN_FB  2050
//   SND_GAME_SVR_END  1562  (bare 8 bytes)
//
// ---------------------------------------------------------------------------
// COMPRESSION: PROVEN ABSENT. This is the one place this milestone contradicts
// the expectation set by WORLD-002, and the contradiction is deliberate.
// ---------------------------------------------------------------------------
//
// The Agent's login response IS compressed. CAgentServer::SendClient calls
// m_pClientManager->SendClient (s_CAgentServer.cpp:712), which batches through
// CNetUser::addSendMsg -> CSendMsgBuffer and emits a NET_COMPRESS envelope
// (s_CClientManager.cpp:437-487, SendMsgBuffer.cpp:146-159).
//
// The Login Server's list is NOT. CLoginServer::SendClient calls
// m_pClientManager->SendClient2 (s_CLoginServer.cpp:827), and SendClient2 copies
// dwSize bytes straight into the IO buffer with no batching and no envelope
// (s_CClientManager.cpp:489-527). CClientLogin inherits SendClient2 unmodified -
// there is no override - and addSendMsg is reached ONLY from
// CClientManager::SendClient. The Login Server never calls it.
//
// So the list travels RAW: a plain concatenation of NET_MSG_GENERIC frames on one
// TCP stream, exactly like the WORLD-001 login request travels raw in the other
// direction. Routing it through the V030 compression layer would produce bytes no
// RAN client accepts, so this milestone reuses V030's framing (ConnectionFramer)
// and deliberately does NOT reuse its compression.
//
// ---------------------------------------------------------------------------
// ENCRYPTION: none. ConnectLoginServer uses the three-argument ConnectServer with
// no CRYPT_KEY, unlike ConnectFieldServer/ConnectBoardServer which pass m_ck
// (s_NetClient.cpp:367-372, :395-418).
// ---------------------------------------------------------------------------

#include "NetworkTypes.h"
#include "types/Result.h"

#include <cstdint>
#include <string>
#include <vector>

namespace Modern::Network
{
	namespace GameServerList
	{
		// s_NetGlobal.h:734-736. All three derive from NET_MSG_LGIN
		// (= NET_MSG_BASE + 507 = 1442), which NetworkTypes.h already carries as
		// Protocol::kLoginBase, so the arithmetic is checked rather than restated.
		constexpr MessageId kRequestGameServersId = Protocol::kLoginBase + 100; // 1542
		constexpr MessageId kGameServerInfoId     = Protocol::kLoginBase + 110; // 1552
		constexpr MessageId kGameServerListEndId  = Protocol::kLoginBase + 120; // 1562

		static_assert(kRequestGameServersId == 1542, "REQ_GAME_SVR is 1542");
		static_assert(kGameServerInfoId == 1552, "SND_GAME_SVR is 1552");
		static_assert(kGameServerListEndId == 1562, "SND_GAME_SVR_END is 1562");

		// s_NetGlobal.h:143 and :150. The grid the server walks and the client
		// indexes.
		//
		// The client array is declared [MAX_SERVER_GROUP][MAX_CHANNEL_NUMBER]
		// (s_NetClient.h:153) but SndReqServerInfo clears it using
		// MAX_SERVER_NUMBER (s_NetClientMsg.cpp:319). Both are 10, so the two
		// spellings agree in value and the discrepancy is cosmetic - recorded
		// because it would become a real bug if either constant ever changed.
		constexpr std::int32_t kMaxServerGroup    = 20;
		constexpr std::int32_t kMaxChannelNumber  = 10;
		constexpr std::size_t   kMaxServers        =
		    static_cast<std::size_t>(kMaxServerGroup) * static_cast<std::size_t>(kMaxChannelNumber);

		// s_NetGlobal.h:146 - MAX_IP_LENGTH. The field is MAX_IP_LENGTH + 1.
		constexpr std::size_t kServerIpFieldSize = 21;

		// sizeof(NET_CUR_INFO_LOGIN), compiler-verified on x86 MSVC/MBCS with a
		// throwaway probe over verbatim struct copies. The entry is a fixed 56
		// bytes with two padding runs.
		constexpr std::size_t kEntrySize = 56;

		// Field offsets, all measured, none inferred from a diagram.
		constexpr std::size_t kOffsetSize          = 0;  // 4  dwSize = 56
		constexpr std::size_t kOffsetType          = 4;  // 4  nType  = 1552
		constexpr std::size_t kOffsetServerIp      = 8;  // 21 szServerIP
		// bytes 29-31 are padding: szServerIP ends at 28 and the next int needs
		// 4-byte alignment.
		constexpr std::size_t kOffsetServicePort      = 32;
		constexpr std::size_t kOffsetServerGroup      = 36;
		constexpr std::size_t kOffsetServerNumber     = 40;
		constexpr std::size_t kOffsetCurrentClients   = 44;
		constexpr std::size_t kOffsetMaxClients       = 48;
		constexpr std::size_t kOffsetPk               = 52; // 1 byte
		// bytes 53-55 are padding to the struct's 4-byte alignment.

		static_assert(kOffsetServerIp + kServerIpFieldSize == 29,
		              "szServerIP occupies bytes 8-28");
		static_assert(kOffsetServicePort == 32, "nServicePort is 3-byte aligned to 32");
		static_assert(kOffsetPk + 4 == kEntrySize, "bPK is followed by 3 bytes of padding");

		// The request and the terminator are both bare NET_MSG_GENERIC: an 8-byte
		// header and no body whatsoever.
		//
		// The terminator carries NO count. The client must therefore either track
		// "have I seen END" or read until END - it cannot know the length in
		// advance. MsgGameSvrInfoEnd does exactly that, setting a flag
		// (s_NetClientMsg.cpp:183-186).
		constexpr std::size_t kBareMessageSize = 8;

		// G_SERVER_CUR_INFO_LOGIN::bPK defaults to true in the constructor
		// (s_NetGlobal.h:577-587), and the server copies whole structs, so a
		// freshly built entry reports PK unless told otherwise.
		constexpr bool kDefaultPk = true;
	}

	// ---------------------------------------------------------------------------
	// Modern application model
	// ---------------------------------------------------------------------------

	// One game server, in modern terms.
	//
	// Deliberately NOT a mirror of G_SERVER_CUR_INFO_LOGIN: the wire struct's
	// padding, its fixed char[21] and its `bool` are transport concerns and do not
	// belong in the model the rest of the modern client reasons about. Every field
	// name here comes from the legacy struct's own comment, not from a guess:
	//
	//   char szServerIP[MAX_IP_LENGTH+1];  ///< Server IP Address
	//   int  nServicePort;                 ///< Server Port
	//   int  nServerGroup;                 ///< Server Group Number
	//   int  nServerNumber;                ///< Channel Number or Server Number
	//   int  nServerCurrentClient;         ///< Channel Current Client
	//   int  nServerMaxClient;             ///< Channel Max Client
	//   bool bPK;                           ///< Channel PK information
	//
	// There is no server NAME on the wire. The list identifies a server by IP and
	// port alone; a display name is a client-side concern RAN resolves separately.
	// All five ints are signed on the wire (native `int`), so they are WireI32 here.
	struct GameServerInfo
	{
		// Dotted-quad IPv4, ASCII. See EndpointAddress below for why this cannot be
		// a hostname.
		std::string ip;

		WireI32 servicePort    = 0;
		WireI32 serverGroup    = 0;
		WireI32 serverNumber   = 0;
		WireI32 currentClients = 0;
		WireI32 maxClients     = 0;
		bool    pk             = GameServerList::kDefaultPk;

		bool operator==(const GameServerInfo& other) const noexcept
		{
			return ip == other.ip && servicePort == other.servicePort &&
			       serverGroup == other.serverGroup && serverNumber == other.serverNumber &&
			       currentClients == other.currentClients && maxClients == other.maxClients &&
			       pk == other.pk;
		}
	};

	// The game-server list, modelled the way legacy actually stores it.
	//
	// Legacy does NOT keep a list. Both ends use a sparse 2-D array indexed by
	// (group, number):
	//
	//   server  s_CLoginServerMsg.cpp:121-134  walks m_sGame[group][channel]
	//   client  s_NetClientMsg.cpp:196-207     writes m_sGame[group][number]
	//
	// which has three observable consequences this type reproduces rather than
	// smooths over:
	//
	//   1. An entry whose group or number is out of range is DROPPED by the client,
	//      silently and individually (s_NetClientMsg.cpp:199-203). One bad entry
	//      does not abort the list.
	//   2. Two entries for the same (group, number) COLLIDE; the last one wins.
	//   3. Order is grid order - group-major, then channel - not arrival order and
	//      not sorted. Iterating the grid reproduces the server's send order exactly.
	//
	// A plain std::vector would get all three wrong, which is why this is a grid.
	class GameServerGrid
	{
	public:
		// Stores an entry at (serverGroup, serverNumber).
		//
		// Returns InvalidArgument and stores nothing when the indices are outside
		// the grid - the modern equivalent of legacy's silent drop, made visible so
		// a caller can count the rejection instead of watching an entry vanish.
		Status Add(const GameServerInfo& info);

		// Every occupied cell in canonical order: group ascending, then channel
		// ascending. Dense, so a caller never has to know the grid exists.
		std::vector<GameServerInfo> Servers() const;

		// Occupied cells, without materialising the vector.
		std::size_t Count() const noexcept { return m_count; }

		void Clear() noexcept;

		// True when (group, number) names a cell that holds an entry.
		bool Contains(WireI32 group, WireI32 number) const noexcept;

		const GameServerInfo* Find(WireI32 group, WireI32 number) const noexcept;

		// True when the indices address a real cell, occupied or not. This is the
		// test legacy performs in MsgGameSvrInfo.
		static bool IsInRange(WireI32 group, WireI32 number) noexcept;

	private:
		GameServerInfo m_grid[GameServerList::kMaxServerGroup][GameServerList::kMaxChannelNumber];
		bool           m_occupied[GameServerList::kMaxServerGroup][GameServerList::kMaxChannelNumber] = {};
		std::size_t    m_count = 0;
	};

	// ---------------------------------------------------------------------------
	// Codec
	// ---------------------------------------------------------------------------

	// Explicit little-endian serialisation of the three messages above.
	//
	// No native struct is ever memcpy'd, so the guarantee that the bytes match
	// legacy stops depending on the compiler's layout rules.
	namespace GameServerListCodec
	{
		// REQ_GAME_SVR: exactly 8 bytes, dwSize = 8, nType = 1542, no body.
		//
		// PROVEN by CNetClient::SndReqServerInfo (s_NetClientMsg.cpp:328-333),
		// which declares a bare NET_MSG_GENERIC, sets dwSize = sizeof(NET_MSG_GENERIC)
		// and nType = NET_MSG_REQ_GAME_SVR, then Sends nSize bytes.
		Status AppendRequest(std::vector<WireU8>& out);

		// SND_GAME_SVR: one 56-byte entry, APPENDED (not replaced), so a caller can
		// build a stream of entries followed by the terminator.
		Status AppendEntry(std::vector<WireU8>& out, const GameServerInfo& info);

		// SND_GAME_SVR_END: a bare 8-byte NET_MSG_GENERIC with no count.
		Status AppendListEnd(std::vector<WireU8>& out);

		// Decodes one SND_GAME_SVR entry from a complete message frame.
		//
		// `frame` must include the 8-byte header, because the entry's dwSize and
		// nType live in it. Rejects, without allocating: a short frame, a wrong
		// nType, a dwSize that disagrees with the real layout, and an IP field that
		// is not NUL-terminated inside its 21 bytes.
		Status DecodeEntry(const std::vector<WireU8>& frame, GameServerInfo& out);

		// Decodes a bare REQ_GAME_SVR / SND_GAME_SVR_END frame and reports which.
		//
		// The terminator MUST be exactly 8 bytes: a longer frame carrying the same
		// id is not a terminator legacy would have sent, so accepting it would let
		// a peer invent a body the protocol has no meaning for.
		Status DecodeBare(const std::vector<WireU8>& frame, MessageId& outId);

		bool IsRequest(MessageId id) noexcept;
		bool IsEntry(MessageId id) noexcept;
		bool IsListEnd(MessageId id) noexcept;

		// Validates an inbound REQ_GAME_SVR: right id, and exactly 8 bytes.
		//
		// The Login Server handler answers ANY message that reaches MsgProcess with
		// this id regardless of dwSize (s_CLoginServerMsg.cpp:36-38), so this check
		// is modern hardening. Extra trailing bytes are refused rather than ignored.
		Status ValidateRequest(const std::vector<WireU8>& frame);

		// The server-side filter, exposed so both ends agree.
		//
		// CLoginServer::MsgSndGameSvrInfo emits an entry only when
		// nServerMaxClient > 0 (s_CLoginServerMsg.cpp:125). A server that is
		// present but advertises no capacity is not offered to the player.
		bool IsAdvertisable(const GameServerInfo& info) noexcept;
	}

	// A Login Server endpoint, as legacy models it.
	//
	// The address is a NUMERIC dotted-quad and cannot be a hostname. Legacy calls
	// ::inet_addr(szServerIP) directly (s_NetClient.cpp:474); the gethostbyname
	// branch immediately above it is commented out, so a name would silently
	// become 0xFFFFFFFF rather than resolve. Encoding that as a validation rule
	// turns a silent misconnection into a diagnosable one.
	struct EndpointAddress
	{
		std::string ip;
		std::uint16_t port = 0;

		// True only for a well-formed dotted-quad with every octet 0-255.
		static bool IsNumericIPv4(const std::string& text) noexcept;
	};
}
