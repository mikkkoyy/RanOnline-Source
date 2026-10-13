#include "FieldFrameNormalization.h"

#include "AttackProtocol.h"
#include "GotoProtocol.h"
#include "LoginProtocol.h"
#include "MovementStateProtocol.h"
#include "SkillProtocol.h"
#include "WorldEntryProtocol.h"

#include <cstddef>
#include <cstring>

namespace Modern::Network
{
	namespace
	{
		// The valid CLEAN payload sizes of one message id: the
		// sizes the message is legal to carry with NO garbage
		// token in it. `step` is the stride between legal sizes,
		// which is 1 for every fixed-size message (min == max,
		// so exactly one size is legal) and 4 for 3303.
		struct CleanPayloadRule
		{
			MessageId id;
			std::size_t minPayload;
			std::size_t maxPayload;
			std::size_t step;
		};

		// The five message ids FieldRoleRuntime::ServePeer
		// dispatches, and nothing else. Every bound is the
		// protocol's own measured constant minus the 8-byte
		// header, so this table reads the same numbers the
		// codecs validate and cannot drift from them.
		//
		// 2359 is a LOBBY-range id, not a GCTRL id - and that
		// matters: the garbage token is inserted into ALL
		// non-login client traffic, so the field identity
		// carries it exactly like a 3032 does. A rule keyed on
		// "id >= kGCtrlBase" would wrongly leave a
		// token-prefixed 2359 unnormalized.
		constexpr CleanPayloadRule kInboundRules[] = {
			// NET_MSG_JOIN_FIELD_IDENTITY 2359 - 24 bytes total
			// (WorldEntryProtocol.h:113).
			{ WorldEntry::kJoinFieldId,
			  WorldEntry::kIdentitySize - kMessageHeaderSize,
			  WorldEntry::kIdentitySize - kMessageHeaderSize,
			  1 },
			// MOVESTATE 3032 - 12 bytes total
			// (MovementStateProtocol.h:73).
			{ MovementState::kMoveStateId,
			  MovementState::kRequestSize - kMessageHeaderSize,
			  MovementState::kRequestSize - kMessageHeaderSize,
			  1 },
			// GOTO 3034 - 36 bytes total (GotoProtocol.h:117).
			{ Goto::kGotoId,
			  Goto::kRequestSize - kMessageHeaderSize,
			  Goto::kRequestSize - kMessageHeaderSize,
			  1 },
			// ATTACK 3036 - 24 bytes total (AttackProtocol.h:97).
			{ Attack::kAttackId,
			  Attack::kRequestSize - kMessageHeaderSize,
			  Attack::kRequestSize - kMessageHeaderSize,
			  1 },
			// REQ_SKILL 3303 - 63 + 4n bytes total, 0 <= n <= 24
			// (SkillProtocol.h:191-197). The payload is 55 + 4n.
			{ Skill::kSkillId,
			  Skill::kRequestHeaderSize - kMessageHeaderSize,
			  Skill::kRequestMaxSize - kMessageHeaderSize,
			  4 },
		};

		// True when `payloadSize` is a legal CLEAN payload size
		// for `rule`. For the fixed-size messages that means the
		// one measured size; for 3303 it means 55 + 4n within
		// the measured bounds.
		bool IsCleanPayload(const CleanPayloadRule& rule,
		                    std::size_t payloadSize) noexcept
		{
			if (payloadSize < rule.minPayload ||
			    payloadSize > rule.maxPayload)
			{
				return false;
			}
			return (payloadSize - rule.minPayload) % rule.step == 0;
		}

		// The rule for `id`, or nullptr when the id is not one
		// the Field connection dispatches.
		const CleanPayloadRule* FindRule(MessageId id) noexcept
		{
			for (const CleanPayloadRule& rule : kInboundRules)
			{
				if (rule.id == id)
				{
					return &rule;
				}
			}
			return nullptr;
		}
	}

