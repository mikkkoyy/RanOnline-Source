#include "LoginServerSession.h"


// steady_clock, so the exchange budget measures time actually waited rather
// than a nominal cost charged per read. See Pump.
#include <chrono>
namespace Modern::Client
{
	using namespace Modern::Network;

	namespace
	{
		// How long one Receive waits before Pump re-checks its budget.
		//
		// Short, because the budget is the loop's responsibility and not this value's:
		// a long poll would make Disconnect() and the end of a slow-but-progressing
		// exchange feel sluggish, and a short one costs a few extra select() calls on a
		// quiet socket.
		constexpr int kReadSliceMilliseconds = 100;

		// How much a single Receive asks for when the caller did not cap it.
		//
		// Larger than any RAN message in this exchange - the biggest is a 56-byte entry
		// plus an 8-byte header - so a healthy response normally arrives whole. "Normally"
		// is the operative word, and it is why ConnectionFramer exists.
		constexpr std::size_t kDefaultChunkBytes = 4096;

		// Never ask for more than the framer could hold, however the caller spelled it.
		//
		// Without this a caller passing a huge maxChunkBytes would get a buffer larger
		// than any message and waste the allocation; and a caller passing zero-ish
		// nonsense would get a zero-length buffer and an infinite loop of zero-byte
		// reads, which is the shape of a hang.
		std::size_t ClampChunk(std::size_t requested) noexcept
		{
			if (requested == 0)
			{
				return kDefaultChunkBytes;
			}
			constexpr std::size_t kMinimum = 1;
			return requested < kMinimum ? kMinimum : requested;
		}
	}

	const char* ToString(LoginServerSessionOutcome outcome) noexcept
	{
		switch (outcome)
		{
		case LoginServerSessionOutcome::Completed:  return "Completed";
		case LoginServerSessionOutcome::PeerClosed: return "PeerClosed";
		case LoginServerSessionOutcome::TimedOut:   return "TimedOut";
		case LoginServerSessionOutcome::Faulted:    return "Faulted";
		}
		return "Unrecognised";
	}

	LoginServerSession::LoginServerSession(LoginServerClient& protocol)
		: m_protocol(protocol)
	{
	}

	LoginServerSession::~LoginServerSession()
	{
		Disconnect();
	}

	Status LoginServerSession::Connect(const EndpointAddress& endpoint, int timeoutMilliseconds)
	{
		// A session is one conversation. Reconnecting one in place would leave the
		// previous connection's bytes somewhere in the protocol object, and the client
		// would have no way to tell them from the new server's - so a second
		// conversation is a second session, and this is refused rather than papered over.
		if (m_transport.IsConnected())
		{
			return Status(ErrorCode::AlreadyExists);
		}

		// BeginConnect validates the address as a numeric dotted-quad. That rule belongs
		// to the RAN client's CONFIGURATION contract, not to TCP - see the header - and
		// it is enforced before a socket is opened so a bad address costs nothing.
		if (const Status status = m_protocol.BeginConnect(endpoint); status.IsError())
		{
			return status;
		}

		// The transport takes a Network::Endpoint, which is the same host and port the
		// protocol validated. The two representations are converted here, once, at the
		// boundary - rather than making either type carry the other's field.
		Endpoint target;
		target.host = endpoint.ip;
		target.port = endpoint.port;

		if (const Status status = m_transport.Connect(target, timeoutMilliseconds);
		    status.IsError())
		{
			// Back to Disconnected rather than left mid-connect, so a caller that
			// retries does not find a session in a state it cannot drive.
			m_protocol.Disconnect();
			return status;
		}

		// Only now that the socket really is connected. Split from Connect on purpose:
		// the socket outcome is the transport's to report, and letting the protocol
		// advance on the strength of a call that failed would leave it claiming a
		// connection that does not exist.
		return m_protocol.CompleteConnect();
	}

	Status LoginServerSession::RequestGameServers()
	{
		std::vector<WireU8> request;
		if (const Status status = m_protocol.RequestGameServers(request); status.IsError())
		{
			return status;
		}
		return m_transport.Send(request.data(), request.size());
	}

