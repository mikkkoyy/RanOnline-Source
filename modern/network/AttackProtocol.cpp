#include "AttackProtocol.h"

namespace Modern::Network::Attack::AttackCodec
{
	namespace
	{
		// The shared header check, the same shape GotoProtocol.cpp uses.
		//
		// `frame.size()` is the authority: a declared size is only ever COMPARED with
		// it, never used to index. That ordering is what stops a peer declaring 24
		// bytes and being read out of a 16-byte buffer.
		Status RequireFrame(const std::vector<WireU8>& frame, MessageId expectedId,
		                    std::size_t expectedSize)
		{
			if (frame.size() != expectedSize)
			{
				return Status(ErrorCode::InvalidArgument);
			}

			if (Network::Codec::ReadU32(frame.data()) != expectedSize ||
			    Network::Codec::ReadU32(frame.data() + 4) != expectedId)
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
		// as GotoProtocol.cpp's PutU32.
		void PutU32(std::vector<WireU8>& out, std::size_t offset, WireU32 value) noexcept
		{
			for (int i = 0; i < 4; ++i)
			{
				out[offset + static_cast<std::size_t>(i)] =
				    static_cast<WireU8>((value >> (8 * i)) & 0xFFu);
			}
		}

		// `dwSize` then `nType`, four bytes each, and NOTHING else on the wire.
		//
		// Deliberately NOT sizeof(MessageHeader): the struct is padded by the
		// compiler, so its size is not the header's wire length. The same trap put
		// nType at offset 8 instead of 4 in an earlier revision of GotoProtocol and
		// produced a frame of exactly the right length and entirely wrong.
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

	bool IsAttack(MessageId id) noexcept
	{
		return id == Attack::kAttackId;
	}

	bool IsAttackBroadcast(MessageId id) noexcept
	{
		return id == Attack::kAttackBrdId;
	}

	bool IsAttackAvoid(MessageId id) noexcept
	{
		return id == Attack::kAttackAvoidId;
	}

	bool IsAttackAvoidBroadcast(MessageId id) noexcept
	{
		return id == Attack::kAttackAvoidBrdId;
	}

	Status AppendAttackRequest(std::vector<WireU8>& out, const AttackRequest& request)
	{
		// Nothing is validated here, and that is a decision rather than an omission.
		//
		// Legacy refuses a bad ATTACK in the RUNTIME, not in the codec
		// (GLCharMsg.cpp:328-375: empty-msg, body, peace zone, target, range). The
		// codec's only job here is to put four DWORDs on the wire at their measured
		// offsets and to refuse to emit a frame its own decoder would reject - and
		// since every field is a DWORD, no value can fail that. An unknown
		// `targetCrow` and a zero `targetId` are therefore CARRIED, and the rule
		// decides what they mean. GotoProtocol refuses a non-finite coordinate
		// because a NaN would silently disable its own 60-unit check; there is no
		// equivalent hazard in four integers.
		out.resize(out.size() + Attack::kRequestSize, 0);

		const std::size_t start = out.size() - Attack::kRequestSize;
		AppendHeader(out, start, Attack::kAttackId, Attack::kRequestSize);

		PutU32(out, start + Attack::kRequestTargetCrowOffset, request.targetCrow);
		PutU32(out, start + Attack::kRequestTargetIdOffset, request.targetId);
		PutU32(out, start + Attack::kRequestAniSelOffset, request.aniSel);
		PutU32(out, start + Attack::kRequestFlagsOffset, request.flags);
		return Ok();
	}

	Status DecodeAttackRequest(const std::vector<WireU8>& frame, AttackRequest& out)
	{
		out = AttackRequest{};

		if (const Status status =
		        RequireFrame(frame, Attack::kAttackId, Attack::kRequestSize);
		    status.IsError())
		{
			return status;
		}

		out.targetCrow = Network::Codec::ReadU32(frame.data() + Attack::kRequestTargetCrowOffset);
		out.targetId   = Network::Codec::ReadU32(frame.data() + Attack::kRequestTargetIdOffset);
		out.aniSel     = Network::Codec::ReadU32(frame.data() + Attack::kRequestAniSelOffset);
		out.flags      = Network::Codec::ReadU32(frame.data() + Attack::kRequestFlagsOffset);
		return Ok();
	}

	Status AppendAttackBroadcast(std::vector<WireU8>& out, const AttackBroadcast& broadcast)
	{
		out.resize(out.size() + Attack::kBroadcastSize, 0);

		const std::size_t start = out.size() - Attack::kBroadcastSize;
		AppendHeader(out, start, Attack::kAttackBrdId, Attack::kBroadcastSize);

		// dwGaeaID FIRST: it comes from SNETPC_BROAD, the base class, and C++
		// inheritance lays a base class's members out first. Asserted at offset 8 in
		// AttackProtocol.h.
		PutU32(out, start + Attack::kBroadcastGaeaIdOffset, broadcast.gaeaId);
		PutU32(out, start + Attack::kBroadcastTargetCrowOffset, broadcast.targetCrow);
		PutU32(out, start + Attack::kBroadcastTargetIdOffset, broadcast.targetId);
		PutU32(out, start + Attack::kBroadcastAniSelOffset, broadcast.aniSel);
		return Ok();
	}

	Status DecodeAttackBroadcast(const std::vector<WireU8>& frame, AttackBroadcast& out)
	{
		out = AttackBroadcast{};

		if (const Status status =
		        RequireFrame(frame, Attack::kAttackBrdId, Attack::kBroadcastSize);
		    status.IsError())
		{
			return status;
		}

		out.gaeaId     = Network::Codec::ReadU32(frame.data() + Attack::kBroadcastGaeaIdOffset);
		out.targetCrow = Network::Codec::ReadU32(frame.data() + Attack::kBroadcastTargetCrowOffset);
		out.targetId   = Network::Codec::ReadU32(frame.data() + Attack::kBroadcastTargetIdOffset);
		out.aniSel     = Network::Codec::ReadU32(frame.data() + Attack::kBroadcastAniSelOffset);
		return Ok();
	}

	Status AppendAttackAvoid(std::vector<WireU8>& out, const AttackAvoid& avoid)
	{
		out.resize(out.size() + Attack::kAvoidSize, 0);

		const std::size_t start = out.size() - Attack::kAvoidSize;
		AppendHeader(out, start, Attack::kAttackAvoidId, Attack::kAvoidSize);

		PutU32(out, start + Attack::kAvoidTargetCrowOffset, avoid.targetCrow);
		PutU32(out, start + Attack::kAvoidTargetIdOffset, avoid.targetId);
		return Ok();
	}

	Status DecodeAttackAvoid(const std::vector<WireU8>& frame, AttackAvoid& out)
	{
		out = AttackAvoid{};

		if (const Status status =
		        RequireFrame(frame, Attack::kAttackAvoidId, Attack::kAvoidSize);
		    status.IsError())
		{
			return status;
		}

		out.targetCrow = Network::Codec::ReadU32(frame.data() + Attack::kAvoidTargetCrowOffset);
		out.targetId   = Network::Codec::ReadU32(frame.data() + Attack::kAvoidTargetIdOffset);
		return Ok();
	}

	Status AppendAttackAvoidBroadcast(std::vector<WireU8>& out,
	                                  const AttackAvoidBroadcast& broadcast)
	{
		out.resize(out.size() + Attack::kAvoidBroadcastSize, 0);

		const std::size_t start = out.size() - Attack::kAvoidBroadcastSize;
		AppendHeader(out, start, Attack::kAttackAvoidBrdId, Attack::kAvoidBroadcastSize);

		// dwGaeaID first, from SNETPC_BROAD, as in 3037.
		PutU32(out, start + Attack::kAvoidBroadcastGaeaIdOffset, broadcast.gaeaId);
		PutU32(out, start + Attack::kAvoidBroadcastTargetCrowOffset, broadcast.targetCrow);
		PutU32(out, start + Attack::kAvoidBroadcastTargetIdOffset, broadcast.targetId);
		return Ok();
	}

	Status DecodeAttackAvoidBroadcast(const std::vector<WireU8>& frame,
	                                  AttackAvoidBroadcast& out)
	{
		out = AttackAvoidBroadcast{};

		if (const Status status =
		        RequireFrame(frame, Attack::kAttackAvoidBrdId, Attack::kAvoidBroadcastSize);
		    status.IsError())
		{
			return status;
		}

		out.gaeaId = Network::Codec::ReadU32(frame.data() + Attack::kAvoidBroadcastGaeaIdOffset);
		out.targetCrow =
		    Network::Codec::ReadU32(frame.data() + Attack::kAvoidBroadcastTargetCrowOffset);
		out.targetId =
		    Network::Codec::ReadU32(frame.data() + Attack::kAvoidBroadcastTargetIdOffset);
		return Ok();
	}
} // namespace Modern::Network::Attack::AttackCodec