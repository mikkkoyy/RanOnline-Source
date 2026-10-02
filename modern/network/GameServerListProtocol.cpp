#include "GameServerListProtocol.h"

#include "NetworkCodec.h"

#include <algorithm>
#include <cstring>

namespace Modern::Network
{
	namespace
	{
		void PutU32(std::vector<WireU8>& out, std::size_t offset, WireU32 value)
		{
			for (int i = 0; i < 4; ++i)
			{
				out[offset + static_cast<std::size_t>(i)] =
				    static_cast<WireU8>((value >> (8 * i)) & 0xFFu);
			}
		}

		// Writes a NUL-terminated, zero-padded fixed field.
		//
		// The field is `char[MAX_IP_LENGTH+1]`, so it holds MAX_IP_LENGTH characters
		// plus a terminator. Legacy fills these from a std::string with a plain
		// assignment, so the practical rule is "21 bytes, zero filled, at most 20
		// characters plus a terminator".
		//
		// Unlike the WORLD-002 email/GID fields this one has no StringCchCopy with a
		// surprising capacity argument, so fieldSize - 1 is correct here - and it is
		// stated explicitly rather than assumed, because those two fields PROVED the
		// opposite assumption wrong once already.
		Status PutFixedIpField(std::vector<WireU8>& out, std::size_t offset,
		                       const std::string& value)
		{
			if (value.size() >= GameServerList::kServerIpFieldSize)
			{
				return Status(ErrorCode::InvalidArgument);
			}
			std::fill(out.begin() + static_cast<std::ptrdiff_t>(offset),
			          out.begin() + static_cast<std::ptrdiff_t>(offset + GameServerList::kServerIpFieldSize),
			          static_cast<WireU8>(0));
			if (!value.empty())
			{
				std::memcpy(out.data() + offset, value.data(), value.size());
			}
			return Ok();
		}

		// Reads a NUL-terminated fixed field.
		//
		// A field with no NUL anywhere in its 21 bytes is REJECTED rather than
		// returned whole. Legacy's clients read these with a C string function and
		// would run off the end; refusing is the modern boundary and costs nothing,
		// because the field is zero-filled by construction on the sending side.
		Status ReadFixedIpField(const std::vector<WireU8>& frame, std::size_t offset,
		                        std::string& out)
		{
			const WireU8* begin = frame.data() + offset;
			const void* nul = std::memchr(begin, 0, GameServerList::kServerIpFieldSize);
			if (nul == nullptr)
			{
				return Status(ErrorCode::InvalidArgument);
			}
			out.assign(reinterpret_cast<const char*>(begin),
			           static_cast<const std::size_t>(static_cast<const WireU8*>(nul) - begin));
			return Ok();
		}
	}

	// ---- GameServerList ------------------------------------------------------

	bool GameServerGrid::IsInRange(WireI32 group, WireI32 number) noexcept
	{
		return group >= 0 && group < GameServerList::kMaxServerGroup &&
		       number >= 0 && number < GameServerList::kMaxChannelNumber;
	}

	Status GameServerGrid::Add(const GameServerInfo& info)
	{
		if (!IsInRange(info.serverGroup, info.serverNumber))
		{
			return Status(ErrorCode::InvalidArgument);
		}

		const std::size_t g = static_cast<std::size_t>(info.serverGroup);
		const std::size_t n = static_cast<std::size_t>(info.serverNumber);

		if (!m_occupied[g][n])
		{
			m_occupied[g][n] = true;
			++m_count;
		}
		// Last write wins, matching m_sGame[group][number] = gscil.
		m_grid[g][n] = info;
		return Ok();
	}

	void GameServerGrid::Clear() noexcept
	{
		for (std::size_t g = 0; g < GameServerList::kMaxServerGroup; ++g)
		{
			for (std::size_t n = 0; n < GameServerList::kMaxChannelNumber; ++n)
			{
				m_occupied[g][n] = false;
				m_grid[g][n] = GameServerInfo{};
			}
		}
		m_count = 0;
	}

	bool GameServerGrid::Contains(WireI32 group, WireI32 number) const noexcept
	{
		return IsInRange(group, number) &&
		       m_occupied[static_cast<std::size_t>(group)][static_cast<std::size_t>(number)];
	}

