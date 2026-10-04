#include "WorldEntryProtocol.h"

#include <algorithm>
#include <cstring>

namespace Modern::Network::WorldEntryCodec
{
	namespace
	{
		void PutU32(std::vector<WireU8>& out, std::size_t offset, WireU32 value) noexcept
		{
			for (int i = 0; i < 4; ++i)
			{
				out[offset + static_cast<std::size_t>(i)] =
				    static_cast<WireU8>((value >> (8 * i)) & 0xFFu);
			}
		}

		WireU32 GetU32(const std::vector<WireU8>& frame, std::size_t offset) noexcept
		{
			WireU32 value = 0;
			for (int i = 0; i < 4; ++i)
			{
				value |= static_cast<WireU32>(frame[offset + static_cast<std::size_t>(i)])
				         << (8 * i);
			}
			return value;
		}

		void PutU16(std::vector<WireU8>& out, std::size_t offset, WireU16 value) noexcept
		{
			out[offset]     = static_cast<WireU8>(value & 0xFFu);
			out[offset + 1] = static_cast<WireU8>((value >> 8) & 0xFFu);
		}

		WireU16 GetU16(const std::vector<WireU8>& frame, std::size_t offset) noexcept
		{
			return static_cast<WireU16>(frame[offset]) |
			       static_cast<WireU16>(static_cast<WireU16>(frame[offset + 1]) << 8);
		}

		void PutFloat(std::vector<WireU8>& out, std::size_t offset, float value) noexcept
		{
			// Bit-cast rather than convert. RAN puts IEEE-754 floats on the wire, and
			// going through a numeric conversion would be both slower and, for a NaN
			// payload, lossy.
			WireU32 bits = 0;
			static_assert(sizeof(bits) == sizeof(value),
			              "float and uint32 must be the same width for a bit-cast");
			std::memcpy(&bits, &value, sizeof(bits));
			PutU32(out, offset, bits);
		}

		float GetFloat(const std::vector<WireU8>& frame, std::size_t offset) noexcept
		{
			const WireU32 bits = GetU32(frame, offset);
			float          value = 0.0f;
			std::memcpy(&value, &bits, sizeof(value));
			return value;
		}

		// The shared header check. `frame.size()` is the authority; a declared size is
		// only ever compared against it, never used to index.
		Status RequireFrame(const std::vector<WireU8>& frame, MessageId expectedId,
		                    std::size_t expectedSize)
		{
			if (frame.size() != expectedSize)
			{
				return Status(ErrorCode::InvalidArgument);
			}
			if (GetU32(frame, 0) != expectedSize || GetU32(frame, 4) != expectedId)
			{
				return Status(ErrorCode::InvalidArgument);
			}
			return Ok();
		}

		void AppendHeader(std::vector<WireU8>& out, std::size_t start, MessageId id,
		                  std::size_t size) noexcept
		{
			PutU32(out, start, static_cast<WireU32>(size));
			PutU32(out, start + 4, id);
		}
	}

	bool IsGameJoin(MessageId id) noexcept
	{
		return id == WorldEntry::kGameJoinId;
	}

	bool IsFieldRedirect(MessageId id) noexcept
	{
		return id == WorldEntry::kConnectFieldId;
	}

	bool IsFieldIdentity(MessageId id) noexcept
	{
		return id == WorldEntry::kJoinFieldId;
	}

	bool IsSpawn(MessageId id) noexcept
	{
		return id == WorldEntry::kCharacterJoinId;
	}

	bool IsJoinFailure(MessageId id) noexcept
	{
		return id == WorldEntry::kCharacterJoinFbId;
	}

	// ---- 2353 ---------------------------------------------------------------

	Status AppendGameJoin(std::vector<WireU8>& out, WireI32 characterNumber)
	{
		const std::size_t start = out.size();
		out.resize(start + WorldEntry::kGameJoinSize, 0);
		AppendHeader(out, start, WorldEntry::kGameJoinId, WorldEntry::kGameJoinSize);
		PutU32(out, start + WorldEntry::kBareMessageSize,
		       static_cast<WireU32>(characterNumber));
		return Ok();
	}

