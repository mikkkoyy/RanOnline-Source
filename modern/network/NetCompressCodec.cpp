#include "NetCompressCodec.h"

// For Codec::ReadU32 / WriteU32 / WriteU8. The header only needs the wire types,
// so this stays in the implementation rather than widening the header's includes.
#include "NetworkCodec.h"

namespace Modern::Network
{
	namespace
	{
		// Writes the 12-byte envelope prefix: dwSize, nType, flag, and the 3 padding
		// bytes legacy's struct layout contributes.
		void WriteEnvelopeHeader(std::vector<WireU8>& out, WireU32 totalSize, bool compressed)
		{
			Codec::WriteU32(out, totalSize);
			Codec::WriteU32(out, WellKnownMessage::kCompress);
			Codec::WriteU8(out, compressed ? 1u : 0u);

			// Padding to the struct's 4-byte alignment. Legacy memcpys NET_COMPRESS,
			// so these bytes exist on the wire and dwSize counts them. Zeroed here
			// because legacy leaves them indeterminate - any fixed value is equally
			// compatible, and zero is reproducible.
			Codec::WriteU8(out, 0u);
			Codec::WriteU8(out, 0u);
			Codec::WriteU8(out, 0u);
		}
	}

	std::size_t NetCompress::MaxCompressedSize(std::size_t innerSize) noexcept
	{
		// Mirrors the sizing in MinLzo1xCodec::Compress: input + 1/16 + slack.
		return innerSize + (innerSize / 16) + 64 + 3;
	}

	Status NetCompress::DecodeEnvelope(const WireU8* bytes, std::size_t available,
	                                   CompressEnvelope& out)
	{
		out = CompressEnvelope{};

		if (bytes == nullptr || available < kCompressEnvelopeSize)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		const WireU32 declaredSize = Codec::ReadU32(bytes);
		const MessageId declaredType = Codec::ReadU32(bytes + 4);

		if (declaredType != WellKnownMessage::kCompress)
		{
			return Status(ErrorCode::InvalidArgument);
		}
		if (declaredSize < kCompressEnvelopeSize)
		{
			return Status(ErrorCode::InvalidArgument);
		}
		if (declaredSize > available)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		const std::size_t payloadSize = declaredSize - kCompressEnvelopeSize;
		if (payloadSize == 0)
		{
			// An envelope carrying nothing decodes to nothing and would spin a
			// receiver. Legacy would produce one here too, but refusing costs nothing.
			return Status(ErrorCode::InvalidArgument);
		}

		out.size           = declaredSize;
		out.type           = declaredType;
		out.compressed     = bytes[8] != 0;
		out.payloadOffset  = kCompressEnvelopeSize;
		out.payloadSize    = payloadSize;
		return Ok();
	}

	Status NetCompress::EncodeServerToClientBatch(Lzo1xCodec& codec,
	                                              const std::vector<WireU8>& inner,
	                                              std::vector<WireU8>& out)
	{
		out.clear();

		if (inner.empty())
		{
			return Status(ErrorCode::InvalidArgument);
		}
		// A batch that could not be described by dwSize would silently truncate.
		const std::size_t envelopeCeiling =
			Protocol::kMaxPacketSize > Protocol::kDataClientMessageBufferSize
				? Protocol::kMaxPacketSize
				: Protocol::kDataClientMessageBufferSize;

		std::vector<WireU8> compressed;
		const CompressionOutcome outcome = codec.Compress(inner, compressed);

		if (outcome == CompressionOutcome::Compressed &&
		    kCompressEnvelopeSize + compressed.size() <= envelopeCeiling)
		{
			out.reserve(kCompressEnvelopeSize + compressed.size());
			WriteEnvelopeHeader(out,
			                    static_cast<WireU32>(kCompressEnvelopeSize + compressed.size()),
			                    true);
			out.insert(out.end(), compressed.begin(), compressed.end());
			return Ok();
		}

		// Legacy's CAN_NOT_COMPRESS path (SendMsgBuffer.cpp:151-166): the batch goes
		// out RAW, still enveloped, flag clear. Reached when LZO declines, when it
		// fails, and when compressing would not fit - all three are legitimate.
		if (kCompressEnvelopeSize + inner.size() > envelopeCeiling)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		out.reserve(kCompressEnvelopeSize + inner.size());
		WriteEnvelopeHeader(out,
		                    static_cast<WireU32>(kCompressEnvelopeSize + inner.size()),
		                    false);
		out.insert(out.end(), inner.begin(), inner.end());
		return Ok();
	}

	Status NetCompress::DecodeServerToClientEnvelope(Lzo1xCodec& codec,
	                                                 const WireU8* bytes,
	                                                 std::size_t available,
	                                                 std::size_t maxOutput,
	                                                 std::vector<WireU8>& out)
	{
		out.clear();

		if (maxOutput == 0 || maxOutput > kMaxDecompressedBatch)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		CompressEnvelope envelope;
		if (const Status status = DecodeEnvelope(bytes, available, envelope); status.IsError())
		{
			return status;
		}

		const WireU8* payload = bytes + envelope.payloadOffset;

		if (!envelope.compressed)
		{
			// Raw batch. Legacy returns here without LZO (RcvMsgBuffer.cpp:165-183).
			// The payload is bounded by maxOutput exactly as the compressed path is,
			// so a large declared frame cannot force an oversized copy.
			if (envelope.payloadSize > maxOutput)
			{
				return Status(ErrorCode::InvalidArgument);
			}
			out.assign(payload, payload + envelope.payloadSize);
			return Ok();
		}

		std::vector<WireU8> compressedPayload(payload, payload + envelope.payloadSize);

		// `maxOutput` is the destination capacity handed to the safe decoder. A
		// truncated or corrupt LZO stream fails here and never writes past it.
		if (const Status status = codec.Decompress(compressedPayload, maxOutput, out);
		    status.IsError())
		{
			out.clear();
			return status;
		}

		return Ok();
	}
}