	const GameServerInfo* GameServerGrid::Find(WireI32 group, WireI32 number) const noexcept
	{
		if (!Contains(group, number))
		{
			return nullptr;
		}
		return &m_grid[static_cast<std::size_t>(group)][static_cast<std::size_t>(number)];
	}

	std::vector<GameServerInfo> GameServerGrid::Servers() const
	{
		std::vector<GameServerInfo> out;
		out.reserve(m_count);

		// Canonical order: group ascending, then channel ascending. This is exactly
		// the nested loop the Login Server walks to build the response, so the
		// client's view matches the order the entries arrived in.
		for (std::size_t g = 0; g < GameServerList::kMaxServerGroup; ++g)
		{
			for (std::size_t n = 0; n < GameServerList::kMaxChannelNumber; ++n)
			{
				if (m_occupied[g][n])
				{
					out.push_back(m_grid[g][n]);
				}
			}
		}
		return out;
	}

	// ---- codec ---------------------------------------------------------------

	bool GameServerListCodec::IsRequest(MessageId id) noexcept
	{
		return id == GameServerList::kRequestGameServersId;
	}

	bool GameServerListCodec::IsEntry(MessageId id) noexcept
	{
		return id == GameServerList::kGameServerInfoId;
	}

	bool GameServerListCodec::IsListEnd(MessageId id) noexcept
	{
		return id == GameServerList::kGameServerListEndId;
	}

	bool GameServerListCodec::IsAdvertisable(const GameServerInfo& info) noexcept
	{
		return info.maxClients > 0;
	}

	Status GameServerListCodec::AppendRequest(std::vector<WireU8>& out)
	{
		const std::size_t start = out.size();
		out.resize(start + GameServerList::kBareMessageSize, 0);
		PutU32(out, start + GameServerList::kOffsetSize,
		       static_cast<WireU32>(GameServerList::kBareMessageSize));
		PutU32(out, start + GameServerList::kOffsetType, GameServerList::kRequestGameServersId);
		return Ok();
	}

	Status GameServerListCodec::AppendListEnd(std::vector<WireU8>& out)
	{
		const std::size_t start = out.size();
		out.resize(start + GameServerList::kBareMessageSize, 0);
		PutU32(out, start + GameServerList::kOffsetSize,
		       static_cast<WireU32>(GameServerList::kBareMessageSize));
		PutU32(out, start + GameServerList::kOffsetType, GameServerList::kGameServerListEndId);
		return Ok();
	}

	Status GameServerListCodec::AppendEntry(std::vector<WireU8>& out, const GameServerInfo& info)
	{
		// Rejected before any bytes are written, so a refused entry cannot leave a
		// half-written frame behind for the caller to trip over.
		if (!GameServerGrid::IsInRange(info.serverGroup, info.serverNumber))
		{
			return Status(ErrorCode::InvalidArgument);
		}

		const std::size_t start = out.size();
		out.resize(start + GameServerList::kEntrySize, 0);

		PutU32(out, start + GameServerList::kOffsetSize,
		       static_cast<WireU32>(GameServerList::kEntrySize));
		PutU32(out, start + GameServerList::kOffsetType, GameServerList::kGameServerInfoId);

		if (const Status status = PutFixedIpField(out, start + GameServerList::kOffsetServerIp, info.ip);
		    status.IsError())
		{
			out.resize(start); // roll back
			return status;
		}

		// Every remaining scalar is a signed native int on the wire, written
		// little-endian. bPK is MSVC's 1-byte bool, written as 0 or 1.
		PutU32(out, start + GameServerList::kOffsetServicePort,
		       static_cast<WireU32>(info.servicePort));
		PutU32(out, start + GameServerList::kOffsetServerGroup,
		       static_cast<WireU32>(info.serverGroup));
		PutU32(out, start + GameServerList::kOffsetServerNumber,
		       static_cast<WireU32>(info.serverNumber));
		PutU32(out, start + GameServerList::kOffsetCurrentClients,
		       static_cast<WireU32>(info.currentClients));
		PutU32(out, start + GameServerList::kOffsetMaxClients,
		       static_cast<WireU32>(info.maxClients));
		out[start + GameServerList::kOffsetPk] = info.pk ? static_cast<WireU8>(1)
		                                                  : static_cast<WireU8>(0);

		// Bytes 29-31 and 53-55 are struct padding. Legacy leaves them
		// indeterminate - `NET_CUR_INFO_LOGIN ncil;` is an uninitialised stack local
		// whose constructor sets only nmg.nType and nmg.dwSize. This zero-fills them,
		// which is a deliberate superset: every reader ignores those bytes, and a
		// deterministic frame is what makes the output testable. Same decision, and
		// same reasoning, as WORLD-002's NET_LOGIN_FEEDBACK_DATA.
		return Ok();
	}