	Status DecodeGameJoin(const std::vector<WireU8>& frame, WireI32& characterNumber)
	{
		characterNumber = 0;
		if (const Status status = RequireFrame(frame, WorldEntry::kGameJoinId,
		                                       WorldEntry::kGameJoinSize);
		    status.IsError())
		{
			return status;
		}
		characterNumber = static_cast<WireI32>(
		    GetU32(frame, WorldEntry::kBareMessageSize));
		return Ok();
	}

	// ---- 2358 ---------------------------------------------------------------

	Status AppendFieldRedirect(std::vector<WireU8>& out, const FieldRedirect& redirect)
	{
		const std::size_t start = out.size();
		out.resize(start + WorldEntry::kRedirectSize, 0);
		AppendHeader(out, start, WorldEntry::kConnectFieldId,
		             WorldEntry::kRedirectSize);

		const std::size_t body = start + WorldEntry::kBareMessageSize;

		PutU32(out, body + WorldEntry::kRedirectOffsetJoinType - WorldEntry::kBareMessageSize,
		       static_cast<WireU32>(redirect.joinType));
		PutU32(out, body + WorldEntry::kRedirectOffsetGaeaId - WorldEntry::kBareMessageSize,
		       redirect.gaeaId);
		PutU32(out, body + WorldEntry::kRedirectOffsetSlot - WorldEntry::kBareMessageSize,
		       redirect.slotFieldAgent);
		PutU32(out, body + WorldEntry::kRedirectOffsetPort - WorldEntry::kBareMessageSize,
		       static_cast<WireU32>(redirect.servicePort));

		if (const Status status = RanWire::PutFixedCharField(
		        out, start + WorldEntry::kRedirectOffsetIp,
		        WorldEntry::kAddressFieldSize, redirect.fieldIp);
		    status.IsError())
		{
			out.resize(start);
			return status;
		}
		return Ok();
	}

	Status DecodeFieldRedirect(const std::vector<WireU8>& frame, FieldRedirect& out)
	{
		out = FieldRedirect{};

		if (const Status status = RequireFrame(frame, WorldEntry::kConnectFieldId,
		                                       WorldEntry::kRedirectSize);
		    status.IsError())
		{
			return status;
		}

		out.joinType = static_cast<WireI32>(
		    GetU32(frame, WorldEntry::kRedirectOffsetJoinType));
		out.gaeaId = GetU32(frame, WorldEntry::kRedirectOffsetGaeaId);
		out.slotFieldAgent = GetU32(frame, WorldEntry::kRedirectOffsetSlot);
		out.servicePort =
		    static_cast<WireI32>(GetU32(frame, WorldEntry::kRedirectOffsetPort));

		return RanWire::ReadFixedCharField(frame, WorldEntry::kRedirectOffsetIp,
		                                   WorldEntry::kAddressFieldSize, out.fieldIp);
	}

	// ---- 2359 ---------------------------------------------------------------

	Status AppendFieldIdentity(std::vector<WireU8>& out, const FieldIdentity& identity)
	{
		const std::size_t start = out.size();
		out.resize(start + WorldEntry::kIdentitySize, 0);
		AppendHeader(out, start, WorldEntry::kJoinFieldId, WorldEntry::kIdentitySize);

		const std::size_t body = start + WorldEntry::kBareMessageSize;

		PutU32(out, body + WorldEntry::kIdentityOffsetJoinType - WorldEntry::kBareMessageSize,
		       static_cast<WireU32>(identity.joinType));
		PutU32(out, body + WorldEntry::kIdentityOffsetGaeaId - WorldEntry::kBareMessageSize,
		       identity.gaeaId);
		PutU32(out, body + WorldEntry::kIdentityOffsetSlot - WorldEntry::kBareMessageSize,
		       identity.slotFieldAgent);
		PutU16(out, body + WorldEntry::kIdentityOffsetKey - WorldEntry::kBareMessageSize + 0,
		       identity.cryptKey.keyDirection);
		PutU16(out, body + WorldEntry::kIdentityOffsetKey - WorldEntry::kBareMessageSize + 2,
		       identity.cryptKey.key);

		// The fourth byte of the CRYPT_KEY pair is the padding between the two
		// USHORTs... which does not exist, because CRYPT_KEY is two adjacent
		// USHORTs with no interior padding. The four bytes written above are exactly
		// the whole field, and the total is 24. The static_asserts in the header prove
		// it; this comment exists so the byte arithmetic below is not mistaken for an
		// off-by-one.
		return Ok();
	}

