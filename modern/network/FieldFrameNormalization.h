#pragma once

// SKILL-023: the ONE inbound field-frame normalization boundary.
//
// ===========================================================================
// WHAT THE LEGACY CLIENT ACTUALLY SENDS
// ===========================================================================
//
// The ASURA client splices one of five garbage tokens into EVERY
// non-login client->server message, immediately after the 8-byte
// NET_MSG_GENERIC header, and inflates dwSize by the token's length
// (s_NetClient.cpp:893-922, SendMsgAddGarbageValue). The token is
// chosen from GARBAGE_DATA (RcvMsgBuffer.cpp:11-12), which is exactly
// the table LoginProtocol::GarbageTokens() already owns:
//
//     K9IHANA    7 bytes
//     L8IDUL     6
//     M7HSET     6
//     N6GNET     6
//     O5FDASEOT  9
//
// The legacy field server identifies the token by CONTENT at offset 8
// (SetGarbageNum, RcvMsgBuffer.cpp:187-211), removes exactly those
// bytes, decreases dwSize, and shifts the payload back into place
// (getOneMsg, RcvMsgBuffer.cpp:229-249) - before any message-specific
// decoding. This file is that one step, in the modern tree, in one
// place.
//
// ===========================================================================
// THE BOUNDARY, AND WHY IT IS HERE
// ===========================================================================
//
// NormalizeInbound runs ONCE, after ConnectionFramer has delivered a
// WHOLE frame (a partial frame is never normalized - the framing
// boundary owns that invariant) and before any message-specific codec
// sees the payload. FieldRoleRuntime::ServePeer calls it between the
// read and the dispatch. Every field codec therefore receives the same
// bytes a modern client sends, and no codec contains a stripper of its
// own - one implementation path for recognition, length checks, buffer
// mutation and size correction, which is what SKILL-023's brief
// requires.
//
// ===========================================================================
// HOW A REAL TOKEN-PREFIXED FRAME IS TOLD APART FROM AN ORDINARY FRAME
// ===========================================================================
//
// This is the only safety question in the design, and it is answered by
// STRUCTURE, not by content alone.
//
// Content matching alone is NOT safe: a valid token-free frame whose
// payload merely begins with the bytes "L8IDUL" (a dwActState of
// 0x4449384C, a skill id, ...) would be corrupted by a stripper that
// matched on content. So the decision uses the frame's declared size:
//
//   1. The valid CLEAN payload sizes of every message id the Field
//      connection dispatches are known constants - 2359 carries 16,
//      3032 carries 4, 3034 carries 28, 3036 carries 16, and 3303
//      carries 55 + 4n for 0 <= n <= 24 (SkillProtocol.h: "THE
//      VARIABLE LENGTH IS REAL", dwSize = 63 + 4n). The table below
//      is wired to those same constants, so it cannot drift from the
//      codecs that validate the same sizes.
//
//   2. Those two families are DISJOINT for every dispatched id. For
//      the four fixed-size messages, a token-prefixed frame carries
//      clean + g bytes and g is 6, 7 or 9 - never 0. For 3303, every
//      clean size is 55 (mod 4) and no token length is 0 (mod 4), so
//      clean + g is never a legal clean size. Therefore "the payload
//      already IS a legal clean size" identifies a token-free frame,
//      and "the payload is an exact token followed by a legal clean
//      size" identifies a token-prefixed frame, with NO overlap and
//      NO case that requires guessing.
//
//   3. The guard order is: clean-valid FIRST, token-strip SECOND. A
//      payload whose size is already a legal clean size is never
//      modified, even when its first bytes spell a token exactly. That
//      is the collision guard, and it is why an ordinary frame whose
//      payload coincidentally starts with token-like bytes is not
//      corrupted: it matches the clean rule before any token is tried.
//
//   4. Anything that is neither (unknown prefix, truncated token, a
//      size that is clean+g for no legal clean size) is left exactly
//      as received. The message-specific decoder is the authority that
//      rejects malformed frames; this boundary never weakens a size
//      check to make a packet pass, and never strips a token that
//      would not leave a legal frame behind.
//
// ===========================================================================
// WHAT THIS DELIBERATELY DOES NOT DO
// ===========================================================================
//
// No compression. NET_COMPRESS (170) is the server->client envelope
// (V030) and inbound field traffic is never compressed; the id is
// excluded by name. NET_MSG_LOGIN_2 (2049) carries the same token but
// is normalized by LoginProtocol::Decode itself, which owns login
// semantics; this boundary excludes it by name and changes nothing
// about it. And an id the Field connection does not dispatch is never
// normalized at all - the fail-closed choice, because the dispatcher
// answers such an id with its unexpected-message refusal, and a frame
// this boundary does not know is never guessed at.

#include "NetworkCodec.h"
#include "NetworkTypes.h"

#include "types/Result.h"

namespace Modern::Network
{
	namespace FieldFrame
	{
		// Normalizes ONE inbound field frame in place: strips the legacy
		// garbage token when - and only when - the frame is a
		// token-prefixed version of a legal field message.
		//
		// Returns Ok() for every frame this boundary does not modify,
		// including clean frames, unknown prefixes and malformed frames
		// (which the message-specific decoder rejects). Returns
		// InvalidArgument only for a structurally inconsistent frame:
		// a declared size below the 8-byte header, or a declared size
		// that disagrees with the payload the frame actually carries.
		//
		// `message` is modified in place - the token bytes are erased
		// from the payload and dwSize is decreased by exactly the token
		// length, in the host-order MessageHeader the framer produced,
		// so ReconstructFrame re-emits the corrected little-endian
		// header.
		Status NormalizeInbound(Message& message) noexcept;
	}
}