	Status GameServerListCodec::DecodeEntry(const std::vector<WireU8>& frame, GameServerInfo& out)
	{
		out = GameServerInfo{};

		if (frame.size() != GameServerList::kEntrySize)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// The declared size and type must agree with the fixed layout. A peer
		// claiming a different length is refused rather than reinterpreted.
		if (Codec::ReadU32(frame.data() + GameServerList::kOffsetSize) !=
		    static_cast<WireU32>(GameServerList::kEntrySize))
		{
			return Status(ErrorCode::InvalidArgument);
		}
		if (!IsEntry(Codec::ReadU32(frame.data() + GameServerList::kOffsetType)))
		{
			return Status(ErrorCode::InvalidArgument);
		}

		if (const Status status = ReadFixedIpField(frame, GameServerList::kOffsetServerIp, out.ip);
		    status.IsError())
		{
			return status;
		}

		out.servicePort = static_cast<WireI32>(Codec::ReadU32(frame.data() + GameServerList::kOffsetServicePort));
		out.serverGroup = static_cast<WireI32>(Codec::ReadU32(frame.data() + GameServerList::kOffsetServerGroup));
		out.serverNumber = static_cast<WireI32>(Codec::ReadU32(frame.data() + GameServerList::kOffsetServerNumber));
		out.currentClients = static_cast<WireI32>(Codec::ReadU32(frame.data() + GameServerList::kOffsetCurrentClients));
		out.maxClients = static_cast<WireI32>(Codec::ReadU32(frame.data() + GameServerList::kOffsetMaxClients));

		const WireU8 pk = frame[GameServerList::kOffsetPk];
		if (pk > 1)
		{
			// Legacy's bool can only be 0 or 1. Anything else is not a value this
			// protocol can express, so it is refused instead of coerced to true.
			return Status(ErrorCode::InvalidArgument);
		}
		out.pk = (pk != 0);
		return Ok();
	}

	Status GameServerListCodec::DecodeBare(const std::vector<WireU8>& frame, MessageId& outId)
	{
		outId = 0;

		if (frame.size() != GameServerList::kBareMessageSize)
		{
			return Status(ErrorCode::InvalidArgument);
		}
		if (Codec::ReadU32(frame.data() + GameServerList::kOffsetSize) !=
		    static_cast<WireU32>(GameServerList::kBareMessageSize))
		{
			return Status(ErrorCode::InvalidArgument);
		}

		outId = Codec::ReadU32(frame.data() + GameServerList::kOffsetType);
		if (!IsRequest(outId) && !IsListEnd(outId))
		{
			return Status(ErrorCode::InvalidArgument);
		}
		return Ok();
	}

	Status GameServerListCodec::ValidateRequest(const std::vector<WireU8>& frame)
	{
		MessageId id = 0;
		if (const Status status = DecodeBare(frame, id); status.IsError())
		{
			return status;
		}
		if (!IsRequest(id))
		{
			return Status(ErrorCode::InvalidArgument);
		}
		return Ok();
	}

	// ---- endpoint ------------------------------------------------------------

	bool EndpointAddress::IsNumericIPv4(const std::string& text) noexcept
	{
		// inet_addr accepts exactly this: four decimal octets, no leading zeros
		// beyond a single digit, nothing else. Hostnames are NOT resolved, because
		// the gethostbyname call in ConnectServer is commented out
		// (s_NetClient.cpp:436-465).
		unsigned octets = 0;
		unsigned digits = 0;
		unsigned value  = 0;

		for (const char c : text)
		{
			if (c >= '0' && c <= '9')
			{
				value = value * 10 + static_cast<unsigned>(c - '0');
				++digits;
				if (digits > 3 || value > 255)
				{
					return false;
				}
			}
			else if (c == '.')
			{
				if (digits == 0)
				{
					return false;
				}
				++octets;
				digits = 0;
				value  = 0;
			}
			else
			{
				return false;
			}
		}

		return octets == 3 && digits != 0;
	}
}
