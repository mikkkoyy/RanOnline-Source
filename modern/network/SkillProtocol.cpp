#include "SkillProtocol.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace Modern::Network::Skill::SkillCodec
{
	namespace
	{
		// `dwSize` then `nType`, four bytes each, and NOTHING else on the wire.
		//
		// Deliberately NOT sizeof(MessageHeader): the struct is padded by the
		// compiler, so its size is not the header's wire length. Same reason and
		// same helper as GotoProtocol.cpp.
		constexpr std::size_t kWireSizeOffset = 0;
		constexpr std::size_t kWireIdOffset   = 4;

		// Writes at an ABSOLUTE offset, into an already-sized vector.
		//
		// Codec::WriteU32 APPENDS, which is the wrong tool: the frame is resized
		// to its final length before anything is written, so an appending writer
		// would place every field after the frame - a correctly-sized vector of
		// zeros followed by the real bytes, which passes a length check and fails
		// silently. Same helper and same reason as GotoProtocol.cpp's PutU32.
		void PutU32(std::vector<WireU8>& out, std::size_t offset, WireU32 value) noexcept
		{
			for (int i = 0; i < 4; ++i)
			{
				out[offset + static_cast<std::size_t>(i)] =
				    static_cast<WireU8>((value >> (8 * i)) & 0xFFu);
			}
		}

		void PutU16(std::vector<WireU8>& out, std::size_t offset, WireU16 value) noexcept
		{
			out[offset]     = static_cast<WireU8>(value & 0xFFu);
			out[offset + 1] = static_cast<WireU8>((value >> 8) & 0xFFu);
		}

		// A float on the wire is its IEEE-754 single bit pattern, little-endian.
		//
		// Decomposed a byte at a time from a memcpy'd 32-bit word, so the bytes do
		// not depend on the host's float layout and the SOURCE of the value is
		// never type-punned.
		void PutFloat(std::vector<WireU8>& out, std::size_t offset, float value) noexcept
		{
			WireU32 bits = 0;
			static_assert(sizeof(bits) == sizeof(value), "float must be 32-bit");
			std::memcpy(&bits, &value, sizeof(bits));
			PutU32(out, offset, bits);
		}

		void PutVector3(std::vector<WireU8>& out, std::size_t offset,
		                const RanWire::Vector3& value) noexcept
		{
			const float components[3] = { value.x, value.y, value.z };
			for (int c = 0; c < 3; ++c)
			{
				PutFloat(out, offset + static_cast<std::size_t>(c) * 4, components[c]);
			}
		}

		// A legacy enum is a signed 32-bit int. Read the 32 bits and reinterpret
		// them, rather than relying on the (implementation-defined before C++20)
		// conversion from an unsigned value that happens to exceed INT_MAX.
		WireI32 I32FromBits(WireU32 bits) noexcept
		{
			WireI32 value = 0;
			std::memcpy(&value, &bits, sizeof(value));
			return value;
		}

		void PutI32(std::vector<WireU8>& out, std::size_t offset, WireI32 value) noexcept
		{
			WireU32 bits = 0;
			std::memcpy(&bits, &value, sizeof(bits));
			PutU32(out, offset, bits);
		}

		void AppendHeader(std::vector<WireU8>& out, std::size_t start, MessageId id,
		                  std::size_t size) noexcept
		{
			PutU32(out, start + kWireSizeOffset, static_cast<WireU32>(size));
			PutU32(out, start + kWireIdOffset, id);
		}

		// The shared fixed-header check, for the FIXED-size messages.
		//
		// `frame.size()` is the authority: a declared size is only ever COMPARED
		// with it, never used to index. Same rule as GotoProtocol.cpp - and note
		// that 3303 and 3305 are the two messages in this tree that CANNOT use it,
		// because they have no single expected size.
		Status RequireExactFrame(const std::vector<WireU8>& frame, MessageId expectedId,
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

		// The variable-length length rule, shared by 3303 and 3305.
		//
		// Returns the target CAPACITY the length supports, or an error. Two
		// separate rejections, because they are different faults:
		//
		//   * a length below the fixed header is a truncated packet;
		//   * a length above the maximum is longer than the array can ever be;
		//   * a length that is not congruent to the 4-byte entry over the header
		//     is not a packet this message can describe at all. 65 and 66 land
		//     here, and so does every length between 63 and 159 that is not
		//     63 + 4n.
		//
		// Checking `(size - header) % entry == 0` is what makes the capacity
		// below exact: the array is the LAST member, so the packet is the fixed
		// header plus a whole number of entries and nothing else.
		Status TargetCapacityFor(std::size_t frameSize, std::size_t headerSize,
		                         std::size_t maxSize, std::size_t& outCapacity)
		{
			if (frameSize < headerSize || frameSize > maxSize)
			{
				return Status(ErrorCode::InvalidArgument);
			}

			const std::size_t tail = frameSize - headerSize;
			if (tail % Skill::kTargetEntryBytes != 0)
			{
				return Status(ErrorCode::InvalidArgument);
			}

			const std::size_t capacity = tail / Skill::kTargetEntryBytes;
			if (capacity > Skill::kMaxTargets)
			{
				return Status(ErrorCode::InvalidArgument);
			}

			outCapacity = capacity;
			return Ok();
		}

		// Every float finite, or the message is not one.
		//
		// A consequence check, not a policy, and the reason here is specific to
		// this message. Legacy compares these five values against its OWN getters
		// with `!=` (GLCharSkillMsg.cpp:286-288 and :312-315). Every comparison
		// involving a NaN is false, so a NaN in `fSum_TarRange` would make the
		// echo check DISAGREE where the client meant to agree - a cast refused
		// for a reason no one can see. An infinity would do the same. And
		// `vTARPOS` drives ReceiveTeleport directly for EMSPECA_RANDOM_EXP_RATE
		// skills (GLCharSkillMsg.cpp:396), where a NaN would become a position.
		bool IsFiniteFloat(float value) noexcept
		{
			return std::isfinite(value);
		}

		bool IsFiniteVector(const RanWire::Vector3& value) noexcept
		{
			return IsFiniteFloat(value.x) && IsFiniteFloat(value.y) && IsFiniteFloat(value.z);
		}

		// Everything an encoder must refuse so that no frame it emits is one its
		// own decoder would reject. Enforced on BOTH sides on purpose: GotoCodec
		// checks the positions on append and the positions AND the delay on
		// decode, which leaves a documented round-trip hole. There is no reason to
		// repeat that here - a field exists to be compared against a
		// server-computed value, and no legitimate value of any of them is
		// non-finite.
		bool RequestFloatsAreEncodable(const Skill::SkillRequest& request) noexcept
		{
			return IsFiniteVector(request.targetPosition) &&
			       IsFiniteFloat(request.sumTarRange) &&
			       IsFiniteFloat(request.attackVelocity) &&
			       IsFiniteFloat(request.attackVelocityItem);
		}

		bool BroadcastFloatsAreEncodable(const Skill::SkillBroadcast& broadcast) noexcept
		{
			return IsFiniteVector(broadcast.targetPosition);
		}
	} // namespace

	bool IsSkill(MessageId id) noexcept
	{
		return id == Skill::kSkillId;
	}

	bool IsSkillFeedback(MessageId id) noexcept
	{
		return id == Skill::kSkillFbId;
	}

	bool IsSkillBroadcast(MessageId id) noexcept
	{
		return id == Skill::kSkillBrdId;
	}

	Status AppendSkillRequest(std::vector<WireU8>& out, const Skill::SkillRequest& request)
	{
		if (request.targets.size() > Skill::kMaxTargets)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		if (!RequestFloatsAreEncodable(request))
		{
			return Status(ErrorCode::InvalidArgument);
		}

		const std::size_t size =
		    Skill::kRequestHeaderSize + request.targets.size() * Skill::kTargetEntryBytes;

		out.resize(out.size() + size, 0);

		const std::size_t start = out.size() - size;
		AppendHeader(out, start, Skill::kSkillId, size);

		PutU32(out, start + Skill::kRequestSkillIdOffset, request.skillId.value);
		PutVector3(out, start + Skill::kRequestTargetPositionOffset, request.targetPosition);
		PutU32(out, start + Skill::kRequestFlagsOffset, request.flags);
		out[start + Skill::kRequestDefenseSkillOffset] = request.defenseSkill;
		PutFloat(out, start + Skill::kRequestSumTarRangeOffset, request.sumTarRange);
		PutU16(out, start + Skill::kRequestBodyRadiusOffset, request.bodyRadius);
		PutU16(out, start + Skill::kRequestAttackRangeOffset, request.attackRange);
		PutU16(out, start + Skill::kRequestSkillRangeApplyOffset, request.skillRangeApply);
		PutU16(out, start + Skill::kRequestSkillRangeTarOffset, request.skillRangeTar);
		PutFloat(out, start + Skill::kRequestAttackVelocityOffset, request.attackVelocity);
		PutFloat(out, start + Skill::kRequestAttackVelocityItemOffset,
		         request.attackVelocityItem);
		PutU32(out, start + Skill::kRequestAnimationKeyOffset, request.animationKey);
		PutI32(out, start + Skill::kRequestAnimationMainTypeOffset, request.animationMainType);
		PutI32(out, start + Skill::kRequestAnimationSubTypeOffset, request.animationSubType);

		// An encoder emits a SELF-CONSISTENT packet: the declared count is what
		// it is actually writing. The 159-bytes-with-zero-targets shape legacy
		// produces is a DECODER concern - a client that sends it must be
		// understood, not reproduced.
		PutU16(out, start + Skill::kRequestTargetCountOffset,
		       static_cast<WireU16>(request.targets.size()));

		for (std::size_t i = 0; i < request.targets.size(); ++i)
		{
			const std::size_t at = start + Skill::kRequestTargetsOffset +
			                       i * Skill::kTargetEntryBytes;
			PutU16(out, at, request.targets[i].crow);
			PutU16(out, at + 2, request.targets[i].id);
		}

		return Ok();
	}

	Status DecodeSkillRequest(const std::vector<WireU8>& frame, Skill::SkillRequest& out)
	{
		out = Skill::SkillRequest{};

		// The header, before anything else. A frame that is not even 63 bytes
		// cannot be read field by field, and reading field by field is what keeps
		// every access in bounds.
		if (frame.size() < kMessageHeaderSize)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		std::size_t capacity = 0;
		if (const Status status =
		        TargetCapacityFor(frame.size(), Skill::kRequestHeaderSize,
		                         Skill::kRequestMaxSize, capacity);
		    status.IsError())
		{
			return status;
		}

		// The declared size is COMPARED with the frame length and never used to
		// index - the rule that stops a peer declaring 159 bytes and being read
		// out of a 63-byte buffer.
		if (Codec::ReadU32(frame.data() + kWireSizeOffset) != frame.size() ||
		    Codec::ReadU32(frame.data() + kWireIdOffset) != Skill::kSkillId)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// Codec::Reader checks the remaining length before EVERY read, so the
		// unaligned floats at 29/41/45 and the 4-byte enums at 53/57 cannot be
		// read past the end even if a future edit moved an offset. It is
		// sequential, which is exactly the order these fields are declared in, so
		// no seek is needed and none is offered.
		Codec::Reader reader(frame.data(), frame.size());

		WireU32 sizeWord = 0;
		WireU32 idWord   = 0;
		if (reader.ReadU32(sizeWord).IsError() || reader.ReadU32(idWord).IsError())
		{
			return Status(ErrorCode::InvalidArgument);
		}

		WireU32 skillId = 0;
		if (reader.ReadU32(skillId).IsError())
		{
			return Status(ErrorCode::InvalidArgument);
		}
		out.skillId.value = skillId;

		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		if (reader.ReadF32(x).IsError() || reader.ReadF32(y).IsError() ||
		    reader.ReadF32(z).IsError())
		{
			return Status(ErrorCode::InvalidArgument);
		}
		out.targetPosition = RanWire::Vector3{ x, y, z };

		WireU32 flags = 0;
		if (reader.ReadU32(flags).IsError())
		{
			return Status(ErrorCode::InvalidArgument);
		}
		out.flags = flags;

		WireU8 defenseSkill = 0;
		if (reader.ReadU8(defenseSkill).IsError())
		{
			return Status(ErrorCode::InvalidArgument);
		}
		// A C++ bool on the wire is 0 or 1. Anything else is not a value legacy
		// could have produced, and legacy only ever tests it for truth, so the
		// byte is normalised to the two values the type can actually hold rather
		// than carried as a number that claims to be a bool.
		out.defenseSkill = defenseSkill != 0 ? 1u : 0u;

		if (reader.ReadF32(out.sumTarRange).IsError() ||
		    reader.ReadU16(out.bodyRadius).IsError() ||
		    reader.ReadU16(out.attackRange).IsError() ||
		    reader.ReadU16(out.skillRangeApply).IsError() ||
		    reader.ReadU16(out.skillRangeTar).IsError() ||
		    reader.ReadF32(out.attackVelocity).IsError() ||
		    reader.ReadF32(out.attackVelocityItem).IsError() ||
		    reader.ReadU32(out.animationKey).IsError())
		{
			return Status(ErrorCode::InvalidArgument);
		}

		WireU32 mainTypeBits = 0;
		WireU32 subTypeBits  = 0;
		if (reader.ReadU32(mainTypeBits).IsError() || reader.ReadU32(subTypeBits).IsError())
		{
			return Status(ErrorCode::InvalidArgument);
		}
		out.animationMainType = I32FromBits(mainTypeBits);
		out.animationSubType  = I32FromBits(subTypeBits);

		WireU16 declaredCount = 0;
		if (reader.ReadU16(declaredCount).IsError())
		{
			return Status(ErrorCode::InvalidArgument);
		}
		out.declaredTargetCount = declaredCount;
		out.targetCapacity     = capacity;

		// The two bounds, applied together. See the header for why each is
		// needed and what it defends against.
		//
		//   capacity     from the validated LENGTH, so no read can leave the
		//                bytes that were actually received;
		//   declared     from wTARGET_NUM, so a legacy full-size packet with
		//                zero targets does not become 24 fabricated ones.
		//
		// Neither can exceed kMaxTargets, so `wTARGET_NUM == 0xFFFF` needs no
		// special case: the minimum is taken before any loop begins.
		const std::size_t effective =
		    std::min(static_cast<std::size_t>(declaredCount), capacity);

		out.targets.reserve(effective);
		for (std::size_t i = 0; i < effective; ++i)
		{
			WireU16 crow = 0;
			WireU16 id   = 0;
			if (reader.ReadU16(crow).IsError() || reader.ReadU16(id).IsError())
			{
				out = Skill::SkillRequest{};
				return Status(ErrorCode::InvalidArgument);
			}
			out.targets.push_back(Skill::SkillTarget{ crow, id });
		}

		// The decoder consumed EXACTLY the bytes it intended to read: the fixed
		// header plus the `effective` targets, and nothing else.
		//
		// This is the check that makes the length rule above a fact rather than a
		// hope - if an offset were ever wrong, the cursor would finish somewhere
		// other than here and the packet would be refused rather than
		// half-understood.
		//
		// The end position is NOT simply "the end of the frame": when the peer
		// declared fewer targets than the length could hold, the difference is
		// the deliberately-ignored tail, which is exactly the legacy
		// full-size-with-zero-targets case. So the expectation is computed from
		// `capacity - effective`, and when the two agree this reduces to plain
		// exhaustion.
		const std::size_t ignoredEntries = capacity - effective;
		const std::size_t expectedEnd =
		    frame.size() - ignoredEntries * Skill::kTargetEntryBytes;

		if (reader.Offset() != expectedEnd)
		{
			out = Skill::SkillRequest{};
			return Status(ErrorCode::InvalidArgument);
		}

		if (!RequestFloatsAreEncodable(out))
		{
			out = Skill::SkillRequest{};
			return Status(ErrorCode::InvalidArgument);
		}

		return Ok();
	}

	Status AppendSkillFeedback(std::vector<WireU8>& out, const Skill::SkillFeedback& feedback)
	{
		// Exactly kNameBytes. The legacy field is char[33] for an id length of
		// 33 and the constructor only zeroed it, so there is no terminator to
		// add and no room for one. Padding a short name or cutting a long one
		// would both be a lie about what the server meant to send, so neither
		// happens: the length must already be right.
		if (feedback.name.size() != Skill::kNameBytes)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		out.resize(out.size() + Skill::kFeedbackSize, 0);

		const std::size_t start = out.size() - Skill::kFeedbackSize;
		AppendHeader(out, start, Skill::kSkillFbId, Skill::kFeedbackSize);

		PutI32(out, start + Skill::kFeedbackCheckOffset, feedback.skillCheck);

		const std::size_t nameAt = start + Skill::kFeedbackNameOffset;
		for (std::size_t i = 0; i < Skill::kNameBytes; ++i)
		{
			// One byte at a time rather than a memcpy of the string's data: the
			// source may contain a NUL, and `name.data()` has no terminator to
			// copy from. This copies exactly kNameBytes of exactly what is there.
			out[nameAt + i] = static_cast<WireU8>(feedback.name[i]);
		}

		return Ok();
	}

	Status DecodeSkillFeedback(const std::vector<WireU8>& frame, Skill::SkillFeedback& out)
	{
		out = Skill::SkillFeedback{};

		if (const Status status =
		        RequireExactFrame(frame, Skill::kSkillFbId, Skill::kFeedbackSize);
		    status.IsError())
		{
			return status;
		}

		const WireI32 check =
		    I32FromBits(Codec::ReadU32(frame.data() + Skill::kFeedbackCheckOffset));

		// Exactly 33 raw bytes, assigned by LENGTH. Never strlen, never
		// NUL-scanned, never assumed to terminate: a peer may legitimately send
		// 33 bytes with no NUL anywhere in them, and this is the only place in
		// the packet that can hold a character name.
		std::vector<WireU8> nameBytes;
		Codec::Reader reader(frame.data() + Skill::kFeedbackNameOffset, Skill::kNameBytes);
		if (reader.ReadBytes(nameBytes, Skill::kNameBytes).IsError())
		{
			return Status(ErrorCode::InvalidArgument);
		}

		out.skillCheck = check;
		out.name.assign(reinterpret_cast<const char*>(nameBytes.data()), nameBytes.size());
		return Ok();
	}

	Status AppendSkillBroadcast(std::vector<WireU8>& out, const Skill::SkillBroadcast& broadcast)
	{
		if (broadcast.targets.size() > Skill::kMaxTargets)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		if (!BroadcastFloatsAreEncodable(broadcast))
		{
			return Status(ErrorCode::InvalidArgument);
		}

		const std::size_t size =
		    Skill::kBroadcastHeaderSize + broadcast.targets.size() * Skill::kTargetEntryBytes;

		out.resize(out.size() + size, 0);

		const std::size_t start = out.size() - size;
		AppendHeader(out, start, Skill::kSkillBrdId, size);

		// emCrow is a FOUR-byte EMCROW here and wCrow inside a STARID is a
		// TWO-byte WORD. Both are what legacy has; neither is a mistake here.
		PutI32(out, start + Skill::kBroadcastCrowOffset, broadcast.crow);

		// The caster's id is the FULL 32 bits, unlike every target's 16.
		PutU32(out, start + Skill::kBroadcastCasterIdOffset, broadcast.casterId);
		PutU32(out, start + Skill::kBroadcastSkillIdOffset, broadcast.skillId.value);
		PutU16(out, start + Skill::kBroadcastLevelOffset, broadcast.level);
		PutVector3(out, start + Skill::kBroadcastTargetPositionOffset, broadcast.targetPosition);
		PutU16(out, start + Skill::kBroadcastTargetCountOffset,
		       static_cast<WireU16>(broadcast.targets.size()));

		for (std::size_t i = 0; i < broadcast.targets.size(); ++i)
		{
			const std::size_t at =
			    start + Skill::kBroadcastTargetsOffset + i * Skill::kTargetEntryBytes;
			PutU16(out, at, broadcast.targets[i].crow);
			PutU16(out, at + 2, broadcast.targets[i].id);
		}

		return Ok();
	}

	Status DecodeSkillBroadcast(const std::vector<WireU8>& frame, Skill::SkillBroadcast& out)
	{
		out = Skill::SkillBroadcast{};

		if (frame.size() < kMessageHeaderSize)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		std::size_t capacity = 0;
		if (const Status status =
		        TargetCapacityFor(frame.size(), Skill::kBroadcastHeaderSize,
		                         Skill::kBroadcastMaxSize, capacity);
		    status.IsError())
		{
			return status;
		}

		if (Codec::ReadU32(frame.data() + kWireSizeOffset) != frame.size() ||
		    Codec::ReadU32(frame.data() + kWireIdOffset) != Skill::kSkillBrdId)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		Codec::Reader reader(frame.data(), frame.size());

		WireU32 sizeWord = 0;
		WireU32 idWord   = 0;
		if (reader.ReadU32(sizeWord).IsError() || reader.ReadU32(idWord).IsError())
		{
			return Status(ErrorCode::InvalidArgument);
		}

		WireU32 crowBits = 0;
		if (reader.ReadU32(crowBits).IsError())
		{
			return Status(ErrorCode::InvalidArgument);
		}
		out.crow = I32FromBits(crowBits);

		if (reader.ReadU32(out.casterId).IsError())
		{
			return Status(ErrorCode::InvalidArgument);
		}

		WireU32 skillId = 0;
		if (reader.ReadU32(skillId).IsError())
		{
			return Status(ErrorCode::InvalidArgument);
		}
		out.skillId.value = skillId;

		if (reader.ReadU16(out.level).IsError())
		{
			return Status(ErrorCode::InvalidArgument);
		}

		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		if (reader.ReadF32(x).IsError() || reader.ReadF32(y).IsError() ||
		    reader.ReadF32(z).IsError())
		{
			return Status(ErrorCode::InvalidArgument);
		}
		out.targetPosition = RanWire::Vector3{ x, y, z };

		WireU16 declaredCount = 0;
		if (reader.ReadU16(declaredCount).IsError())
		{
			return Status(ErrorCode::InvalidArgument);
		}
		out.declaredTargetCount = declaredCount;
		out.targetCapacity     = capacity;

		const std::size_t effective =
		    std::min(static_cast<std::size_t>(declaredCount), capacity);

		out.targets.reserve(effective);
		for (std::size_t i = 0; i < effective; ++i)
		{
			WireU16 crow = 0;
			WireU16 id   = 0;
			if (reader.ReadU16(crow).IsError() || reader.ReadU16(id).IsError())
			{
				out = Skill::SkillBroadcast{};
				return Status(ErrorCode::InvalidArgument);
			}
			out.targets.push_back(Skill::SkillTarget{ crow, id });
		}

		// Exactly the bytes intended - see the identical check and its reasoning
		// in DecodeSkillRequest.
		const std::size_t ignoredEntries = capacity - effective;
		const std::size_t expectedEnd =
		    frame.size() - ignoredEntries * Skill::kTargetEntryBytes;

		if (reader.Offset() != expectedEnd)
		{
			out = Skill::SkillBroadcast{};
			return Status(ErrorCode::InvalidArgument);
		}

		if (!BroadcastFloatsAreEncodable(out))
		{
			out = Skill::SkillBroadcast{};
			return Status(ErrorCode::InvalidArgument);
		}

		return Ok();
	}
} // namespace Modern::Network::Skill::SkillCodec