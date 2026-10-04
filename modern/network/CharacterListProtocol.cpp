#include "CharacterListProtocol.h"

#include "NetworkCodec.h"

#include <algorithm>
#include <cstring>

namespace Modern::Network::CharacterListCodec
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

		// The shared header check.
		//
		// Every decode in this namespace starts here, because a frame whose declared
		// size disagrees with its actual length is the single most common way a
		// protocol codec reads past its buffer. `frame.size()` is the authority; the
		// declared size is only ever checked against it, never used to index.
		Status RequireFrame(const std::vector<WireU8>& frame, MessageId expectedId,
		                    std::size_t expectedSize)
		{
			if (frame.size() != expectedSize)
			{
				return Status(ErrorCode::InvalidArgument);
			}

			const WireU32 declaredSize = GetU32(frame, 0);
			const WireU32 declaredType = GetU32(frame, 4);

			if (declaredSize != expectedSize || declaredType != expectedId)
			{
				return Status(ErrorCode::InvalidArgument);
			}
			return Ok();
		}

		// Appends an 8-byte header. Used by every append below so that the size and
		// type fields are written the same way in the same place.
		void AppendHeader(std::vector<WireU8>& out, std::size_t start, MessageId id,
		                  std::size_t size) noexcept
		{
			PutU32(out, start, static_cast<WireU32>(size));
			PutU32(out, start + 4, id);
		}
	}

	bool IsRequestAll(MessageId id) noexcept
	{
		return id == CharacterList::kRequestAllId;
	}

	bool IsIdList(MessageId id) noexcept
	{
		return id == CharacterList::kAllInfoId;
	}

	bool IsRequestOne(MessageId id) noexcept
	{
		return id == CharacterList::kRequestOneId;
	}

	bool IsCharacterDetail(MessageId id) noexcept
	{
		return id == CharacterList::kCharacterDetailId;
	}

	std::size_t ExpectedDetailCount(const CharacterIdList& list) noexcept
	{
		return list.ids.size();
	}

	Status AppendRequestAll(std::vector<WireU8>& out)
	{
		const std::size_t start = out.size();
		out.resize(start + CharacterList::kBareMessageSize, 0);
		AppendHeader(out, start, CharacterList::kRequestAllId,
		             CharacterList::kBareMessageSize);
		return Ok();
	}

	Status AppendIdList(std::vector<WireU8>& out, const std::vector<WireU32>& ids)
	{
		// FIXED WIDTH, not a variable tail.
		//
		// NET_CHA_BBA_INFO's constructor (s_NetGlobal.h:3987-3992) sets
		//   nmg.dwSize = sizeof(NET_CHA_BBA_INFO);
		// and nChaNum[] is a fixed-size array member, so the emitted size does NOT
		// depend on nChaSNum. An account with two characters still sends 28 bytes
		// with two ids followed by two zero slots.
		//
		// This build's width is 28, because no country macro is defined here (see
		// CharacterListProtocol.h). A variable-width encoder would emit 20 bytes for
		// two characters, and a real client reading dwSize=20 against a fixed
		// MAX_ONESERVERCHAR_NUM of 4 would be reading a packet this codebase has no
		// way to produce - i.e. it would be wrong to itself.
		//
		// Refused rather than truncated: silently dropping characters past slot 4
		// would make a five-character account look like a four-character one, which
		// is the exact failure this packet exists to prevent.
		if (ids.size() > CharacterList::kLocalSlots)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		const std::size_t start = out.size();
		out.resize(start + CharacterList::kLocalSize, 0);

		AppendHeader(out, start, CharacterList::kAllInfoId, CharacterList::kLocalSize);
		// nChaSNum is a signed int on the wire. The count is bounded well below
		// INT_MAX by the check above, so the cast cannot lose a bit.
		PutU32(out, start + CharacterList::kListOffsetCount,
		       static_cast<WireU32>(static_cast<WireI32>(ids.size())));

		// The unused slots stay zero: the resize above zeroed them and nothing writes
		// there. That matches legacy, whose constructor memsets the whole struct.
		for (std::size_t i = 0; i < ids.size(); ++i)
		{
			PutU32(out, start + CharacterList::kListOffsetFirstId + i * 4, ids[i]);
		}
		return Ok();
	}

	Status DecodeIdList(const std::vector<WireU8>& frame, CharacterIdList& out)
	{
		out.ids.clear();

		// BOTH widths accepted, and neither preferred.
		//
		// This is the whole reason the id list is decoded by width rather than by a
		// constant: a released Korean server sends 76 bytes and this checkout's own
		// build sends 28, and both are correct for their configuration. Choosing one
		// here would desynchronise the other.
		if (frame.size() != CharacterList::kListSmallSize &&
		    frame.size() != CharacterList::kListReleasedSize)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		if (GetU32(frame, 4) != CharacterList::kAllInfoId)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		const std::size_t declaredSize = GetU32(frame, 0);
		if (declaredSize != frame.size())
		{
			// The header's own size field must agree with the bytes actually present.
			// Trusting either one alone is how a codec ends up reading past its buffer.
			return Status(ErrorCode::InvalidArgument);
		}

		const WireU32 rawCount = GetU32(frame, CharacterList::kListOffsetCount);

		// Read as signed because the field is a native int, and a peer that sends
		// 0xFFFFFFFF must be rejected rather than treated as 4 billion characters.
		const WireI32 count = static_cast<WireI32>(rawCount);
		if (count < 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// The count is the number of REAL characters, which is routinely less than the
		// array's capacity: legacy emits a full-width packet and zero-fills the slots
		// it is not using. So the test is `count <= capacity`, NOT `count == capacity`.
		//
		// The upper bound still has to be enforced. Without it a peer could declare 28
		// bytes with a count of 1000, and a decoder that trusted the count would read
		// a thousand ids out of sixteen bytes of payload.
		const std::size_t capacity =
		    (frame.size() - CharacterList::kListHeaderSize) / 4;
		if (static_cast<std::size_t>(count) > capacity)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// Only the slots the count covers are reported. The remaining zeros are
		// padding, not characters, and returning them would invent characters that
		// do not exist - which is how a client ends up requesting a 2332 for slot 3
		// and being told the character does not exist.
		out.ids.reserve(static_cast<std::size_t>(count));
		for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i)
		{
			out.ids.push_back(
			    GetU32(frame, CharacterList::kListOffsetFirstId + i * 4));
		}
		return Ok();
	}

	Status AppendRequestOne(std::vector<WireU8>& out, WireU32 characterId)
	{
		const std::size_t start = out.size();
		out.resize(start + CharacterList::kRequestOneSize, 0);
		AppendHeader(out, start, CharacterList::kRequestOneId,
		             CharacterList::kRequestOneSize);
		PutU32(out, start + CharacterList::kBareMessageSize, characterId);
		return Ok();
	}

	Status ValidateRequestOne(const std::vector<WireU8>& frame, WireU32& characterId)
	{
		if (const Status status = RequireFrame(frame, CharacterList::kRequestOneId,
		                                       CharacterList::kRequestOneSize);
		    status.IsError())
		{
			return status;
		}
		characterId = GetU32(frame, CharacterList::kBareMessageSize);
		return Ok();
	}

	Status ReadRequestOneId(const std::vector<WireU8>& frame, WireU32& characterId)
	{
		characterId = 0;
		if (const Status status = ValidateRequestOne(frame, characterId); status.IsError())
		{
			return status;
		}
		return Ok();
	}

	Status AppendCharacterDetail(std::vector<WireU8>& out, const CharacterDetail& detail)
	{
		const std::size_t start = out.size();
		out.resize(start + CharacterList::kCharacterDetailSize, 0);

		AppendHeader(out, start, CharacterList::kCharacterDetailId,
		             CharacterList::kCharacterDetailSize);

		const std::size_t payload = start + CharacterList::kBareMessageSize;

		PutU32(out, payload + CharacterList::kDetailOffsetCharacterId,
		       detail.characterId);
		PutU32(out, payload + CharacterList::kDetailOffsetCharacterClass,
		       detail.characterClass);
		PutU16(out, payload + CharacterList::kDetailOffsetSchool, detail.school);
		PutU16(out, payload + CharacterList::kDetailOffsetLevel, detail.level);

		PutU32(out, payload + CharacterList::kDetailOffsetHp + 0, detail.hp.now);
		PutU32(out, payload + CharacterList::kDetailOffsetHp + 4, detail.hp.max);

		PutU32(out, payload + CharacterList::kDetailOffsetSaveMapId,
		       detail.saveMapId.value);

		if (const Status status = RanWire::PutFixedCharField(
		        out, payload + CharacterList::kDetailOffsetName,
		        CharacterList::kNameFieldSize, detail.name);
		    status.IsError())
		{
			// Roll back rather than leave a partial frame for the caller to trip
			// over. The reserved regions were already zero by construction.
			out.resize(start);
			return status;
		}

		// Everything not written above stays zero, because resize() zero-filled and
		// nothing has touched it. That covers the appearance words, experience,
		// brightness, stats, the whole 1056-byte equipment array and scale range.
		return Ok();
	}

	Status DecodeCharacterDetail(const std::vector<WireU8>& frame, CharacterDetail& out)
	{
		out = CharacterDetail{};

		if (const Status status = RequireFrame(frame, CharacterList::kCharacterDetailId,
		                                       CharacterList::kCharacterDetailSize);
		    status.IsError())
		{
			return status;
		}

		const std::size_t payload = CharacterList::kBareMessageSize;

		out.characterId = GetU32(frame, payload + CharacterList::kDetailOffsetCharacterId);
		out.characterClass =
		    GetU32(frame, payload + CharacterList::kDetailOffsetCharacterClass);
		out.school = GetU16(frame, payload + CharacterList::kDetailOffsetSchool);
		out.level  = GetU16(frame, payload + CharacterList::kDetailOffsetLevel);
		out.hp.now = GetU32(frame, payload + CharacterList::kDetailOffsetHp + 0);
		out.hp.max = GetU32(frame, payload + CharacterList::kDetailOffsetHp + 4);
		out.saveMapId.value =
		    GetU32(frame, payload + CharacterList::kDetailOffsetSaveMapId);

		return RanWire::ReadFixedCharField(frame,
		                                    payload + CharacterList::kDetailOffsetName,
		                                    CharacterList::kNameFieldSize, out.name);
	}
}