#pragma once

// WORLD-002: the client receive side of the login response.
//
// Consumes raw bytes as they arrive from the socket, unwraps the V030 envelope,
// frames the inner message stream, recognises LOGIN_FB and exposes the result as a
// login phase.
//
// The pipeline is the one V028 established, in V030's order:
//
//   TCP bytes
//     -> NET_COMPRESS envelope        NetCompress::DecodeServerToClientEnvelope
//     -> inner NET_MSG_GENERIC stream ConnectionFramer
//     -> NET_MSG_LOGIN_FB             LoginResponse::Decode
//     -> login phase
//
// Nothing here re-implements decompression, framing or the envelope. A single
// minTea codec is held because the envelope decode needs one; it is the same codec
// type the request path already uses, and it is passed in rather than constructed
// so a caller can share one.
//
// Partial delivery is the normal case, not an edge case: a frame can arrive across
// many reads. Feed() therefore buffers and returns how many complete messages are
// now available, and only a real protocol error latches failure.

#include "CompressionCodec.h"
#include "NetworkConnection.h"
#include "LoginResponseProtocol.h"
#include "NetCompressCodec.h"
#include "NetworkCodec.h"
#include "NetworkTypes.h"

#include "types/Result.h"

#include <cstddef>
#include <string>
#include <vector>

namespace Modern::Client
{
	// The login states this milestone actually traces.
	//
	// Mirrors Modern::Server::LoginPhase. Duplicated rather than shared because the
	// client must not depend on the server, and vice versa - the two are peers on the
	// wire, not a caller and a callee.
	enum class LoginPhase : std::uint8_t
	{
		Disconnected = 0,
		LoginSent,
		LoginResponseReceived,
		LoginAccepted,
		LoginRejected,
	};

	const char* ToString(LoginPhase phase) noexcept;

	// Receives and interprets the login response.
	//
	// Not a state-machine framework and not a socket owner. It takes bytes; the
	// transport stays the caller's (V027's LoopbackTransport in tests, a real socket
	// later).
	class LoginResponseClient
	{
	public:
		explicit LoginResponseClient(Network::MinLzo1xCodec& codec) noexcept
			: m_codec(codec)
		{
		}

		// Records that the request went out, moving the phase to LoginSent.
		void NoteLoginSent() noexcept { m_phase = LoginPhase::LoginSent; }

		LoginPhase Phase() const noexcept { return m_phase; }

		// The last decoded response. Valid once the phase has reached
		// LoginResponseReceived or beyond.
		const Network::LoginFeedback& Feedback() const noexcept { return m_feedback; }

		// Feeds bytes received from the transport.
		//
		// Returns the number of complete inner messages now available, or an error.
		// Envelope problems are reported but are NOT fatal in the way a framing
		// problem is: a malformed envelope is discarded, because a peer may send a
		// bad batch and the next one may be fine.
		Status Feed(const Network::WireU8* data, std::size_t size,
		                     std::size_t& messagesHandled);

		// Same, over a vector.
		Status Feed(const std::vector<Network::WireU8>& data,
		                     std::size_t& messagesHandled)
		{
			return Feed(data.data(), data.size(), messagesHandled);
		}

		// True once the framer has latched an unrecoverable framing error. The
		// caller must drop the connection; a desynchronised stream cannot resync.
		bool IsFailed() const noexcept { return m_framer.IsFailed(); }

		// Errors from malformed envelopes, for logging. Cleared by Reset.
		std::size_t EnvelopeErrorCount() const noexcept { return m_envelopeErrors; }

		void Reset() noexcept
		{
			m_framer.Reset();
			m_pending.clear();
			m_phase = LoginPhase::Disconnected;
			m_feedback = Network::LoginFeedback{};
			m_envelopeErrors = 0;
		}

		// The ceiling for decompressed batch size, passed to the envelope decoder.
		// A frame claiming more than this is refused rather than allocated.
		static constexpr std::size_t kMaxDecompressed = Network::kMaxDecompressedBatch;

	private:
		// Consumes every complete NET_COMPRESS envelope at the front of m_pending,
		// feeding each inner stream to the framer, and leaves any partial trailing
		// frame buffered for the next read.
		//
		// Returns Ok with nothing done when more bytes are needed, which is the
		// normal case rather than an error.
		Status ConsumeEnvelopes();

		Network::MinLzo1xCodec& m_codec;
		Network::ConnectionFramer m_framer;
		LoginPhase                m_phase = LoginPhase::Disconnected;
		Network::LoginFeedback    m_feedback{};
		std::size_t               m_envelopeErrors = 0;

		// Transport bytes not yet consumed, and not yet classifiable.
		//
		// Held across Feed calls because a NET_COMPRESS envelope can straddle any
		// number of TCP reads. Always cleared once its bytes have been handed to the
		// framer, so it never replays them.
		std::vector<Network::WireU8> m_pending;
	};
}