#pragma once

// VERTICAL-030: server->client batching.
//
// What COMPRESS_PACKET_SIZE actually means. Legacy declares it as 1000
// (SendMsgBuffer.h:43) and the easy misreading is "the maximum size of a compressed
// packet". It is not. It is a FLUSH TRIGGER, and the distinction has an observable
// consequence on the wire.
//
// CSendMsgBuffer::addMsg (SendMsgBuffer.cpp:65-113) does exactly this:
//
//     dwTotal = m_dwPos + dwSize;                  // bytes already batched
//     if (dwTotal < COMPRESS_PACKET_SIZE)          // strictly less than 1000
//         append; keep buffering                    -> BUFFER_ADDED
//     else if (m_dwPos == 0)                       // nothing batched yet
//         batch this message ALONE; flush           -> BUFFER_SEND
//     else                                         // something is batched
//         flush the batch WITHOUT this message      -> BUFFER_SEND_ADD
//                                                  // then start a new batch with it
//
// and CClientManager::SendClientFinal (s_CClientManager.cpp:420-433) calls
// getSendSize() for EVERY connection each tick, unconditionally.
//
// Three consequences, all of which this reproduces:
//
//   1. 1000 is a lower bound on when a flush is FORCED, not a cap. A batch that
//      stops at 999 bytes is still transmitted - at the end of the tick. Threshold
//      tests must therefore assert *flush timing*, never "this size is rejected".
//
//   2. Buffering requires `dwTotal < 1000`, so the flush side BEGINS AT 1000.
//      999 buffers; 1000 and 1001 both flush. Getting this one byte wrong is the
//      difference between "almost" wire-compatible and not, and it manifests only
//      as a frame-count mismatch against a real client - never as a crash.
//
//   3. A single message at or over the threshold flushes on its own rather than
//      waiting for a partner, so batches are bounded by the largest single message
//      rather than by the threshold.
//
// Legacy returns BUFFER_SEND_ADD from a function that has no output parameter for
// the payload, which is why the intended driver survives only as a comment
// (SendMsgBuffer.cpp:39-62). This class makes it explicit and returns what to do.
//
// Not thread-safe, by design: a batching buffer is per-connection state, and the
// legacy original guarded it with a CRITICAL_SECTION per connection. Hoist the lock
// to the connection, do not hide one here.

#include "CompressionCodec.h"
#include "NetCompressCodec.h"
#include "NetworkTypes.h"

#include <cstddef>
#include <vector>

namespace Modern::Network
{
	// What the caller should do after offering a message to the batcher.
	enum class BatchAction : uint8_t
	{
		// Buffered. Nothing to send yet.
		Buffered = 0,

		// `pending` holds a frame that must go out now, and the offered message was
		// NOT part of it. Add the message again after sending.
		FlushPending,

		// `pending` holds a frame that must go out now, and the offered message WAS
		// the whole batch.
		FlushOnly,
	};

	// Accumulates complete, already-serialised NET_MSG_GENERIC messages and emits
	// NET_COMPRESS frames.
	//
	// SERVER -> CLIENT ONLY. Client->server traffic never passes through here; it
	// goes straight to ConnectionFramer as raw messages, because legacy
	// CNetClient::SendBuffer2 does exactly that. Offering this class a message it is
	// not meant to wrap is the mistake the class name exists to prevent.
	class ServerBatchEncoder
	{
	public:
		// `codec` must outlive this object. Borrowed, not owned.
		explicit ServerBatchEncoder(Lzo1xCodec& codec) noexcept
			: m_codec(codec)
		{
		}

		// Offers one complete inner message.
		//
		// `innerMessage` must be a whole NET_MSG_GENERIC, header included - the same
		// bytes that would go on the wire unwrapped. Its declared size is read to
		// decide when to flush; it is NOT re-encoded, so a caller that builds
		// messages with Codec::EncodeMessage and appends the result is consistent by
		// construction.
		BatchAction Add(const std::vector<WireU8>& innerMessage,
		                 std::vector<WireU8>& pending);

		// Emits whatever is batched, even if below the threshold. Call once per tick
		// per connection, mirroring SendClientFinal. Returns false when the batch is
		// empty, which is normal and not an error.
		bool Flush(std::vector<WireU8>& pending);

		// Bytes currently batched.
		std::size_t Buffered() const noexcept { return m_batch.size(); }

		// Messages currently batched.
		std::size_t Count() const noexcept { return m_count; }

		// Discards the batch without emitting. Used when a connection is torn down.
		void Reset() noexcept
		{
			m_batch.clear();
			m_count = 0;
		}

	private:
		// Wraps the current batch in an envelope and clears it.
		bool Emit(std::vector<WireU8>& pending);

		Lzo1xCodec&        m_codec;
		std::vector<WireU8> m_batch;
		std::size_t        m_count = 0;
	};
}