	namespace FieldFrame
	{
		Status NormalizeInbound(Message& message) noexcept
		{
			// ---- dwSize is untrusted: the header is validated
			// before any token byte is read or any byte moves.
			if (message.header.size < kMessageHeaderSize)
			{
				return Status(ErrorCode::InvalidArgument);
			}

			// The framer guarantees this equality for every
			// message it delivers - DecodeMessage copies exactly
			// `header.size - 8` payload bytes - and checking it
			// here keeps the function safe when a test calls it
			// on a hand-built Message, and catches a frame whose
			// declared size disagrees with the payload it
			// actually carries.
			if (message.header.size !=
			    kMessageHeaderSize + message.payload.size())
			{
				return Status(ErrorCode::InvalidArgument);
			}

			// ---- The two envelopes that are not field traffic.
			//
			// NET_MSG_LOGIN_2 (2049) carries the same garbage
			// token, but LoginProtocol::Decode identifies and
			// strips it itself; this boundary must not touch
			// login semantics. NET_COMPRESS (170) is the
			// server->client compression envelope (V030); inbound
			// field traffic is never compressed, and nothing here
			// may add compression to it.
			if (message.header.type == LoginProtocol::kLoginMessageId ||
			    message.header.type == WellKnownMessage::kCompress)
			{
				return Ok();
			}

			// ---- Only the ids the Field connection dispatches
			// are ever normalized. An empty payload has no token
			// to strip and no clean size to match, so it is left
			// for the decoder to reject.
			const CleanPayloadRule* rule = FindRule(message.header.type);
			if (rule == nullptr || message.payload.empty())
			{
				return Ok();
			}

			// ---- THE COLLISION GUARD.
			//
			// A payload whose size is already a legal CLEAN size
			// is a valid token-free frame - the form every
			// modern client and test tool sends - and is never
			// modified, even when its first bytes happen to spell
			// one of the five tokens exactly. The size families
			// are disjoint (see the header), so this check cannot
			// hide a real token: a genuine token-prefixed frame
			// is clean + g bytes, which is never a clean size.
			if (IsCleanPayload(*rule, message.payload.size()))
			{
				return Ok();
			}

			// ---- A garbage token: exactly one of the five, at
			// payload offset 0, matched by CONTENT. The table is
			// LoginProtocol's own - the single legacy token
			// definition in this tree - and each entry is
			// matched in full, with no length guessing and no
			// unchecked C-string read: the length is the token's
			// own, and the payload is bounds-checked against it
			// before the comparison.
			for (const char* token : LoginProtocol::GarbageTokens())
			{
				const std::size_t tokenLength = std::strlen(token);
				if (message.payload.size() < tokenLength)
				{
					continue;
				}
				if (std::memcmp(message.payload.data(), token,
				                tokenLength) != 0)
				{
					continue;
				}

				// Strip only when the frame is a token-prefixed
				// version of a LEGAL frame: after removing exactly
				// the token bytes, the payload must be a legal
				// clean size for this id. A payload that is
				// clean + g for no legal clean size (malformed,
				// or an unknown prefix that happens to match) is
				// left for the decoder to reject.
				if (!IsCleanPayload(*rule,
				                    message.payload.size() -
				                        tokenLength))
				{
					continue;
				}

				// Remove exactly the token bytes at offset 8 and
				// decrease the declared size by exactly the token
				// length. `header.size` is the host-order field
				// the framer produced, so ReconstructFrame
				// re-emits the corrected little-endian header.
				message.payload.erase(
				    message.payload.begin(),
				    message.payload.begin() +
				        static_cast<std::ptrdiff_t>(tokenLength));
				message.header.size -= static_cast<WireU32>(tokenLength);
				return Ok();
			}

			// Neither a clean frame nor a token-prefixed one:
			// malformed, and the message-specific decoder is the
			// authority that rejects it.
			return Ok();
		}
	}
}
