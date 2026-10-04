#include "MessageReader.h"

#include "NetworkCodec.h"

#include <chrono>
#include <vector>

namespace Modern::Network
{
	namespace
	{
		// How long a single receive waits before the loop re-checks the budget.
		//
		// Short, because the BUDGET is what is enforced - a long poll would make a
		// shutdown and the end of an exchange feel sluggish, and a short one costs
		// nothing but a few extra select() calls on an idle connection.
		constexpr int kReadSliceMilliseconds = 250;

		// Default read buffer. Larger than any single RAN message in this phase (the
		// 1022-byte spawn plus its 12-byte envelope) so a message usually arrives
		// whole - but "usually" is exactly the case ConnectionFramer exists for, and
		// the tests prove it by reading far smaller slices.
		constexpr std::size_t kReadBufferSize = 2048;
	}

	const char* ToString(ReadOutcome outcome) noexcept
	{
		switch (outcome)
		{
		case ReadOutcome::Ok:              return "Ok";
		case ReadOutcome::PeerClosed:      return "PeerClosed";
		case ReadOutcome::TimedOut:        return "TimedOut";
		case ReadOutcome::Oversized:       return "Oversized";
		case ReadOutcome::MalformedHeader: return "MalformedHeader";
		case ReadOutcome::TransportFault:  return "TransportFault";
		}
		return "Unrecognised";
	}

	ReadResult ReadOneMessage(TcpTransport& connection, ConnectionFramer& reader,
	                          Message& message, int budgetMilliseconds,
	                          std::size_t maxChunkBytes)
	{
		ReadResult result;

		// A non-positive budget would make the loop exit before trying, which is a
		// silent no-op rather than a wait. Treated as "wait a little" rather than
		// rejected, because a caller that computed a zero remainder meant "as little
		// as possible", not "never read".
		if (budgetMilliseconds <= 0)
		{
			budgetMilliseconds = 1;
		}

		const std::size_t chunk =
		    (maxChunkBytes == 0 || maxChunkBytes > kReadBufferSize) ? kReadBufferSize
		                                                             : maxChunkBytes;

		// steady_clock, so the budget measures time actually waited rather than
		// iterations. See the header for why that distinction is load-bearing.
		const auto deadline = std::chrono::steady_clock::now() +
		                     std::chrono::milliseconds(budgetMilliseconds);

		for (;;)
		{
			// THE BUFFER IS CHECKED BEFORE THE SOCKET IS.
			//
			// This ordering is the whole reason the loop is written this way, and getting
			// it wrong is a real defect rather than a style point.
			//
			// Two small client writes - a 2244 and a 2353, say - routinely arrive in ONE
			// TCP segment. An earlier read may therefore have buffered BOTH messages and
			// returned only the first. If this loop called Receive() before trying
			// Next(), it would block on the socket waiting for bytes that had already
			// been consumed, while a complete message sat in the framer. The caller would
			// sit there until the read budget expired and then report a timeout - for a
			// message it was already holding.
			//
			// It is intermittent because it depends on whether the kernel happened to
			// coalesce those two writes, which is why it presents as a flaky failure and
			// not as a deterministic one.
			{
				const FrameStatus buffered = reader.Next(message);
				if (buffered == FrameStatus::Ok)
				{
					result.outcome = ReadOutcome::Ok;
					return result;
				}
				if (buffered == FrameStatus::InvalidLength)
				{
					// A header that cannot be valid. The stream is no longer trustworthy -
					// there is no longer a known place where the next message starts - so
					// the connection is dropped rather than resynchronised onto garbage.
					result.outcome = ReadOutcome::MalformedHeader;
					result.detail  = "malformed frame header";
					return result;
				}
				// NeedMoreData: nothing whole is buffered, so the socket is the only place
				// more bytes can come from.
			}

			const auto now = std::chrono::steady_clock::now();			if (now >= deadline)
			{
				result.outcome = ReadOutcome::TimedOut;
				result.detail  = "no complete message within the read budget";
				return result;
			}

			const auto left =
			    std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count();
			if (left <= 0)
			{
				result.outcome = ReadOutcome::TimedOut;
				result.detail  = "no complete message within the read budget";
				return result;
			}

			const int slice =
			    left < kReadSliceMilliseconds ? static_cast<int>(left)
			                                 : kReadSliceMilliseconds;

			std::vector<WireU8> buffer(chunk);
			std::size_t        received = 0;

			const Status status =
			    connection.Receive(buffer.data(), buffer.size(), received, slice);

			if (status.IsError())
			{
				result.outcome = ReadOutcome::TransportFault;
				result.detail  = "receive failed: ";
				result.detail += status.GetMessage();
				result.detail += " (fault ";
				result.detail += ToString(connection.Fault());
				result.detail += ")";
				return result;
			}

			if (received == 0)
			{
				if (connection.Fault() == TransportFault::PeerClosed)
				{
					result.outcome = ReadOutcome::PeerClosed;
					result.detail  = "peer closed before sending a complete message";
					return result;
				}
				// Nothing yet. The deadline test at the top of the loop is the timeout.
				continue;
			}

			// Oversized: the peer is trying to make the buffer grow without bound. The
			// framer refuses rather than discarding, because silently dropping bytes
			// would desynchronise everything after them - and the connection is dropped
			// either way, so what matters is that the server NOTICES instead of growing.
			const FrameStatus fed = reader.Feed(buffer.data(), received);
			if (fed == FrameStatus::Oversized)
			{
				result.outcome = ReadOutcome::Oversized;
				result.detail  = "framer refused the received bytes (oversized)";
				return result;
			}
			if (fed == FrameStatus::InvalidLength)
			{
				result.outcome = ReadOutcome::MalformedHeader;
				result.detail  = "framer refused the received bytes";
				return result;
			}

			// Loop for the rest of a partial message. The next pass checks the buffer
			// first, so a message completed by THIS read is returned without another
			// syscall. The deadline is what stops a client that dribbles bytes forever:
			// it is bounded here rather than holding the connection open.
		}
	}

	std::vector<WireU8> ReconstructFrame(const Message& message)
	{
		// The header is written back exactly as it came off the wire, not
		// re-derived from the payload length. A codec that validates dwSize must be
		// shown the dwSize the peer actually declared, including a wrong one - that is
		// the value it exists to reject.
		std::vector<WireU8> frame;
		frame.reserve(static_cast<std::size_t>(message.header.size));
		(void)Codec::WriteU32(frame, message.header.size);
		(void)Codec::WriteU32(frame, message.header.type);
		frame.insert(frame.end(), message.payload.begin(), message.payload.end());
		return frame;
	}
}