#include "MovementStateProtocol.h"

namespace Modern::Network::MovementState::MovementStateCodec
{
	namespace
	{
		// The shared header check, the same shape WorldEntryProtocol.cpp uses.
		//
		// `frame.size()` is the authority: a declared size is only ever COMPARED with
		// it, never used to index. That ordering is what stops a peer declaring 76
		// bytes and being read out of a 12-byte buffer.
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
		// `resize`d to its final length before anything is written, so an appending
		// writer would place every field AFTER the frame instead of inside it - and
		// the result is a correctly-sized vector of zeros followed by the real bytes,
		// which is exactly the kind of frame that passes a length check and fails
		// silently in a test that only asserts on size.
		//
		// Same helper, and the same reason, as WorldEntryProtocol.cpp's PutU32.
		void PutU32(std::vector<WireU8>& out, std::size_t offset, WireU32 value) noexcept
		{
			for (int i = 0; i < 4; ++i)
			{
				out[offset + static_cast<std::size_t>(i)] =
				    static_cast<WireU8>((value >> (8 * i)) & 0xFFu);
			}
		}

		// `dwSize` then `nType`, each four bytes, and NOTHING else on the wire.
		//
		// Deliberately NOT sizeof(MessageHeader): the struct is padded by the compiler,
		// so its size is not the header's wire length. Using it put nType at offset 8
		// instead of 4 - which put a size field where the id belonged and produced a
		// frame that was the right LENGTH and completely wrong. The two constants below
		// are checked against the measured 12- and 16-byte sizes by static_assert in
		// MovementStateProtocol.h.
		constexpr std::size_t kWireSizeOffset   = 0;
		constexpr std::size_t kWireIdOffset     = 4;
		constexpr std::size_t kWireHeaderLength = 8;

		void AppendHeader(std::vector<WireU8>& out, std::size_t start, MessageId id,
		                  std::size_t size) noexcept
		{
			PutU32(out, start + kWireSizeOffset, static_cast<WireU32>(size));
			PutU32(out, start + kWireIdOffset, id);
		}
	}

	bool IsMoveState(MessageId id) noexcept
	{
		return id == MovementState::kMoveStateId;
	}

	bool IsMoveStateBroadcast(MessageId id) noexcept
	{
		return id == MovementState::kMoveStateBrdId;
	}

	Status AppendMoveStateRequest(std::vector<WireU8>& out,
	                              const MoveStateRequest& request)
	{
		out.resize(out.size() + MovementState::kRequestSize, 0);

		const std::size_t start = out.size() - MovementState::kRequestSize;
		AppendHeader(out, start, MovementState::kMoveStateId,
		             MovementState::kRequestSize);

		PutU32(out, start + MovementState::kRequestActStateOffset, request.actState);
		return Ok();
	}

	Status DecodeMoveStateRequest(const std::vector<WireU8>& frame,
	                               MoveStateRequest& out)
	{
		out = MoveStateRequest{};

		if (const Status status = RequireFrame(frame, MovementState::kMoveStateId,
		                                       MovementState::kRequestSize);
		    status.IsError())
		{
			return status;
		}

		out.actState = Codec::ReadU32(frame.data() + MovementState::kRequestActStateOffset);
		return Ok();
	}

	Status AppendMoveStateBroadcast(std::vector<WireU8>& out,
	                                 const MoveStateBroadcast& broadcast)
	{
		out.resize(out.size() + MovementState::kBroadcastSize, 0);

		const std::size_t start = out.size() - MovementState::kBroadcastSize;
		AppendHeader(out, start, MovementState::kMoveStateBrdId,
		             MovementState::kBroadcastSize);

		// dwGaeaID FIRST, then dwActState - the base-class-first order that C++
		// inheritance produces and that the probe measured.
		PutU32(out, start + MovementState::kBroadcastGaeaIdOffset, broadcast.gaeaId);
		PutU32(out, start + MovementState::kBroadcastActStateOffset, broadcast.actState);
		return Ok();
	}

	Status DecodeMoveStateBroadcast(const std::vector<WireU8>& frame,
	                                MoveStateBroadcast& out)
	{
		out = MoveStateBroadcast{};

		if (const Status status = RequireFrame(frame, MovementState::kMoveStateBrdId,
		                                       MovementState::kBroadcastSize);
		    status.IsError())
		{
			return status;
		}

		out.gaeaId   = Codec::ReadU32(frame.data() + MovementState::kBroadcastGaeaIdOffset);
		out.actState = Codec::ReadU32(frame.data() + MovementState::kBroadcastActStateOffset);
		return Ok();
	}
}