#pragma once

// WORLD-ENTRY-001 Phase C: the one read loop shared by every server role.
//
// ---------------------------------------------------------------------------
// WHY THIS IS A SHARED TYPE AND NOT A PER-SERVER PRIVATE METHOD
// ---------------------------------------------------------------------------
//
// LOGIN-002's LoginServerRuntime has a private ReadMessage: a deadline, a Receive
// slice, ConnectionFramer::Feed, ConnectionFramer::Next, and a mapping from each
// possible failure to a reason. Phase C needs that exact loop twice more - once for
// the Agent role and once for the Field role - and it is the same shape both
// times, differing only in which refusal enum they report into.
//
// Copy-pasting it a third and fourth time would put four subtly different copies
// of a deadline loop in the tree, and they would drift: one would gain a bounds
// check, another would not, and neither difference would be visible in a test.
//
// So it lives here, and LOGIN-002's ReadMessage is a thin wrapper over it. That
// keeps ONE implementation, which is the point - see NetworkConnection.h for the
// same argument about ConnectionFramer itself, which this type uses rather than
// reimplements.
//
// ---------------------------------------------------------------------------
// WHAT IT DELIBERATELY DOES NOT CONTAIN
// ---------------------------------------------------------------------------
//
// No protocol knowledge. It does not know that 2247 exists, what a session state
// is, or that a message must be a certain length. Those belong to the role. What
// it owns is exactly one question: "give me the next whole message, or tell me
// precisely why you could not."
//
// It reads no clock of its own: the deadline is computed from a steady_clock at
// the call and compared per iteration, so the budget measures time ACTUALLY
// WAITED. LOGIN-002 arrived at that after a bug - subtracting the read timeout per
// iteration charged a full slice for a read that returned instantly, so a server
// that read fast exhausted its budget while making real progress.
//
// ---------------------------------------------------------------------------
// THE FOUR OUTCOMES ARE FOUR OUTCOMES
// ---------------------------------------------------------------------------
//
// PeerClosed, TimedOut and TransportFault are distinct because they mean
// different things to a caller: the client left, the client is still there and
// silent, or the socket failed. Collapsing them would make a retry decision and
// a fault report indistinguishable. `MalformedHeader` is separate again, and is
// terminal for the connection - once the stream position is unknown there is no
// resynchronisation, so the caller must drop the connection rather than keep
// parsing.

#include "NetworkConnection.h"
#include "NetworkTransport.h"
#include "NetworkTypes.h"
#include "TcpTransport.h"
#include "types/Result.h"

#include <cstddef>
#include <string>
#include <vector>

namespace Modern::Network
{
	// Why a read did not produce a message.
	//
	// Not a Status: `Status` has no member for "not yet" or "the peer went away",
	// and overloading InvalidState to cover them would make a caller's error
	// handling lie. Same reasoning as FrameStatus, and for the same reason.
	enum class ReadOutcome : std::uint8_t
	{
		// A whole message is in `message`.
		Ok = 0,

		// The peer closed before a complete message arrived.
		//
		// Distinct from TimedOut on purpose: this client left, whereas a timeout
		// means it is still connected and still saying nothing.
		PeerClosed,

		// Nothing arrived within the budget.
		TimedOut,

		// The framer refused the received bytes: the buffer would exceed capacity, so
		// the peer is trying to make the server grow without bound.
		Oversized,

		// A header that cannot be valid, or the framer latched an unrecoverable
		// error. Terminal for this connection - the stream is no longer trustworthy.
		MalformedHeader,

		// Receive or send failed for a reason that is not an orderly close.
		TransportFault,
	};

	const char* ToString(ReadOutcome outcome) noexcept;

	// How one read ended, with a human-readable detail for logs.
	struct ReadResult
	{
		ReadOutcome outcome = ReadOutcome::Ok;
		std::string detail;

		bool IsOk() const noexcept { return outcome == ReadOutcome::Ok; }
	};

	// Reads one whole message from a transport.
	//
	// `reader` is supplied by the caller rather than owned here, because a framer
	// belongs to a CONNECTION and must not outlive one - a framer that survived its
	// connection could complete a message out of bytes left over from a client that
	// had already gone. That is invisible in testing and obvious in production, so
	// every role creates one per accepted socket.
	//
	// `budgetMilliseconds` bounds the WHOLE read, not each Receive. A per-read
	// timeout would let a peer that dribbles one byte at a time hold a connection
	// open indefinitely, which is the same denial of service in miniature that the
	// per-exchange budget exists to prevent.
	//
	// `maxChunkBytes` caps how much a single Receive asks for. 0 means "whatever is
	// reasonable". It exists so a test can force 1-byte reads and prove that
	// ConnectionFramer - not luck - is what reassembles a message; production
	// callers always pass 0.
	ReadResult ReadOneMessage(TcpTransport&  connection,
	                          ConnectionFramer& reader,
	                          Message&          message,
	                          int               budgetMilliseconds,
	                          std::size_t       maxChunkBytes = 0);

	// Rebuilds the on-wire bytes of a message the framer has already split.
	//
	// ConnectionFramer splits an 8-byte NET_MSG_GENERIC out of the stream and hands
	// the caller a Message whose header carries dwSize and nType while `payload`
	// carries only the body. Every protocol codec in this tree takes the WHOLE frame
	// - header included - because it validates both fields, and both live in the part
	// the framer removed.
	//
	// So a role that wants to hand a decoded message to a codec has to put the header
	// back. Doing that in one place rather than as six copied lines in each role is
	// the whole reason this function exists, and it is byte-for-byte what a caller
	// would otherwise write.
	std::vector<WireU8> ReconstructFrame(const Message& message);
}