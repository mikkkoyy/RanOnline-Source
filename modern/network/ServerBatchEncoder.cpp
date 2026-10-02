#include "ServerBatchEncoder.h"

namespace Modern::Network
{
	bool ServerBatchEncoder::Emit(std::vector<WireU8>& pending)
	{
		pending.clear();

		if (m_batch.empty())
		{
			return false;
		}

		const bool ok = NetCompress::EncodeServerToClientBatch(m_codec, m_batch, pending).IsOk();

		// Cleared regardless of outcome: a batch that cannot be encoded is a fault to
		// report upward, not a reason to retry it forever on every tick.
		m_batch.clear();
		m_count = 0;

		return ok;
	}

	BatchAction ServerBatchEncoder::Add(const std::vector<WireU8>& innerMessage,
	                                    std::vector<WireU8>& pending)
	{
		pending.clear();

		if (innerMessage.size() < kMessageHeaderSize)
		{
			return BatchAction::Buffered; // nothing sensible to do with a fragment
		}

		const std::size_t dwTotal = m_batch.size() + innerMessage.size();

		// Strictly less than. A batch of exactly COMPRESS_PACKET_SIZE does not force
		// a flush here; it waits for the next message or for the end-of-tick Flush.
		if (dwTotal < Protocol::kCompressThreshold)
		{
			m_batch.insert(m_batch.end(), innerMessage.begin(), innerMessage.end());
			++m_count;
			return BatchAction::Buffered;
		}

		if (m_batch.empty())
		{
			// Nothing batched, so this message alone crosses the threshold. Legacy
			// sends it by itself rather than waiting for a partner.
			m_batch = innerMessage;
			m_count = 1;
			Emit(pending);
			return BatchAction::FlushOnly;
		}

		// Something is batched: flush what we have WITHOUT this message, then let the
		// caller re-add it. Legacy signals exactly this with BUFFER_SEND_ADD.
		Emit(pending);
		m_batch = innerMessage;
		m_count = 1;
		return BatchAction::FlushPending;
	}

	bool ServerBatchEncoder::Flush(std::vector<WireU8>& pending)
	{
		return Emit(pending);
	}
}