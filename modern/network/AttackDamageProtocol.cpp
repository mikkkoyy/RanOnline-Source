#include "AttackDamageProtocol.h"

#include <cstring>

namespace Modern::Network::Attack::AttackDamageCodec
{
	namespace
	{
		// The shared header check, the same shape AttackProtocol.cpp uses.
		//
		// `frame.size()` is the authority: a declared size is only ever COMPARED
		// with it, never used to index.
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

		// Writes at an ABSOLUTE offset, little-endian by construction, so the bytes
		// do not depend on the host's layout. Same helper and same reason as
		// AttackProtocol.cpp's PutU32.
		void PutU32(std::vector<WireU8>& out, std::size_t offset, WireU32 value) noexcept
		{
			for (int i = 0; i < 4; ++i)
			{
				out[offset + static_cast<std::size_t>(i)] =
				    static_cast<WireU8>((value >> (8 * i)) & 0xFFu);
			}
		}

		// The SIGNED counterpart.
		//
		// Not an alias for PutU32 with a cast: the shift and mask are done in the
		// unsigned domain and the result is cast back, which is the only spelling
		// that is correct for a negative value under a right-shift on a signed type.
		void PutI32(std::vector<WireU8>& out, std::size_t offset, WireI32 value) noexcept
		{
			WireU32 bits = static_cast<WireU32>(value);
			for (int i = 0; i < 4; ++i)
			{
				out[offset + static_cast<std::size_t>(i)] =
				    static_cast<WireU8>((bits >> (8 * i)) & 0xFFu);
			}
		}

		// A SIGNED 32-bit read.
		//
		// Local rather than added to NetworkCodec: the shared codec exposes
		// ReadU32 and ReadF32 only, and this is the one field in the whole
		// codebase that is a signed int on the wire. Widening a shared helper for a
		// single caller would be a wider change than the field warrants.
		//
		// The bits are read unsigned and then REINTERPRETED, never converted: a
		// cast would saturate out-of-range values instead of preserving them.
		WireI32 ReadI32(const WireU8* data) noexcept
		{
			const WireU32 bits = Network::Codec::ReadU32(data);
			WireI32       value = 0;
			static_assert(sizeof(bits) == sizeof(value), "int32 must be 32-bit");
			std::memcpy(&value, &bits, sizeof(value));
			return value;
		}

		constexpr std::size_t kWireSizeOffset = 0;
		constexpr std::size_t kWireIdOffset   = 4;

		void AppendHeader(std::vector<WireU8>& out, std::size_t start, MessageId id,
		                  std::size_t size) noexcept
		{
			PutU32(out, start + kWireSizeOffset, static_cast<WireU32>(size));
			PutU32(out, start + kWireIdOffset, id);
		}
	}

	bool IsAttackDamage(MessageId id) noexcept
	{
		return id == Attack::kAttackDamageId;
	}

	bool IsAttackDamageBroadcast(MessageId id) noexcept
	{
		return id == Attack::kAttackDamageBrdId;
	}

	Status AppendAttackDamage(std::vector<WireU8>& out, const AttackDamage& damage)
	{
		// Legacy cannot produce a negative nDAMAGE: DamageProc floors it at 1 after
		// each truncating multiply (GLChar.cpp:2492, :2517). Refusing one here
		// keeps the encoder and decoder in agreement - every value the encoder
		// accepts must survive this pair, or a round-trip test would be the only
		// thing that noticed.
		if (damage.damage < 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		out.resize(out.size() + Attack::kDamageSize, 0);

		const std::size_t start = out.size() - Attack::kDamageSize;
		AppendHeader(out, start, Attack::kAttackDamageId, Attack::kDamageSize);

		PutU32(out, start + Attack::kDamageTargetCrowOffset, damage.targetCrow);
		PutU32(out, start + Attack::kDamageTargetIdOffset, damage.targetId);
		PutI32(out, start + Attack::kDamageAmountOffset, damage.damage);
		PutU32(out, start + Attack::kDamageFlagOffset, damage.damageFlag);
		return Ok();
	}

	Status DecodeAttackDamage(const std::vector<WireU8>& frame, AttackDamage& out)
	{
		out = AttackDamage{};

		if (const Status status = RequireFrame(frame, Attack::kAttackDamageId,
		                                       Attack::kDamageSize);
		    status.IsError())
		{
			return status;
		}

		out.targetCrow = Network::Codec::ReadU32(frame.data() + Attack::kDamageTargetCrowOffset);
		out.targetId   = Network::Codec::ReadU32(frame.data() + Attack::kDamageTargetIdOffset);
		out.damage     = ReadI32(frame.data() + Attack::kDamageAmountOffset);
		out.damageFlag = Network::Codec::ReadU32(frame.data() + Attack::kDamageFlagOffset);

		// Symmetric with the encoder. A negative amount is refused rather than
		// propagated, so a caller that trusted `damage` as "how much HP I took"
		// cannot be handed one that subtracts.
		if (out.damage < 0)
		{
			out = AttackDamage{};
			return Status(ErrorCode::InvalidArgument);
		}

		return Ok();
	}

	Status AppendAttackDamageBroadcast(std::vector<WireU8>& out,
	                                   const AttackDamageBroadcast& broadcast)
	{
		if (broadcast.damage < 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		out.resize(out.size() + Attack::kDamageBroadcastSize, 0);

		const std::size_t start = out.size() - Attack::kDamageBroadcastSize;
		AppendHeader(out, start, Attack::kAttackDamageBrdId, Attack::kDamageBroadcastSize);

		// dwGaeaID first: it comes from SNETPC_BROAD, the base class, and C++
		// inheritance lays a base class's members out first.
		PutU32(out, start + Attack::kDamageBroadcastGaeaIdOffset, broadcast.gaeaId);
		PutU32(out, start + Attack::kDamageBroadcastTargetCrowOffset, broadcast.targetCrow);
		PutU32(out, start + Attack::kDamageBroadcastTargetIdOffset, broadcast.targetId);
		PutI32(out, start + Attack::kDamageBroadcastAmountOffset, broadcast.damage);
		PutU32(out, start + Attack::kDamageBroadcastFlagOffset, broadcast.damageFlag);
		return Ok();
	}

	Status DecodeAttackDamageBroadcast(const std::vector<WireU8>& frame,
	                                   AttackDamageBroadcast& out)
	{
		out = AttackDamageBroadcast{};

		if (const Status status = RequireFrame(frame, Attack::kAttackDamageBrdId,
		                                       Attack::kDamageBroadcastSize);
		    status.IsError())
		{
			return status;
		}

		out.gaeaId =
		    Network::Codec::ReadU32(frame.data() + Attack::kDamageBroadcastGaeaIdOffset);
		out.targetCrow =
		    Network::Codec::ReadU32(frame.data() + Attack::kDamageBroadcastTargetCrowOffset);
		out.targetId =
		    Network::Codec::ReadU32(frame.data() + Attack::kDamageBroadcastTargetIdOffset);
		out.damage = ReadI32(frame.data() + Attack::kDamageBroadcastAmountOffset);
		out.damageFlag =
		    Network::Codec::ReadU32(frame.data() + Attack::kDamageBroadcastFlagOffset);

		if (out.damage < 0)
		{
			out = AttackDamageBroadcast{};
			return Status(ErrorCode::InvalidArgument);
		}

		return Ok();
	}
} // namespace Modern::Network::Attack::AttackDamageCodec