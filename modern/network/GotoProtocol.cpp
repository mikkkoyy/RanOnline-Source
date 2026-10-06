#include "GotoProtocol.h"

#include <cmath>

namespace Modern::Network::Goto::GotoCodec
{
	namespace
	{
		// The shared header check, the same shape MovementStateProtocol.cpp uses.
		//
		// `frame.size()` is the authority: a declared size is only ever COMPARED with
		// it, never used to index. That ordering is what stops a peer declaring 44
		// bytes and being read out of a 36-byte buffer.
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
		// writer would place every field AFTER the frame - a correctly-sized vector
		// of zeros followed by the real bytes, which passes a length check and fails
		// silently in a test that only asserts on size. Same helper and same reason
		// as MovementStateProtocol.cpp's PutU32.
		void PutU32(std::vector<WireU8>& out, std::size_t offset, WireU32 value) noexcept
		{
			for (int i = 0; i < 4; ++i)
			{
				out[offset + static_cast<std::size_t>(i)] =
				    static_cast<WireU8>((value >> (8 * i)) & 0xFFu);
			}
		}

		// Three IEEE-754 singles, x then y then z, at an absolute offset.
		//
		// Little-endian by construction: the bit pattern is decomposed a byte at a
		// time, so the bytes on the wire do not depend on the host's float layout.
		// That is the same guarantee Codec::WriteF32 gives, expressed at an offset.
		void PutVector3(std::vector<WireU8>& out, std::size_t offset,
		                const RanWire::Vector3& value) noexcept
		{
			const float components[3] = { value.x, value.y, value.z };
			for (int c = 0; c < 3; ++c)
			{
				WireU32 bits = 0;
				static_assert(sizeof(bits) == sizeof(float), "float must be 32-bit");
				std::memcpy(&bits, &components[c], sizeof(bits));

				const std::size_t at = offset + static_cast<std::size_t>(c) * 4;
				PutU32(out, at, bits);
			}
		}

		RanWire::Vector3 ReadVector3(const WireU8* data) noexcept
		{
			RanWire::Vector3 value;
			float*           components[3] = { &value.x, &value.y, &value.z };

			for (int c = 0; c < 3; ++c)
			{
				const WireU32 bits = Codec::ReadU32(data + static_cast<std::size_t>(c) * 4);
				std::memcpy(components[c], &bits, sizeof(bits));
			}
			return value;
		}

		void PutFloat(std::vector<WireU8>& out, std::size_t offset, float value) noexcept
		{
			WireU32 bits = 0;
			std::memcpy(&bits, &value, sizeof(bits));
			PutU32(out, offset, bits);
		}

		// Every component finite, or the message is not one.
		//
		// This is a consequence check, not a policy. `vCurPos` feeds the 60-unit
		// desynchronisation test as `|m_vPos - vCurPos| > 60.0f`; with a NaN that
		// comparison is false, so a NaN would DISABLE the anti-teleport check rather
		// than trip it. `vTarPos` is handed straight to the navigation tree, where a
		// NaN propagates into an A* cost. Refusing here is where the damage is still
		// cheap to prevent.
		bool IsFiniteVector(const RanWire::Vector3& value) noexcept
		{
			return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
		}

		// `dwSize` then `nType`, four bytes each, and NOTHING else on the wire.
		//
		// Deliberately NOT sizeof(MessageHeader): the struct is padded by the
		// compiler, so its size is not the header's wire length. The same trap put
		// nType at offset 8 instead of 4 in an earlier revision of this code and
		// produced a frame of exactly the right length and entirely wrong. The
		// constants are checked against the measured 36- and 44-byte sizes by
		// static_assert in GotoProtocol.h.
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

	bool IsGoto(MessageId id) noexcept
	{
		return id == Goto::kGotoId;
	}

	bool IsGotoBroadcast(MessageId id) noexcept
	{
		return id == Goto::kGotoBrdId;
	}

