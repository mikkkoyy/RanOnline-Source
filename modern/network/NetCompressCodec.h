#pragma once

// VERTICAL-030: the NET_COMPRESS envelope.
//
// Direction matters and is not negotiable. Legacy compresses on ONE side only:
//
//   SERVER -> CLIENT   batched, LZO-compressed, wrapped in a NET_COMPRESS envelope
//   CLIENT -> SERVER   raw NET_MSG_GENERIC, one per send, no envelope, no compression
//
// Asymmetry established by V028 (SendMsgBuffer.cpp:116-172 and
// CClientManager::SendClientFinal at :420-433) and V029
// (CNetClient::SendBuffer2 at s_NetClient.cpp:815-831, which passes dwSize straight
// to ::send with no wrapping).
//
// Hence the two deliberately un-symmetric names below. There is no generic
// `Encode`/`Decode` pair, because a generic pair is exactly how a client->server
// login request ends up wrapped in a compression envelope that the server will
// reject. Client->server framing belongs to ConnectionFramer and nothing else.
//
// The envelope layout, from the compiled struct - not from the stale diagram:
//
//   s_NetGlobal.h:2811-2819
//       struct NET_COMPRESS { NET_MSG_GENERIC nmg; bool bCompress; };
//
//   offset 0  u32  dwSize      total frame bytes, envelope INCLUDED
//   offset 4  u32  nType       170, NET_MSG_COMPRESS
//   offset 8  u8   bCompress   1 = LZO payload, 0 = raw payload
//   offset 9  ..   payload
//
// There is NO Count field. SendMsgBuffer.h:33 shows a diagram reading
// `| Size(4) | Type(4) | Compress(1) | Count(2) | Data(...) |`, and that Count(2) is
// a stale Doxygen comment contradicted by the struct, by the constructor, and by
// every write site. The batch carries no count: it is self-delimiting because each
// inner NET_MSG_GENERIC declares its own size. V028 recorded this trap and V027
// quoted the diagram without flagging it; nothing here may reintroduce a count.

#include "CompressionCodec.h"
#include "NetworkTypes.h"

#include "types/Result.h"

#include <cstddef>
#include <vector>

namespace Modern::Network
{
	// Envelope geometry.
	//
	// sizeof(NET_COMPRESS) is 12 on x86 with MSVC's 1-byte bool: 8 header bytes plus
	// 1 flag byte, padded to the struct's 4-byte alignment. The padding is real wire
	// data - legacy memcpys the struct, so bytes 9-11 are whatever the compiler left
	// there, and dwSize counts them. This codec therefore always emits 12, never 9.
	constexpr std::size_t kCompressEnvelopeSize = 12;

	static_assert(kCompressEnvelopeSize == kMessageHeaderSize + 1 + 3,
	              "NET_COMPRESS is the 8-byte header plus a bool padded to 4-byte alignment");

	// Parsed envelope. The payload is left in the caller's buffer; only the header
	// and the payload extent are described here, because the payload is either
	// compressed or not and the two cases are decoded by different code.
	struct CompressEnvelope
	{
		WireU32    size        = 0;   // total frame size, envelope included
		MessageId  type        = 0;   // always WellKnownMessage::kCompress
		bool       compressed  = false;
		std::size_t payloadOffset = 0; // kCompressEnvelopeSize
		std::size_t payloadSize   = 0; // size - payloadOffset
	};

	namespace NetCompress
	{
		// Validates and describes one envelope at the front of `bytes`.
		//
		// This is the security boundary for untrusted network input and it rejects,
		// without allocating:
		//   - a buffer shorter than the envelope
		//   - an nType that is not NET_MSG_COMPRESS (170)
		//   - a dwSize smaller than the envelope
		//   - a dwSize larger than what was actually received (truncation)
		//   - a zero payload, which would carry no message at all
		//
		// It does NOT validate the payload's contents. An envelope claiming to be
		// compressed has not been proven so until LZO has accepted it, and a raw
		// payload's inner messages are not proven until the framer has read them.
		// Those are separate stages and are validated separately.
		Status DecodeEnvelope(const WireU8* bytes, std::size_t available,
		                      CompressEnvelope& out);

		// SERVER -> CLIENT. Wraps a batch of already-serialised inner messages in an
		// envelope, compressing when that is possible and beneficial.
		//
		// `inner` is a concatenation of complete NET_MSG_GENERIC messages. It is
		// transmitted verbatim (possibly compressed); this function never parses it,
		// because parsing is the framer's job and mixing the two would hide exactly
		// the boundary bugs the layering exists to expose.
		//
		// When LZO cannot help, legacy ships the batch RAW WITH THE ENVELOPE STILL
		// AROUND IT and the flag clear (SendMsgBuffer.cpp:151-166). That is what this
		// does, and it is not a fallback for its own sake: the receiver keys its
		// decode path off the flag, so a raw batch MUST still be enveloped.
		Status EncodeServerToClientBatch(Lzo1xCodec& codec,
		                                 const std::vector<WireU8>& inner,
		                                 std::vector<WireU8>& out);

		// CLIENT RECEIVE. Unwraps an envelope into the inner message stream.
		//
		// On success `out` holds a concatenation of complete NET_MSG_GENERIC messages,
		// ready to be fed to ConnectionFramer. It is NOT validated here beyond what
		// LZO's "safe" decoder guarantees; per-message bounds are the framer's check.
		//
		// `maxOutput` bounds the decompressed size. A hostile or truncated envelope
		// cannot make this allocate beyond it.
		Status DecodeServerToClientEnvelope(Lzo1xCodec& codec,
		                                    const WireU8* bytes,
		                                    std::size_t available,
		                                    std::size_t maxOutput,
		                                    std::vector<WireU8>& out);

		// The compressed-payload upper bound used when sizing a scratch buffer.
		// LZO1X-1 output never exceeds input plus 1/16 plus a small constant; the
		// envelope adds its own 12 bytes on top.
		std::size_t MaxCompressedSize(std::size_t innerSize) noexcept;

		// Whether `type` is the compression envelope id. Exposed so a receiver can
		// dispatch without re-reading the header.
		constexpr bool IsEnvelope(MessageId type) noexcept
		{
			return type == WellKnownMessage::kCompress;
		}
	}
}