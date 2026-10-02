#pragma once

// VERTICAL-030: the LZO1X compression boundary.
//
// This is the ONLY place in the modern tree that knows LZO exists. Everything above
// it deals in bytes and Status; everything below it is vendored third-party C.
//
// Legacy provenance:
//
//   Compressor   Lib_Network/MinLzo.cpp:95-138
//       lzo1x_1_compress(in, inLen, out, &outLen, workmem)
//       then: if (outLen >= inLen) -> MINLZO_CAN_NOT_COMPRESS
//
//   Decompressor Lib_Network/MinLzo.cpp:140-175
//       lzo1x_decompress_safe(compressed, compressedLen, dest, &destLen, NULL)
//       then: if (destLen >= compressedLen) -> failure
//
// Two legacy rules are load-bearing and are reproduced exactly, because the wire
// depends on them:
//
//   1. COMPRESSION IS FALLIBLE AND THE FALLBACK IS LEGAL. Legacy treats
//      "compressed output is not smaller than the input" as a distinct outcome, not
//      an error, and the caller responds by shipping the batch UNCOMPRESSED but still
//      wrapped in a NET_COMPRESS envelope with the flag clear
//      (SendMsgBuffer.cpp:118-166). A codec that "succeeds" on incompressible data
//      and returns it anyway would produce a peer-visible difference: the peer's
//      bCompress flag would disagree with the payload.
//
//   2. `lzo1x_decompress_safe` treats its length argument as IN/OUT. On entry it is
//      the destination CAPACITY; on exit it is the bytes written. Legacy passes
//      NET_DATA_BUFSIZE in (RcvMsgBuffer.cpp:125, the "packet crash fix") and passes
//      NULL for the optional dictionary. Getting this backwards does not crash - it
//      returns LZO_E_INPUT_OVERRUN - which is exactly the kind of bug that survives
//      into production because nothing threw.
//
// The decompressed-size check in legacy (`destLen >= compressedLen`) is NOT
// reproduced: it is a sanity check that happens to hold for compressible input and
// wrongly rejects valid results for incompressible input. The modern layer validates
// the declared size against the actual output instead, which is stricter and correct.

#include "types/Result.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Modern::Network
{
	// Why a compression attempt did not yield compressed bytes.
	//
	// A distinct code rather than a bool, because the caller must react
	// differently: `NotCompressible` is a legitimate protocol outcome that becomes a
	// valid envelope with the flag clear, whereas a real error is a failure.
	enum class CompressionOutcome : uint8_t
	{
		// Compressed output produced and strictly smaller than the input.
		Compressed = 0,

		// LZO succeeded but the output was not smaller than the input. Legacy's
		// MINLZO_CAN_NOT_COMPRESS. NOT an error - the batch ships uncompressed.
		NotCompressible,

		// LZO rejected the input, or the codec is not usable at all.
		Failed,
	};

	// The narrow codec interface.
	//
	// Injected rather than global so tests can substitute a deterministic backend and
	// so the vendored LZO can be replaced without touching any caller. There is no
	// `CompressEverything()` entry point: direction is not a parameter here because
	// this codec is only ever used on the server-to-client path, and that fact is
	// enforced one layer up in NetCompressCodec rather than by a bool flag.
	class Lzo1xCodec
	{
	public:
		virtual ~Lzo1xCodec() = default;

		// Compresses `input` into `out`.
		//
		// `out` is resized as needed and the bytes written are returned. On
		// NotCompressible, `out` is left empty and the caller must use the original
		// bytes - it is never a case where the codec invents something.
		virtual CompressionOutcome Compress(const std::vector<uint8_t>& input,
		                                    std::vector<uint8_t>& out) = 0;

		// Decompresses `input` into at most `maxOutput` bytes.
		//
		// `maxOutput` is a hard ceiling the decoder must not exceed, not a hint. It is
		// passed through to `lzo1x_decompress_safe` as the destination capacity, which
		// is what makes that function "safe" rather than merely "best effort".
		virtual Status Decompress(const std::vector<uint8_t>& input,
		                          std::size_t maxOutput,
		                          std::vector<uint8_t>& out) = 0;
	};

	// The real backend, over vendored miniLZO 2.10.
	//
	// A class rather than free functions because LZO1X-1 compression needs a work
	// buffer, and a function-local static would be shared mutable state across
	// threads. Instances are cheap but NOT internally synchronised; a server with
	// concurrent senders should hold one per thread.
	//
	// See modern/network/CompressionCodec.cpp and third_party/minilzo/VENDOR.md.
	class MinLzo1xCodec final : public Lzo1xCodec
	{
	public:
		MinLzo1xCodec();
		~MinLzo1xCodec() override;

		MinLzo1xCodec(const MinLzo1xCodec&) = delete;
		MinLzo1xCodec& operator=(const MinLzo1xCodec&) = delete;

		CompressionOutcome Compress(const std::vector<uint8_t>& input,
		                            std::vector<uint8_t>& out) override;

		Status Decompress(const std::vector<uint8_t>& input,
		                  std::size_t maxOutput,
		                  std::vector<uint8_t>& out) override;

		// True when LZO initialised and a work buffer exists. A false codec fails
		// every call with InvalidState rather than misbehaving.
		bool IsReady() const noexcept { return m_ready; }

	private:
		void* m_workmem = nullptr;
		bool  m_ready   = false;
	};

	// The ceiling on decompressed batch size.
	//
	// Legacy decompresses into NET_DATA_BUFSIZE (RcvMsgBuffer.cpp:125). The client
	// receive buffer is larger (NET_DATA_CLIENT_MSG_BUFSIZE, 16384), so a batch may
	// legitimately exceed the 2048 single-message cap - which is precisely why the
	// batch and the per-message limits are different numbers.
	constexpr std::size_t kMaxDecompressedBatch = 16384;
}