	Status AppendGotoRequest(std::vector<WireU8>& out, const GotoRequest& request)
	{
		if (!IsFiniteVector(request.currentPosition) || !IsFiniteVector(request.targetPosition))
		{
			return Status(ErrorCode::InvalidArgument);
		}

		out.resize(out.size() + Goto::kRequestSize, 0);

		const std::size_t start = out.size() - Goto::kRequestSize;
		AppendHeader(out, start, Goto::kGotoId, Goto::kRequestSize);

		PutU32(out, start + Goto::kRequestActStateOffset, request.actState);
		PutVector3(out, start + Goto::kRequestCurrentPositionOffset, request.currentPosition);
		PutVector3(out, start + Goto::kRequestTargetPositionOffset, request.targetPosition);
		return Ok();
	}

	Status DecodeGotoRequest(const std::vector<WireU8>& frame, GotoRequest& out)
	{
		out = GotoRequest{};

		if (const Status status =
		        RequireFrame(frame, Goto::kGotoId, Goto::kRequestSize);
		    status.IsError())
		{
			return status;
		}

		out.actState = Codec::ReadU32(frame.data() + Goto::kRequestActStateOffset);
		out.currentPosition =
		    ReadVector3(frame.data() + Goto::kRequestCurrentPositionOffset);
		out.targetPosition = ReadVector3(frame.data() + Goto::kRequestTargetPositionOffset);

		if (!IsFiniteVector(out.currentPosition) || !IsFiniteVector(out.targetPosition))
		{
			out = GotoRequest{};
			return Status(ErrorCode::InvalidArgument);
		}
		return Ok();
	}

	Status AppendGotoBroadcast(std::vector<WireU8>& out, const GotoBroadcast& broadcast)
	{
		// `delay` is checked here for the same reason the two positions are, and NOT
		// because anyone sets it: the decoder below refuses a non-finite delay, so an
		// encoder that did not check would be able to emit a frame its own decoder
		// rejects. Every value the encoder accepts must survive a round trip through
		// this pair, or the two have drifted apart and no round-trip test would notice
		// until a real one failed.
		if (!IsFiniteVector(broadcast.currentPosition) || !IsFiniteVector(broadcast.targetPosition) ||
		    !std::isfinite(broadcast.delay))
		{
			return Status(ErrorCode::InvalidArgument);
		}

		out.resize(out.size() + Goto::kBroadcastSize, 0);

		const std::size_t start = out.size() - Goto::kBroadcastSize;
		AppendHeader(out, start, Goto::kGotoBrdId, Goto::kBroadcastSize);

		// dwGaeaID FIRST: it comes from SNETPC_BROAD, the base class, and C++
		// inheritance lays a base class's members out first. The probe measured 8.
		PutU32(out, start + Goto::kBroadcastGaeaIdOffset, broadcast.gaeaId);
		PutU32(out, start + Goto::kBroadcastActStateOffset, broadcast.actState);
		PutVector3(out, start + Goto::kBroadcastCurrentPositionOffset,
		           broadcast.currentPosition);
		PutVector3(out, start + Goto::kBroadcastTargetPositionOffset, broadcast.targetPosition);
		PutFloat(out, start + Goto::kBroadcastDelayOffset, broadcast.delay);
		return Ok();
	}

	Status DecodeGotoBroadcast(const std::vector<WireU8>& frame, GotoBroadcast& out)
	{
		out = GotoBroadcast{};

		if (const Status status =
		        RequireFrame(frame, Goto::kGotoBrdId, Goto::kBroadcastSize);
		    status.IsError())
		{
			return status;
		}

		out.gaeaId   = Codec::ReadU32(frame.data() + Goto::kBroadcastGaeaIdOffset);
		out.actState = Codec::ReadU32(frame.data() + Goto::kBroadcastActStateOffset);
		out.currentPosition =
		    ReadVector3(frame.data() + Goto::kBroadcastCurrentPositionOffset);
		out.targetPosition = ReadVector3(frame.data() + Goto::kBroadcastTargetPositionOffset);
		out.delay = Codec::ReadF32(frame.data() + Goto::kBroadcastDelayOffset);

		if (!IsFiniteVector(out.currentPosition) || !IsFiniteVector(out.targetPosition) ||
		    !std::isfinite(out.delay))
		{
			out = GotoBroadcast{};
			return Status(ErrorCode::InvalidArgument);
		}
		return Ok();
	}
}
