#include "UpdateStateProtocol.h"

namespace Modern::Network::UpdateState::UpdateStateCodec
{
	namespace
	{
		// The shared header check, the same shape GotoProtocol.cpp uses.
		//
		// `frame.size()` is the authority: a declared size is only ever
		// COMPARED with it, never used to index. That ordering is what
		// stops a peer declaring 82 bytes and being read out of a
		// 21-byte buffer.
		Status RequireFrame(const std::vector<WireU8>& frame, MessageId expectedId,
		                    std::size_t expectedSize)
		{
			if (frame.size() != expectedSize)
			{
				return Status(ErrorCode::InvalidArgument);
			}

			if (Codec::ReadU32(frame.data()) != expectedSize ||
			    Codec::ReadU32(frame.data() + 4) != expectedId)
			{
				return Status(ErrorCode::InvalidArgument);
			}
			return Ok();
		}

		// Writes at an ABSOLUTE offset.
		//
		// Codec::WriteU32 appends, which is the wrong tool here: the frame is
		// `resize`d to its final length before anything is written, so an
		// appending writer would place every field AFTER the frame - a
		// correctly-sized vector of zeros followed by the real bytes, which
		// passes a length check and fails silently in a test that only asserts
		// on size. Same helper and same reason as GotoProtocol.cpp's PutU32.
		void PutU32(std::vector<WireU8>& out, std::size_t offset,
		              WireU32 value) noexcept
		{
			for (int i = 0; i < 4; ++i)
			{
				out[offset + static_cast<std::size_t>(i)] =
				    static_cast<WireU8>((value >> (8 * i)) & 0xFFu);
			}
		}

		// A GLDWDATA on the wire: `dwNow` then `dwMax`, four bytes each
		// (GLDefine.h:400-413). Little-endian by construction, decomposed a
		// byte at a time, so the bytes on the wire do not depend on the
		// host's integer layout.
		void PutPool(std::vector<WireU8>& out, std::size_t offset,
		               const RanWire::DwPair& pool) noexcept
		{
			PutU32(out, offset, pool.now);
			PutU32(out, offset + sizeof(WireU32), pool.max);
		}

		RanWire::DwPair ReadPool(const WireU8* data) noexcept
		{
			RanWire::DwPair pool;
			pool.now = Codec::ReadU32(data);
			pool.max = Codec::ReadU32(data + sizeof(WireU32));
			return pool;
		}

		// `dwSize` then `nType`, four bytes each, and NOTHING else on the
		// wire.
		//
		// Deliberately NOT sizeof(MessageHeader): the struct is padded by the
		// compiler, so its size is not the header's wire length. The same
		// trap put nType at offset 8 instead of 4 in an earlier revision of
		// GotoProtocol.cpp and produced a frame of exactly the right length
		// and entirely wrong.
		constexpr std::size_t kWireSizeOffset   = 0;
		constexpr std::size_t kWireIdOffset     = 4;
		constexpr std::size_t kWireHeaderLength = 8;

		void AppendHeader(std::vector<WireU8>& out, std::size_t start,
		                  MessageId id, std::size_t size) noexcept
		{
			PutU32(out, start + kWireSizeOffset, static_cast<WireU32>(size));
			PutU32(out, start + kWireIdOffset, id);
		}

		// The name field is a fixed CHAR_SZNAME run, zero-filled by the
		// legacy constructor (GLContrlPcMsg.h:963) and by this writer. The
		// bytes after the terminator are the zero fill, which is what the
		// receiver's `strcmp`-based mismatch check expects.
		void PutName(std::vector<WireU8>& out, std::size_t offset,
		               const std::string& name) noexcept
		{
			for (std::size_t i = 0; i < kNameFieldSize; ++i)
			{
				out[offset + i] = i < name.size()
				                        ? static_cast<WireU8>(name[i])
				                        : static_cast<WireU8>(0);
			}
		}

		std::string ReadName(const WireU8* data)
		{
			// Runs to the first zero byte, exactly as the receiver-side
			// mismatch check reads it (GLCharacterMsg.cpp:57 compares with
			// strcmp). A frame with no terminator at all - all 33 bytes
			// non-zero - is a name the field cannot hold, and the codec
			// reports that rather than inventing a terminator.
			std::size_t length = 0;
			while (length < kNameFieldSize && data[length] != 0)
			{
				++length;
			}

			if (length == kNameFieldSize)
			{
				return std::string(reinterpret_cast<const char*>(data),
				                   kNameFieldSize);
			}
			return std::string(reinterpret_cast<const char*>(data), length);
		}
	}

