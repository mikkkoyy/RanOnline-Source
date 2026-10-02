#pragma once

// VERTICAL-027: message framing.
//
// Framing owns message BOUNDARIES and nothing else. It accumulates whatever the
// transport hands it, and yields whole, validated messages. It does not decode
// payloads, does not route, and does not know what a message means.
//
// This is the layer that most often gets skipped and most often gets written
// twice. Legacy has one, in RcvMsgBuffer.cpp, and it is worth reading: the
// receive loop at :90-118 is the reference for what "a valid header" means.
//
//     | Type1 | Size1 | Data1 | Type2 | Size2 | Data2 | ...
//     ^                       ^
//     m_nPos                  m_nPos + Size1

#include "NetworkCodec.h"
#include "NetworkTransport.h"

#include "types/Result.h"

#include <cstring>
#include <vector>

namespace Modern::Network
{
	// Why a frame could not be produced.
	//
	// `NeedMoreData` is not a failure - it means the peer has not finished
	// sending. It is an enum value rather than a Status because `Status` has no
	// "try again later" member, and overloading `NotFound` to mean it would make
	// a caller's error handling lie.
	enum class FrameStatus : uint8_t
	{
		Ok = 0,
		NeedMoreData,
		InvalidLength,
		Oversized,
	};

	// Accumulates bytes and yields whole messages.
	//
	// Capacity is fixed at construction, bounded by Protocol::kMaxPacketSize.
	// That bound is the whole security story of this class: a peer that never
	// finishes a message cannot make the buffer grow without limit, because it
	// cannot exceed one.
	class ConnectionFramer
	{
	public:
		explicit ConnectionFramer(std::size_t capacity = Protocol::kMaxPacketSize) noexcept
			: m_capacity(capacity < kMessageHeaderSize ? kMessageHeaderSize : capacity)
		{
		}

		// Appends received bytes. Returns Oversized rather than truncating if the
		// peer would push the buffer past capacity - silently discarding bytes
		// would desynchronise every message that follows.
		FrameStatus Feed(const WireU8* data, std::size_t size) noexcept
		{
			if (data == nullptr && size != 0)
			{
				return FrameStatus::InvalidLength;
			}
			if (m_buffer.size() + size > m_capacity)
			{
				return FrameStatus::Oversized;
			}
			m_buffer.insert(m_buffer.end(), data, data + size);
			return FrameStatus::Ok;
		}

		// Pops the next whole message, if one is buffered.
		//
		// A header that cannot be valid (size below the header, above the
		// protocol maximum, or a zero type) latches the framer into a failed
		// state: the byte stream is no longer trustworthy, because we no longer
		// know where the next message starts. `IsFailed()` reports that, and the
		// owner must drop the connection rather than keep parsing.
		FrameStatus Next(Message& out) noexcept
		{
			if (m_failed)
			{
				return FrameStatus::InvalidLength;
			}
			if (m_buffer.size() < kMessageHeaderSize)
			{
				return FrameStatus::NeedMoreData;
			}

			const WireU32 declaredSize = Codec::ReadU32(m_buffer.data());
			const WireU32 declaredType = Codec::ReadU32(m_buffer.data() + 4);

			if (declaredType == 0 ||
			    declaredSize < kMessageHeaderSize ||
			    declaredSize > Protocol::kMaxPacketSize)
			{
				m_failed = true;
				return FrameStatus::InvalidLength;
			}

			if (m_buffer.size() < declaredSize)
			{
				return FrameStatus::NeedMoreData;
			}

			if (const Status status = Codec::DecodeMessage(m_buffer.data(), m_buffer.size(), out);
			    status.IsError())
			{
				m_failed = true;
				return FrameStatus::InvalidLength;
			}

			m_buffer.erase(m_buffer.begin(),
			               m_buffer.begin() + static_cast<std::ptrdiff_t>(declaredSize));
			return FrameStatus::Ok;
		}

		// True once an unrecoverable framing error has been seen.
		bool IsFailed() const noexcept { return m_failed; }

		// Bytes buffered but not yet forming a whole message.
		std::size_t Buffered() const noexcept { return m_buffer.size(); }

	// Total buffer capacity chosen at construction.
	//
	// Exposed so a caller that receives large bursts can feed in slices bounded by
	// the room actually left, draining between them, instead of handing over a
	// chunk larger than capacity and having Feed() report Oversized. Capacity
	// bounds BUFFERED PARTIAL DATA, not total stream length, so a peer may legally
	// send far more than this overall.
	std::size_t Capacity() const noexcept { return m_capacity; }

		void Reset() noexcept
		{
			m_buffer.clear();
			m_failed = false;
		}

	private:
		std::vector<WireU8> m_buffer;
		std::size_t         m_capacity;
		bool                m_failed = false;
	};
}