	Status DecodeFieldIdentity(const std::vector<WireU8>& frame, FieldIdentity& out)
	{
		out = FieldIdentity{};

		if (const Status status = RequireFrame(frame, WorldEntry::kJoinFieldId,
		                                       WorldEntry::kIdentitySize);
		    status.IsError())
		{
			return status;
		}

		out.joinType = static_cast<WireI32>(
		    GetU32(frame, WorldEntry::kIdentityOffsetJoinType));
		out.gaeaId = GetU32(frame, WorldEntry::kIdentityOffsetGaeaId);
		out.slotFieldAgent = GetU32(frame, WorldEntry::kIdentityOffsetSlot);
		out.cryptKey.keyDirection = GetU16(frame, WorldEntry::kIdentityOffsetKey + 0);
		out.cryptKey.key          = GetU16(frame, WorldEntry::kIdentityOffsetKey + 2);
		return Ok();
	}

	// ---- 2335 ---------------------------------------------------------------

	Status AppendJoinFailure(std::vector<WireU8>& out, WireI32 reason)
	{
		const std::size_t start = out.size();
		out.resize(start + WorldEntry::kJoinFailureSize, 0);
		AppendHeader(out, start, WorldEntry::kCharacterJoinFbId,
		             WorldEntry::kJoinFailureSize);
		PutU32(out, start + WorldEntry::kBareMessageSize, static_cast<WireU32>(reason));
		return Ok();
	}

	Status DecodeJoinFailure(const std::vector<WireU8>& frame, WireI32& reason)
	{
		reason = 0;
		if (const Status status = RequireFrame(frame, WorldEntry::kCharacterJoinFbId,
		                                       WorldEntry::kJoinFailureSize);
		    status.IsError())
		{
			return status;
		}
		reason = static_cast<WireI32>(GetU32(frame, WorldEntry::kBareMessageSize));
		return Ok();
	}

	// ---- 2333 ---------------------------------------------------------------

	Status AppendSpawn(std::vector<WireU8>& out, const SpawnState& spawn)
	{
		const std::size_t start = out.size();
		out.resize(start + WorldEntry::kSpawnSize, 0);
		AppendHeader(out, start, WorldEntry::kCharacterJoinId, WorldEntry::kSpawnSize);

		const std::size_t base = start + WorldEntry::kSpawnOffsetUserId;

		PutU32(out, base + (WorldEntry::kSpawnOffsetClientId - WorldEntry::kSpawnOffsetUserId),
		       spawn.clientId);
		PutU32(out, base + (WorldEntry::kSpawnOffsetGaeaId - WorldEntry::kSpawnOffsetUserId),
		       spawn.gaeaId);
		PutU32(out, base + (WorldEntry::kSpawnOffsetMapId - WorldEntry::kSpawnOffsetUserId),
		       spawn.mapId.value);
		PutFloat(out, base + (WorldEntry::kSpawnOffsetPosition - WorldEntry::kSpawnOffsetUserId) + 0,
		         spawn.position.x);
		PutFloat(out, base + (WorldEntry::kSpawnOffsetPosition - WorldEntry::kSpawnOffsetUserId) + 4,
		         spawn.position.y);
		PutFloat(out, base + (WorldEntry::kSpawnOffsetPosition - WorldEntry::kSpawnOffsetUserId) + 8,
		         spawn.position.z);

		PutU32(out, base + (WorldEntry::kSpawnOffsetStartMapId - WorldEntry::kSpawnOffsetUserId),
		       spawn.startMapId.value);
		PutU32(out, base + (WorldEntry::kSpawnOffsetStartGate - WorldEntry::kSpawnOffsetUserId),
		       spawn.startGate);

		// The record. Its offsets are relative to the record's own start, which is
		// kSpawnOffsetData in packet terms and 0 in record terms - hence the
		// subtraction on every line below.
		const std::size_t record = start + WorldEntry::kSpawnOffsetData;

		PutU32(out, record + WorldEntry::kRecordOffsetAccountId, spawn.accountId);
		PutU32(out, record + WorldEntry::kRecordOffsetCharacterId, spawn.characterId);
		PutU32(out, record + WorldEntry::kRecordOffsetCharacterClass,
		       spawn.characterClass);
		PutU16(out, record + WorldEntry::kRecordOffsetSchool, spawn.school);
		PutU16(out, record + WorldEntry::kRecordOffsetLevel, spawn.level);

		PutU32(out, record + WorldEntry::kRecordOffsetHp + 0, spawn.hp.now);
		PutU32(out, record + WorldEntry::kRecordOffsetHp + 4, spawn.hp.max);
		PutU32(out, record + WorldEntry::kRecordOffsetMp + 0, spawn.mp.now);
		PutU32(out, record + WorldEntry::kRecordOffsetMp + 4, spawn.mp.max);
		PutU32(out, record + WorldEntry::kRecordOffsetSp + 0, spawn.sp.now);
		PutU32(out, record + WorldEntry::kRecordOffsetSp + 4, spawn.sp.max);

		if (const Status status =
		        RanWire::PutFixedCharField(out, start + WorldEntry::kSpawnOffsetUserId,
		                                  WorldEntry::kUserIdFieldSize, spawn.userId);
		    status.IsError())
		{
			out.resize(start);
			return status;
		}

		if (const Status status = RanWire::PutFixedCharField(
		        out, record + WorldEntry::kRecordOffsetName, WorldEntry::kNameFieldSize,
		        spawn.characterName);
		    status.IsError())
		{
			out.resize(start);
			return status;
		}

		// Everything not written stays zero, which covers the 288 bytes of quick
		// slots, the 44 bytes of counts, the cosmetics, the last-call map and
		// position, the tracing flag and the cafe-class fields - and the reserved
		// runs inside the 600-byte record.
		return Ok();
	}