	bool IsStateUpdate(MessageId id) noexcept
	{
		return id == kUpdateStateId;
	}

	bool IsStateBroadcast(MessageId id) noexcept
	{
		return id == kUpdateStateBrdId;
	}

	Status AppendStateUpdate(std::vector<WireU8>& out, const StateUpdate& update)
	{
		// A name the field cannot hold would be silently shortened by a
		// truncating writer, and a shortened name fails the receiver's
		// mismatch check (GLCharacterMsg.cpp:57) for a reason that is not
		// the character's. Refused instead.
		if (update.name.size() >= kNameFieldSize)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		out.resize(out.size() + kUpdateStateSize, 0);

		const std::size_t start = out.size() - kUpdateStateSize;
		AppendHeader(out, start, kUpdateStateId, kUpdateStateSize);

		PutPool(out, start + kUpdateStateHpOffset, update.hp);
		PutPool(out, start + kUpdateStateMpOffset, update.mp);
		PutPool(out, start + kUpdateStateSpOffset, update.sp);
		PutPool(out, start + kUpdateStateCpOffset, update.cp);
		PutName(out, start + kUpdateStateNameOffset, update.name);
		PutU32(out, start + kUpdateStateGaeaIdOffset, update.gaeaId);
		PutU32(out, start + kUpdateStateCharIdOffset, update.charId);
		out[start + kUpdateStateSafeTimeOffset] = update.safeTime ? 1 : 0;
		return Ok();
	}

	Status DecodeStateUpdate(const std::vector<WireU8>& frame, StateUpdate& out)
	{
		out = StateUpdate{};

		if (const Status status =
		        RequireFrame(frame, kUpdateStateId, kUpdateStateSize);
		    status.IsError())
		{
			return status;
		}

		out.hp   = ReadPool(frame.data() + kUpdateStateHpOffset);
		out.mp   = ReadPool(frame.data() + kUpdateStateMpOffset);
		out.sp   = ReadPool(frame.data() + kUpdateStateSpOffset);
		out.cp   = ReadPool(frame.data() + kUpdateStateCpOffset);
		out.name = ReadName(frame.data() + kUpdateStateNameOffset);
		out.gaeaId = Codec::ReadU32(frame.data() + kUpdateStateGaeaIdOffset);
		out.charId = Codec::ReadU32(frame.data() + kUpdateStateCharIdOffset);
		out.safeTime = frame[kUpdateStateSafeTimeOffset] != 0;
		return Ok();
	}

	Status AppendStateBroadcast(std::vector<WireU8>& out,
	                            const StateBroadcast& broadcast)
	{
		out.resize(out.size() + kUpdateStateBrdSize, 0);

		const std::size_t start = out.size() - kUpdateStateBrdSize;
		AppendHeader(out, start, kUpdateStateBrdId, kUpdateStateBrdSize);

		// dwGaeaID FIRST: it comes from SNETPC_BROAD, the base class, and
		// C++ inheritance lays a base class's members out first. The probe
		// measured 8.
		PutU32(out, start + kUpdateStateBrdGaeaIdOffset, broadcast.gaeaId);
		PutPool(out, start + kUpdateStateBrdHpOffset, broadcast.hp);
		out[start + kUpdateStateBrdSafeTimeOffset] =
		    broadcast.safeTime ? 1 : 0;
		return Ok();
	}

	Status DecodeStateBroadcast(const std::vector<WireU8>& frame,
	                            StateBroadcast& out)
	{
		out = StateBroadcast{};

		if (const Status status =
		        RequireFrame(frame, kUpdateStateBrdId, kUpdateStateBrdSize);
		    status.IsError())
		{
			return status;
		}

		out.gaeaId = Codec::ReadU32(frame.data() + kUpdateStateBrdGaeaIdOffset);
		out.hp = ReadPool(frame.data() + kUpdateStateBrdHpOffset);
		out.safeTime = frame[kUpdateStateBrdSafeTimeOffset] != 0;
		return Ok();
	}
}
