#include "login/LoginResponseClient.h"

namespace Modern::Client
{
	using namespace Modern::Network;

	const char* ToString(LoginPhase phase) noexcept
	{
		switch (phase)
		{
		case LoginPhase::Disconnected:          return "Disconnected";
		case LoginPhase::LoginSent:             return "LoginSent";
		case LoginPhase::LoginResponseReceived: return "LoginResponseReceived";
		case LoginPhase::LoginAccepted:         return "LoginAccepted";
		case LoginPhase::LoginRejected:         return "LoginRejected";
		}
		return "Unrecognised";
	}

	// The largest envelope worth buffering: the compressed ceiling for a full batch
	// plus the envelope header. Anything claiming more is refused rather than
	// accumulated, so a hostile dwSize cannot grow this buffer without bound.
	const std::size_t kMaxEnvelopeSize =
	    Network::NetCompress::MaxCompressedSize(LoginResponseClient::kMaxDecompressed) +
	    Network::kCompressEnvelopeSize;

	Status LoginResponseClient::Feed(const WireU8* data, std::size_t size,
	                                std::size_t& messagesHandled)
	{
		messagesHandled = 0;

		if (data == nullptr && size != 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// Accumulate first, decide afterwards.
		//
		// The envelope declares its own total size, so a TCP read boundary has no
		// meaning to this protocol: a frame may arrive across any number of reads.
		// Classifying each read as it arrives - rather than the accumulated stream -
		// is the bug this replaces, because a one-byte read cannot be classified at
		// all and would otherwise be dropped.
		if (size != 0)
		{
			m_pending.insert(m_pending.end(), data, data + size);
		}

		// Fewer than 8 bytes cannot be classified (that is the header), so they stay
		// buffered until more arrive.
		if (m_pending.size() >= Network::kMessageHeaderSize)
		{
			const MessageId type = Codec::ReadU32(m_pending.data() + 4);

			if (!Network::NetCompress::IsEnvelope(type))
			{
				// Not enveloped: a raw NET_MSG_GENERIC stream, which the framer owns.
				//
				// The buffer is cleared after every raw feed precisely so the next
				// read does not replay bytes the framer has already consumed.
				if (const FrameStatus fed = m_framer.Feed(m_pending.data(), m_pending.size());
				    fed != FrameStatus::Ok)
				{
					m_pending.clear();
					return Status(ErrorCode::InvalidArgument);
				}
				m_pending.clear();
			}
			else if (const Status status = ConsumeEnvelopes(); status.IsError())
			{
				return status;
			}
		}

		// Drain whatever inner messages are now complete.
		Message message;
		while (m_framer.Next(message) == FrameStatus::Ok)
		{
			++messagesHandled;

			// A batch may carry messages this handler does not own. Skipping them
			// keeps the handler honest rather than guessing at ids it was not given.
			//
			// The test is on header.type, NOT on the payload: Message::payload is the
			// body only, and the id lives in the 8-byte header ahead of it.
			if (message.header.type != Network::LoginResponse::kLoginFeedbackMessageId)
			{
				continue;
			}

			// Reassemble the full frame for the codec, which validates the declared
			// size against the message it was handed.
			std::vector<WireU8> frame;
			frame.reserve(static_cast<std::size_t>(message.header.size) + Network::kMessageHeaderSize);
			Codec::WriteU32(frame, message.header.size);
			Codec::WriteU32(frame, message.header.type);
			frame.insert(frame.end(), message.payload.begin(), message.payload.end());

			LoginFeedback decoded;
			if (const Status status = Network::LoginResponse::Decode(frame.data(), frame.size(), decoded);
			    status.IsError())
			{
				return status;
			}

			m_feedback = decoded;

			// A response WAS received either way; the terminal state then records
			// whether it was accepted.
			m_phase = LoginPhase::LoginResponseReceived;
			m_phase = decoded.IsSuccess() ? LoginPhase::LoginAccepted
			                              : LoginPhase::LoginRejected;
		}

		if (m_framer.IsFailed())
		{
			return Status(ErrorCode::InvalidArgument);
		}
		return Ok();
	}

	Status LoginResponseClient::ConsumeEnvelopes()
	{
		// Consumes every COMPLETE envelope at the front of the buffer, leaving a
		// partial trailing frame for the next read.
		while (m_pending.size() >= Network::kCompressEnvelopeSize)
		{
			const WireU32    declared = Codec::ReadU32(m_pending.data());
			const std::size_t declaredSize = static_cast<std::size_t>(declared);

			// An impossible size cannot be waited out, and a self-describing-length
			// stream cannot be resynchronised: the next boundary is unknowable. Refuse
			// it and drop what we hold rather than growing without bound.
			if (declaredSize < Network::kCompressEnvelopeSize || declaredSize > kMaxEnvelopeSize)
			{
				++m_envelopeErrors;
				m_pending.clear();
				return Ok();
			}

			// Header present, body still arriving. Not an error.
			if (m_pending.size() < declaredSize)
			{
				return Ok();
			}

			std::vector<WireU8> inner;
			const Status status = Network::NetCompress::DecodeServerToClientEnvelope(
			    m_codec, m_pending.data(), declaredSize, kMaxDecompressed, inner);

			// The envelope is consumed either way, so a malformed one cannot wedge
			// the stream. Legacy likewise drops the frame and moves on
			// (RcvMsgBuffer.cpp:145-160).
			m_pending.erase(m_pending.begin(), m_pending.begin() + declaredSize);

			if (status.IsError())
			{
				++m_envelopeErrors;
				continue;
			}

			if (const FrameStatus fed = m_framer.Feed(inner.data(), inner.size()); fed != FrameStatus::Ok)
			{
				return Status(ErrorCode::InvalidArgument);
			}
		}
		return Ok();
	}
}