	Status DecodeSpawn(const std::vector<WireU8>& frame, SpawnState& out)
	{
		out = SpawnState{};

		if (const Status status = RequireFrame(frame, WorldEntry::kCharacterJoinId,
		                                       WorldEntry::kSpawnSize);
		    status.IsError())
		{
			return status;
		}

		if (const Status status = RanWire::ReadFixedCharField(
		        frame, WorldEntry::kSpawnOffsetUserId, WorldEntry::kUserIdFieldSize,
		        out.userId);
		    status.IsError())
		{
			return status;
		}

		out.clientId = GetU32(frame, WorldEntry::kSpawnOffsetClientId);
		out.gaeaId   = GetU32(frame, WorldEntry::kSpawnOffsetGaeaId);
		out.mapId.value = GetU32(frame, WorldEntry::kSpawnOffsetMapId);

		out.position.x = GetFloat(frame, WorldEntry::kSpawnOffsetPosition + 0);
		out.position.y = GetFloat(frame, WorldEntry::kSpawnOffsetPosition + 4);
		out.position.z = GetFloat(frame, WorldEntry::kSpawnOffsetPosition + 8);

		out.startMapId.value = GetU32(frame, WorldEntry::kSpawnOffsetStartMapId);
		out.startGate        = GetU32(frame, WorldEntry::kSpawnOffsetStartGate);

		const std::size_t record = WorldEntry::kSpawnOffsetData;

		out.accountId     = GetU32(frame, record + WorldEntry::kRecordOffsetAccountId);
		out.characterId   = GetU32(frame, record + WorldEntry::kRecordOffsetCharacterId);
		out.characterClass = GetU32(frame, record + WorldEntry::kRecordOffsetCharacterClass);
		out.school = GetU16(frame, record + WorldEntry::kRecordOffsetSchool);
		out.level  = GetU16(frame, record + WorldEntry::kRecordOffsetLevel);

		out.hp.now = GetU32(frame, record + WorldEntry::kRecordOffsetHp + 0);
		out.hp.max = GetU32(frame, record + WorldEntry::kRecordOffsetHp + 4);
		out.mp.now = GetU32(frame, record + WorldEntry::kRecordOffsetMp + 0);
		out.mp.max = GetU32(frame, record + WorldEntry::kRecordOffsetMp + 4);
		out.sp.now = GetU32(frame, record + WorldEntry::kRecordOffsetSp + 0);
		out.sp.max = GetU32(frame, record + WorldEntry::kRecordOffsetSp + 4);

		return RanWire::ReadFixedCharField(frame, record + WorldEntry::kRecordOffsetName,
		                                   WorldEntry::kNameFieldSize,
		                                   out.characterName);
	}
}