	LoginServerExchange LoginServerSession::Pump(int timeoutMilliseconds, std::size_t maxChunkBytes)
	{
		LoginServerExchange result;

		// The request must already have gone out. Feeding bytes into a client that has
		// not asked is refused by the protocol itself, and surfacing that as Faulted
		// would be misleading - nothing failed on the wire.
		if (m_protocol.Phase() != LoginServerPhase::RequestingGameServers &&
		    m_protocol.Phase() != LoginServerPhase::ReceivingGameServers)
		{
			result.outcome = LoginServerSessionOutcome::Faulted;
			result.status  = Status(ErrorCode::InvalidState);
			return result;
		}

		const std::size_t chunk = ClampChunk(maxChunkBytes);

		// A WALL CLOCK DEADLINE, not a counter.
		//
		// The obvious implementation subtracts the read timeout from a budget on every
		// iteration, and it is wrong in a way that only shows up when reads are FAST:
		// a read that returns immediately still costs the full slice, so a client
		// reading 1 byte at a time spends its whole budget after 80 one-byte reads -
		// 80 bytes of a 176-byte response - and reports TimedOut with a partial list.
		// The earlier version of this function did exactly that, and it is why the
		// one-byte-at-a-time fragmentation test failed against a server that was
		// working perfectly.
		//
		// Measuring elapsed time charges only what was actually waited on, which is
		// what a deadline means.
		const auto total = timeoutMilliseconds > 0 ? timeoutMilliseconds : 10000;
		const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(total);

		while (true)
		{
			// Already complete - an empty list answers with a bare terminator, and a
			// caller may also have driven Feed by hand before calling Pump.
			if (m_protocol.IsComplete())
			{
				result.outcome = LoginServerSessionOutcome::Completed;
				result.status  = Ok();
				return result;
			}

			const auto   now      = std::chrono::steady_clock::now();
			if (now >= deadline)
			{
				result.outcome = LoginServerSessionOutcome::TimedOut;
				result.status  = Ok();
				return result;
			}

			const auto left = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count();
			if (left <= 0)
			{
				result.outcome = LoginServerSessionOutcome::TimedOut;
				result.status  = Ok();
				return result;
			}

			const int slice = left < kReadSliceMilliseconds ? static_cast<int>(left)
			                                                  : kReadSliceMilliseconds;

			std::vector<WireU8> buffer(chunk);
			std::size_t        received = 0;
			const Status       status = m_transport.Receive(buffer.data(), buffer.size(),
			                                                   received, slice);
			if (status.IsError())
			{
				result.outcome = LoginServerSessionOutcome::Faulted;
				result.status  = status;
				return result;
			}

			if (received == 0)
			{
				if (m_transport.Fault() == TransportFault::PeerClosed)
				{
					// An orderly close. Whether that is a fault depends entirely on
					// whether the list had already completed, and the loop above has
					// already established that it had not. So this is reported as its own
					// outcome: a server that hung up mid-response is an ordinary network
					// event, and a caller will usually retry rather than report it.
					result.outcome = LoginServerSessionOutcome::PeerClosed;
					result.status  = Ok();
					return result;
				}
				// Nothing yet. The deadline test at the top of the loop is the timeout.
				continue;
			}

			// Every received byte range goes to the protocol, which owns the framer.
			// No second framing parser here, and no assumption that one read is one
			// message - the slice size is the caller's to choose precisely so that this
			// cannot be assumed.
			std::size_t handled = 0;
			if (const Status fed = m_protocol.Feed(buffer.data(), received, handled);
			    fed.IsError())
			{
				// A response the protocol cannot accept: a malformed entry, a body where
				// a bare message belongs, or a framer that has latched. All three mean the
				// same thing to a caller - this connection's bytes are no longer
				// trustworthy - so they are one outcome rather than three.
				result.outcome = LoginServerSessionOutcome::Faulted;
				result.status  = fed;
				return result;
			}

			result.messagesHandled += handled;
		}
	}

	LoginServerExchange LoginServerSession::Exchange(const EndpointAddress& endpoint,
	                                                 int timeoutMilliseconds,
	                                                 std::size_t maxChunkBytes)
	{
		LoginServerExchange result;

		if (const Status status = Connect(endpoint, timeoutMilliseconds); status.IsError())
		{
			result.outcome = LoginServerSessionOutcome::Faulted;
			result.status  = status;
			return result;
		}

		if (const Status status = RequestGameServers(); status.IsError())
		{
			result.outcome = LoginServerSessionOutcome::Faulted;
			result.status  = status;
			return result;
		}

		return Pump(timeoutMilliseconds, maxChunkBytes);
	}

	void LoginServerSession::Disconnect() noexcept
	{
		// Transport first, protocol second. The order matters: the socket has to be
		// closed even if the protocol object is mid-exchange, and the protocol reset
		// must not be skipped because the socket was already gone.
		m_transport.Disconnect();
		(void)m_protocol.Disconnect();
	}
}
