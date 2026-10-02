// VERTICAL-030: server->client NET_COMPRESS + LZO1X protocol layer.
//
// Deterministic throughout: no socket, no clock, no RNG, no real network, no
// external server. The only dependency under test is a deterministic byte pattern.
//
// What is being pinned, and why each one is a wire fact rather than a style choice,
// is documented at each test. The short version: the compression layer exists on
// ONE side only, 1000 is a flush TRIGGER rather than a size cap, and the
// "uncompressed" case is still wrapped in an envelope.
//
// The LZO payload itself is validated SEMANTICALLY - decode and compare - rather
// than against fixed bytes. Compression output is not guaranteed identical across
// builds or settings, so asserting exact compressed bytes would encode an
// accident as a requirement. Envelope header bytes ARE asserted exactly, because
// those are fixed by the legacy struct layout and nothing else.

#include "TestHarness.h"

#include "CompressionCodec.h"
#include "NetCompressCodec.h"
#include "NetworkCodec.h"
#include "NetworkConnection.h"
#include "NetworkTypes.h"
#include "ServerBatchEncoder.h"

namespace
{
	using namespace Modern;
	using namespace Modern::Network;

	// A codec over real miniLZO, built per test so no state leaks between them.
	MinLzo1xCodec& Lzo()
	{
		static MinLzo1xCodec instance;
		return instance;
	}

	// Highly compressible, deterministic: repeated runs of a byte counter. Chosen
	// because a batch of real RAN messages is mostly small ids and repeated
	// structure, so it must actually shrink for the compressed path to be tested
	// at all.
	std::vector<WireU8> Compressible(std::size_t size)
	{
		std::vector<WireU8> bytes(size);
		for (std::size_t i = 0; i < size; ++i)
		{
			bytes[i] = static_cast<WireU8>((i / 7) & 0x0F);
		}
		return bytes;
	}

	// Incompressible: every byte differs from its neighbour, so LZO cannot shrink
	// it. This is how the raw-fallback path gets exercised for real rather than by
	// stubbing the codec.
	std::vector<WireU8> Incompressible(std::size_t size)
	{
		std::vector<WireU8> bytes(size);
		WireU8 state = 0x5A;
		for (std::size_t i = 0; i < size; ++i)
		{
			// xorshift-ish walk: deterministic, no RNG, no long runs.
			state = static_cast<WireU8>(state * 167u + 13u + static_cast<WireU8>(i));
			bytes[i] = state;
		}
		return bytes;
	}

	std::vector<WireU8> MakeMessage(MessageId id, const std::vector<WireU8>& payload = {})
	{
		MessageHeader header;
		header.type = id;
		std::vector<WireU8> bytes;
		(void) Codec::EncodeMessage(header, payload, bytes);
		return bytes;
	}
}

// ---------------------------------------------------------------------------
// Codec
// ---------------------------------------------------------------------------

MODERN_TEST(Compression_RoundTripsDeterministicPayload)
{
	const std::vector<WireU8> original = Compressible(2048);

	std::vector<WireU8> compressed;
	CHECK(Lzo().Compress(original, compressed) == CompressionOutcome::Compressed);
	CHECK(compressed.size() < original.size());

	std::vector<WireU8> restored;
	CHECK(Lzo().Decompress(compressed, kMaxDecompressedBatch, restored).IsOk());
	CHECK(restored == original);
}

MODERN_TEST(Compression_BinaryPayloadRoundTrips)
{
	// Every byte value present, including NUL and 0xFF, because a codec that
	// treats its input as a string would pass the other tests and fail here.
	std::vector<WireU8> original(1024);
	for (std::size_t i = 0; i < original.size(); ++i)
	{
		original[i] = static_cast<WireU8>(i & 0xFF);
	}

	std::vector<WireU8> compressed;
	CHECK(Lzo().Compress(original, compressed) == CompressionOutcome::Compressed);

	std::vector<WireU8> restored;
	CHECK(Lzo().Decompress(compressed, kMaxDecompressedBatch, restored).IsOk());
	CHECK(restored == original);
}

MODERN_TEST(Compression_IncompressiblePayloadReportsNotCompressible)
{
	// Legacy's MINLZO_CAN_NOT_COMPRESS. This is NOT an error: the caller ships the
	// batch raw inside an envelope with the flag clear.
	const std::vector<WireU8> original = Incompressible(512);

	std::vector<WireU8> compressed;
	const CompressionOutcome outcome = Lzo().Compress(original, compressed);

	CHECK(outcome == CompressionOutcome::NotCompressible);
	// Nothing is invented when compression declines.
	CHECK(compressed.empty());
}

MODERN_TEST(Compression_RejectsEmptyInput)
{
	std::vector<WireU8> out;
	CHECK(Lzo().Compress({}, out) == CompressionOutcome::Failed);
}

MODERN_TEST(Decompression_RejectsMalformedLzoStream)
{
	// Structurally valid envelope, garbage LZO payload. Must fail cleanly.
	std::vector<WireU8> garbage(64, 0xAB);
	std::vector<WireU8> restored;

	CHECK(Lzo().Decompress(garbage, kMaxDecompressedBatch, restored).IsError());
	CHECK(restored.empty());
}

MODERN_TEST(Decompression_RejectsTruncatedCompressedStream)
{
	// A real LZO stream cut short must not decode to a partial success.
	const std::vector<WireU8> original = Compressible(4096);
	std::vector<WireU8> compressed;
	CHECK(Lzo().Compress(original, compressed) == CompressionOutcome::Compressed);

	std::vector<WireU8> truncated(compressed.begin(), compressed.end() - 8);
	std::vector<WireU8> restored;
	CHECK(Lzo().Decompress(truncated, kMaxDecompressedBatch, restored).IsError());
}

MODERN_TEST(Decompression_RejectsZeroOutputCeiling)
{
	const std::vector<WireU8> original = Compressible(512);
	std::vector<WireU8> compressed;
	CHECK(Lzo().Compress(original, compressed) == CompressionOutcome::Compressed);

	std::vector<WireU8> restored;
	CHECK(Lzo().Decompress(compressed, 0, restored).IsError());
}

MODERN_TEST(Decompression_RejectsOutputCeilingAboveHardLimit)
{
	// The ceiling is not advisory: a caller cannot ask for an unbounded buffer.
	const std::vector<WireU8> original = Compressible(512);
	std::vector<WireU8> compressed;
	CHECK(Lzo().Compress(original, compressed) == CompressionOutcome::Compressed);

	std::vector<WireU8> restored;
	CHECK(Lzo().Decompress(compressed, kMaxDecompressedBatch + 1, restored).IsError());
}

MODERN_TEST(Decompression_DeclaredSizeMismatchIsCaughtByCeiling)
{
	// A small ceiling against a larger real payload: the safe decoder must refuse
	// rather than overrun the caller's buffer.
	const std::vector<WireU8> original = Compressible(4096);
	std::vector<WireU8> compressed;
	CHECK(Lzo().Compress(original, compressed) == CompressionOutcome::Compressed);

	std::vector<WireU8> restored;
	CHECK(Lzo().Decompress(compressed, 64, restored).IsError());
	CHECK(restored.empty());
}

// ---------------------------------------------------------------------------
// Envelope layout - byte level
// ---------------------------------------------------------------------------

MODERN_TEST(Envelope_SizeIsTwelveBytes)
{
	// NET_COMPRESS is NET_MSG_GENERIC (8) + bool (1) padded to 4-byte alignment.
	// The padding is real wire data because legacy memcpys the struct and dwSize
	// counts it. A 9-byte envelope would be rejected by the shipped client.
	CHECK_EQ(kCompressEnvelopeSize, std::size_t{ 12 });
}

MODERN_TEST(Envelope_HeaderBytesAreExact)
{
	const std::vector<WireU8> inner = Compressible(256);
	std::vector<WireU8> frame;
	CHECK(NetCompress::EncodeServerToClientBatch(Lzo(), inner, frame).IsOk());

	// dwSize: little-endian, total frame bytes INCLUDING the envelope.
	CHECK_EQ(static_cast<std::size_t>(Codec::ReadU32(frame.data())), frame.size());

	// nType: NET_MSG_COMPRESS, preserved exactly. No new modern id was invented.
	CHECK_EQ(Codec::ReadU32(frame.data() + 4),
	         static_cast<WireU32>(WellKnownMessage::kCompress));
	CHECK_EQ(static_cast<WireU32>(WellKnownMessage::kCompress), WireU32{ 170 });

	// bCompress, then the 3 alignment bytes.
	CHECK_EQ(static_cast<int>(frame[8]), 1);

	CompressEnvelope envelope;
	CHECK(NetCompress::DecodeEnvelope(frame.data(), frame.size(), envelope).IsOk());

	// No phantom Count(2) field: the payload starts at offset 12, immediately after
	// the padding. V028 found the Count(2) in SendMsgBuffer.h's stale Doxygen
	// diagram; it does not exist in the struct, and a reader that expected it would
	// start its payload 2 bytes late.
	CHECK_EQ(envelope.payloadOffset, kCompressEnvelopeSize);

	// The three padding bytes are part of the frame and counted by dwSize. If they
	// were omitted the frame would be 9 bytes and dwSize would disagree with the
	// envelope the shipped client computes.
	CHECK_EQ(frame.size() - envelope.payloadSize, kCompressEnvelopeSize);
}

MODERN_TEST(Envelope_IncompressibleBatchIsStillWrappedWithFlagClear)
{
	// The conditional behaviour, pinned. Legacy SendMsgBuffer.cpp:151-166 wraps the
	// raw batch and clears bCompress. An implementation that dropped the envelope
	// and sent the batch bare would be caught here.
	const std::vector<WireU8> inner = Incompressible(512);

	std::vector<WireU8> frame;
	CHECK(NetCompress::EncodeServerToClientBatch(Lzo(), inner, frame).IsOk());

	CompressEnvelope envelope;
	CHECK(NetCompress::DecodeEnvelope(frame.data(), frame.size(), envelope).IsOk());
	CHECK(!envelope.compressed);

	// And the payload is the batch, byte for byte.
	CHECK_EQ(envelope.payloadSize, inner.size());
	CHECK(frame.size() == kCompressEnvelopeSize + inner.size());

	MinLzo1xCodec codec;
	std::vector<WireU8> restored;
	CHECK(NetCompress::DecodeServerToClientEnvelope(
	          codec, frame.data(), frame.size(), kMaxDecompressedBatch, restored).IsOk());
	CHECK(restored == inner);
}

MODERN_TEST(Envelope_RoundTripsCompressedBatch)
{
	const std::vector<WireU8> inner = Compressible(1024);

	std::vector<WireU8> frame;
	CHECK(NetCompress::EncodeServerToClientBatch(Lzo(), inner, frame).IsOk());

	CompressEnvelope envelope;
	CHECK(NetCompress::DecodeEnvelope(frame.data(), frame.size(), envelope).IsOk());
	CHECK(envelope.compressed);
	CHECK_LT(frame.size(), inner.size()); // genuinely compressed

	MinLzo1xCodec codec;
	std::vector<WireU8> restored;
	CHECK(NetCompress::DecodeServerToClientEnvelope(
	          codec, frame.data(), frame.size(), kMaxDecompressedBatch, restored).IsOk());
	CHECK(restored == inner);
}

MODERN_TEST(Envelope_RejectsEmptyBatch)
{
	std::vector<WireU8> frame;
	CHECK(NetCompress::EncodeServerToClientBatch(Lzo(), {}, frame).IsError());
	CHECK(frame.empty());
}

// ---------------------------------------------------------------------------
// Envelope validation - untrusted input
// ---------------------------------------------------------------------------

MODERN_TEST(Envelope_RejectsTruncatedEnvelope)
{
	std::vector<WireU8> frame;
	CHECK(NetCompress::EncodeServerToClientBatch(Lzo(), Compressible(128), frame).IsOk());

	MinLzo1xCodec codec;
	std::vector<WireU8> out;
	// Everything short of the full 12-byte envelope.
	for (std::size_t n = 0; n < kCompressEnvelopeSize; ++n)
	{
		CHECK(NetCompress::DecodeServerToClientEnvelope(
		          codec, frame.data(), n, kMaxDecompressedBatch, out).IsError());
	}
}

MODERN_TEST(Envelope_RejectsInvalidMessageType)
{
	std::vector<WireU8> frame;
	CHECK(NetCompress::EncodeServerToClientBatch(Lzo(), Compressible(128), frame).IsOk());

	// Rewrite nType to 256. An unwrapped message arriving where a batch is expected
	// - or a login reply misfiled - must not be treated as an envelope.
	std::vector<WireU8> wrongType = frame;
	const WireU32 kWrong = 256;
	wrongType[4] = static_cast<WireU8>(kWrong & 0xFF);
	wrongType[5] = static_cast<WireU8>((kWrong >> 8) & 0xFF);
	wrongType[6] = static_cast<WireU8>((kWrong >> 16) & 0xFF);
	wrongType[7] = static_cast<WireU8>((kWrong >> 24) & 0xFF);

	CompressEnvelope envelope;
	CHECK(NetCompress::DecodeEnvelope(wrongType.data(), wrongType.size(), envelope).IsError());

	MinLzo1xCodec codec;
	std::vector<WireU8> out;
	CHECK(NetCompress::DecodeServerToClientEnvelope(
	          codec, wrongType.data(), wrongType.size(), kMaxDecompressedBatch, out).IsError());
}

MODERN_TEST(Envelope_RejectsZeroDeclaredSize)
{
	std::vector<WireU8> frame(64, 0);
	frame[0] = 0; frame[1] = 0; frame[2] = 0; frame[3] = 0;  // dwSize = 0
	frame[4] = static_cast<WireU8>(WellKnownMessage::kCompress & 0xFF);
	frame[5] = static_cast<WireU8>((WellKnownMessage::kCompress >> 8) & 0xFF);
	frame[6] = static_cast<WireU8>((WellKnownMessage::kCompress >> 16) & 0xFF);
	frame[7] = static_cast<WireU8>((WellKnownMessage::kCompress >> 24) & 0xFF);

	CompressEnvelope envelope;
	CHECK(NetCompress::DecodeEnvelope(frame.data(), frame.size(), envelope).IsError());
}

MODERN_TEST(Envelope_RejectsSizeBelowEnvelopeLength)
{
	// dwSize = 8, i.e. the header alone: a NET_COMPRESS must be at least 12.
	std::vector<WireU8> frame(32, 0);
	frame[0] = 8;
	frame[4] = static_cast<WireU8>(WellKnownMessage::kCompress & 0xFF);
	frame[5] = static_cast<WireU8>((WellKnownMessage::kCompress >> 8) & 0xFF);
	frame[6] = static_cast<WireU8>((WellKnownMessage::kCompress >> 16) & 0xFF);
	frame[7] = static_cast<WireU8>((WellKnownMessage::kCompress >> 24) & 0xFF);

	CompressEnvelope envelope;
	CHECK(NetCompress::DecodeEnvelope(frame.data(), frame.size(), envelope).IsError());
}

MODERN_TEST(Envelope_RejectsDeclaredSizeBeyondReceivedBytes)
{
	// A hostile header claiming far more than arrived - the classic truncation
	// shape. Must be refused rather than read past the end.
	std::vector<WireU8> frame(16, 0);
	frame[0] = 0xFF; frame[1] = 0xFF; frame[2] = 0x00; frame[3] = 0x00; // dwSize = 65535
	frame[4] = static_cast<WireU8>(WellKnownMessage::kCompress & 0xFF);
	frame[5] = static_cast<WireU8>((WellKnownMessage::kCompress >> 8) & 0xFF);
	frame[6] = static_cast<WireU8>((WellKnownMessage::kCompress >> 16) & 0xFF);
	frame[7] = static_cast<WireU8>((WellKnownMessage::kCompress >> 24) & 0xFF);

	CompressEnvelope envelope;
	CHECK(NetCompress::DecodeEnvelope(frame.data(), frame.size(), envelope).IsError());

	MinLzo1xCodec codec;
	std::vector<WireU8> out;
	CHECK(NetCompress::DecodeServerToClientEnvelope(
	          codec, frame.data(), frame.size(), kMaxDecompressedBatch, out).IsError());
	CHECK(out.empty());
}

MODERN_TEST(Envelope_RejectsZeroLengthPayload)
{
	// Exactly the envelope and nothing inside: decodes to no messages at all.
	std::vector<WireU8> frame(kCompressEnvelopeSize, 0);
	const WireU32 size = static_cast<WireU32>(kCompressEnvelopeSize);
	frame[0] = static_cast<WireU8>(size & 0xFF);
	frame[1] = static_cast<WireU8>((size >> 8) & 0xFF);
	frame[2] = static_cast<WireU8>((size >> 16) & 0xFF);
	frame[3] = static_cast<WireU8>((size >> 24) & 0xFF);
	frame[4] = static_cast<WireU8>(WellKnownMessage::kCompress & 0xFF);
	frame[5] = static_cast<WireU8>((WellKnownMessage::kCompress >> 8) & 0xFF);
	frame[6] = static_cast<WireU8>((WellKnownMessage::kCompress >> 16) & 0xFF);
	frame[7] = static_cast<WireU8>((WellKnownMessage::kCompress >> 24) & 0xFF);

	CompressEnvelope envelope;
	CHECK(NetCompress::DecodeEnvelope(frame.data(), frame.size(), envelope).IsError());
}

MODERN_TEST(Envelope_RawPayloadLargerThanOutputCeilingIsRejected)
{
	// A frame that declares a big raw payload but is given a small ceiling.
	const std::vector<WireU8> inner = Incompressible(1024);
	std::vector<WireU8> frame;
	CHECK(NetCompress::EncodeServerToClientBatch(Lzo(), inner, frame).IsOk());

	MinLzo1xCodec codec;
	std::vector<WireU8> out;
	CHECK(NetCompress::DecodeServerToClientEnvelope(
	          codec, frame.data(), frame.size(), 256, out).IsError());
	CHECK(out.empty());
}

MODERN_TEST(Envelope_MaxCompressedSizeIsNeverSmallerThanInput)
{
	// The sizing helper must be usable as a buffer bound.
	for (std::size_t n : { std::size_t{ 0 }, std::size_t{ 1 }, std::size_t{ 100 },
	                       std::size_t{ 2048 }, std::size_t{ 16384 } })
	{
		CHECK_GE(NetCompress::MaxCompressedSize(n), n);
	}
}

MODERN_TEST(Envelope_IsEnvelopeIdentifiesOnlyType170)
{
	CHECK(NetCompress::IsEnvelope(170));
	CHECK(!NetCompress::IsEnvelope(0));
	// 2049 is the V029 login id - it must NOT be treated as an envelope.
	CHECK(!NetCompress::IsEnvelope(2049));
}

// ---------------------------------------------------------------------------
// Multiple inner messages - the reason the batch exists
// ---------------------------------------------------------------------------

MODERN_TEST(Batch_ThreeMessagesSurviveEnvelopeInOrder)
{
	// The central test. Three messages become ONE inner stream, ONE envelope, and
	// come back out of ConnectionFramer in the original order.
	std::vector<WireU8> a = MakeMessage(WellKnownMessage::kVersionOk, { 0x11 });
	std::vector<WireU8> b = MakeMessage(WellKnownMessage::kVersionInfo, { 0x22, 0x22, 0x22 });
	std::vector<WireU8> c = MakeMessage(WellKnownMessage::kHeartbeatClientReq, { 0x33 });

	// Pad so the batch is worth compressing and exercises the LZO path.
	std::vector<WireU8> payloadA(200, 0x41);
	std::vector<WireU8> payloadB(200, 0x42);
	std::vector<WireU8> payloadC(200, 0x43);
	a = MakeMessage(WellKnownMessage::kVersionOk, payloadA);
	b = MakeMessage(WellKnownMessage::kVersionInfo, payloadB);
	c = MakeMessage(WellKnownMessage::kHeartbeatClientReq, payloadC);

	ServerBatchEncoder encoder(Lzo());

	std::vector<WireU8> pending;
	CHECK(encoder.Add(a, pending) == BatchAction::Buffered);
	CHECK(encoder.Add(b, pending) == BatchAction::Buffered);
	CHECK(encoder.Add(c, pending) == BatchAction::Buffered);
	CHECK_EQ(encoder.Count(), std::size_t{ 3 });

	CHECK(encoder.Flush(pending));
	CHECK(!pending.empty());

	// Exactly one envelope for three messages.
	CompressEnvelope envelope;
	CHECK(NetCompress::DecodeEnvelope(pending.data(), pending.size(), envelope).IsOk());

	MinLzo1xCodec codec;
	std::vector<WireU8> inner;
	CHECK(NetCompress::DecodeServerToClientEnvelope(
	          codec, pending.data(), pending.size(), kMaxDecompressedBatch, inner).IsOk());

	// The inner stream is the three messages back to back.
	CHECK_EQ(inner.size(), a.size() + b.size() + c.size());

	// And the V027 framer recovers them individually, in order.
	ConnectionFramer framer;
	CHECK(framer.Feed(inner.data(), inner.size()) == FrameStatus::Ok);

	Message out;
	CHECK(framer.Next(out) == FrameStatus::Ok);
	CHECK_EQ(static_cast<WireU32>(out.header.type),
	         static_cast<WireU32>(WellKnownMessage::kVersionOk));
	CHECK_EQ(out.payload.size(), std::size_t{ 200 });

	CHECK(framer.Next(out) == FrameStatus::Ok);
	CHECK_EQ(static_cast<WireU32>(out.header.type),
	         static_cast<WireU32>(WellKnownMessage::kVersionInfo));
	CHECK_EQ(out.payload.size(), std::size_t{ 200 });

	CHECK(framer.Next(out) == FrameStatus::Ok);
	CHECK_EQ(static_cast<WireU32>(out.header.type),
	         static_cast<WireU32>(WellKnownMessage::kHeartbeatClientReq));
	CHECK_EQ(out.payload.size(), std::size_t{ 200 });

	CHECK(framer.Next(out) == FrameStatus::NeedMoreData);
}

MODERN_TEST(Batch_InnerFramerRejectsTruncatedInnerMessage)
{
	// A batch whose payload is cut mid-message must surface as a framer refusal,
	// not as a partial message. The envelope can be valid and the stream still bad.
	std::vector<WireU8> inner = MakeMessage(WellKnownMessage::kVersionInfo,
	                                        std::vector<WireU8>(64, 0x55));
	inner.resize(inner.size() - 8); // chop the declared-size tail

	std::vector<WireU8> frame;
	CHECK(NetCompress::EncodeServerToClientBatch(Lzo(), inner, frame).IsOk());

	MinLzo1xCodec codec;
	std::vector<WireU8> restored;
	CHECK(NetCompress::DecodeServerToClientEnvelope(
	          codec, frame.data(), frame.size(), kMaxDecompressedBatch, restored).IsOk());

	ConnectionFramer framer;
	CHECK(framer.Feed(restored.data(), restored.size()) == FrameStatus::Ok);

	Message out;
	CHECK(framer.Next(out) == FrameStatus::NeedMoreData);
	CHECK(!framer.IsFailed()); // incomplete, not malformed
}

MODERN_TEST(Batch_InnerFramerRejectsInvalidInnerMessageSize)
{
	// A syntactically valid envelope wrapping a message with an impossible size.
	std::vector<WireU8> inner = MakeMessage(WellKnownMessage::kVersionInfo,
	                                        std::vector<WireU8>(32, 0x66));
	inner[0] = 0xFF; inner[1] = 0xFF; inner[2] = 0xFF; inner[3] = 0x7F; // absurd dwSize

	std::vector<WireU8> frame;
	CHECK(NetCompress::EncodeServerToClientBatch(Lzo(), inner, frame).IsOk());

	MinLzo1xCodec codec;
	std::vector<WireU8> restored;
	CHECK(NetCompress::DecodeServerToClientEnvelope(
	          codec, frame.data(), frame.size(), kMaxDecompressedBatch, restored).IsOk());

	ConnectionFramer framer;
	CHECK(framer.Feed(restored.data(), restored.size()) == FrameStatus::Ok);

	Message out;
	CHECK(framer.Next(out) == FrameStatus::InvalidLength);
	CHECK(framer.IsFailed());
}

// ---------------------------------------------------------------------------
// Batching - the 1000-byte trigger
// ---------------------------------------------------------------------------

MODERN_TEST(Batching_ThresholdIsOneThousand)
{
	CHECK_EQ(Protocol::kCompressThreshold, std::size_t{ 1000 });
}

MODERN_TEST(Batching_BelowThresholdDoesNotFlush)
{
	// 999 bytes buffered: legacy compares `dwTotal < COMPRESS_PACKET_SIZE` strictly,
	// so 999 stays put. The batch is still real, just not forced out yet.
	ServerBatchEncoder encoder(Lzo());

	// 992-byte message then a 7-byte message = 999 total.
	std::vector<WireU8> first = MakeMessage(WellKnownMessage::kVersionInfo,
	                                        std::vector<WireU8>(992 - kMessageHeaderSize, 0x41));
	std::vector<WireU8> second = MakeMessage(WellKnownMessage::kVersionOk, {});

	// The second message is 8 bytes; pad the first so the running total is 999.
	first = MakeMessage(WellKnownMessage::kVersionInfo,
	                     std::vector<WireU8>(999 - kMessageHeaderSize - kMessageHeaderSize, 0x41));
	CHECK_EQ(first.size(), std::size_t{ 991 });

	std::vector<WireU8> pending;
	CHECK(encoder.Add(first, pending) == BatchAction::Buffered);
	CHECK(encoder.Add(second, pending) == BatchAction::Buffered);
	CHECK_EQ(encoder.Buffered(), std::size_t{ 999 });
	CHECK(pending.empty());
}

MODERN_TEST(Batching_ExactlyOneThousandFlushes)
{
	// The threshold is `dwTotal < 1000` for BUFFERING, so 1000 is already on the
	// flush side. Legacy's else-branch runs, the batch is empty, and the message is
	// sent ALONE. Getting this wrong by one byte is the difference between a batch
	// that flushes now and one that waits - and it only shows up as a frame-count
	// mismatch against a real client, not as a crash.
	ServerBatchEncoder encoder(Lzo());

	std::vector<WireU8> message = MakeMessage(WellKnownMessage::kVersionInfo,
	                                          std::vector<WireU8>(1000 - kMessageHeaderSize, 0x41));
	CHECK_EQ(message.size(), std::size_t{ 1000 });

	std::vector<WireU8> pending;
	CHECK(encoder.Add(message, pending) == BatchAction::FlushOnly);
	CHECK(!pending.empty());
	CHECK_EQ(encoder.Buffered(), std::size_t{ 0 });
}

MODERN_TEST(Batching_NineHundredNinetyNineBuffers)
{
	// One byte below the threshold: buffered, no flush. The boundary pair with the
	// test above.
	ServerBatchEncoder encoder(Lzo());

	std::vector<WireU8> message = MakeMessage(WellKnownMessage::kVersionInfo,
	                                          std::vector<WireU8>(999 - kMessageHeaderSize, 0x41));
	CHECK_EQ(message.size(), std::size_t{ 999 });

	std::vector<WireU8> pending;
	CHECK(encoder.Add(message, pending) == BatchAction::Buffered);
	CHECK_EQ(encoder.Buffered(), std::size_t{ 999 });
	CHECK(pending.empty());
}

MODERN_TEST(Batching_OneThousandAndOneFlushes)
{
	// 1001 crosses the trigger. With an empty batch, legacy sends this message
	// ALONE and reports FlushOnly.
	ServerBatchEncoder encoder(Lzo());

	std::vector<WireU8> message = MakeMessage(WellKnownMessage::kVersionInfo,
	                                          std::vector<WireU8>(1001 - kMessageHeaderSize, 0x41));
	CHECK_EQ(message.size(), std::size_t{ 1001 });

	std::vector<WireU8> pending;
	CHECK(encoder.Add(message, pending) == BatchAction::FlushOnly);
	CHECK(!pending.empty());
	CHECK_EQ(encoder.Buffered(), std::size_t{ 0 }); // flushed and cleared
}

MODERN_TEST(Batching_CrossingThresholdFlushesWithoutTheNewMessage)
{
	// The BUFFER_SEND_ADD shape: the accumulated batch goes out WITHOUT the message
	// that crossed the threshold, which the caller must then re-offer.
	ServerBatchEncoder encoder(Lzo());

	std::vector<WireU8> small = MakeMessage(WellKnownMessage::kVersionOk, {}); // 8 bytes
	std::vector<WireU8> big = MakeMessage(WellKnownMessage::kVersionInfo,
	                                      std::vector<WireU8>(992, 0x42));     // 1000 bytes

	std::vector<WireU8> pending;
	CHECK(encoder.Add(small, pending) == BatchAction::Buffered);
	CHECK_EQ(encoder.Buffered(), std::size_t{ 8 });

	CHECK(encoder.Add(big, pending) == BatchAction::FlushPending);
	CHECK(!pending.empty()); // the 8-byte message was flushed

	// The batch now holds only the big message.
	CHECK_EQ(encoder.Buffered(), std::size_t{ 1000 });
	CHECK_EQ(encoder.Count(), std::size_t{ 1 });

	// And the flushed frame contains the small message, not the big one.
	MinLzo1xCodec codec;
	std::vector<WireU8> inner;
	CHECK(NetCompress::DecodeServerToClientEnvelope(
	          codec, pending.data(), pending.size(), kMaxDecompressedBatch, inner).IsOk());
	CHECK_EQ(inner.size(), small.size());

	ConnectionFramer framer;
	CHECK(framer.Feed(inner.data(), inner.size()) == FrameStatus::Ok);
	Message out;
	CHECK(framer.Next(out) == FrameStatus::Ok);
	CHECK_EQ(static_cast<WireU32>(out.header.type),
	         static_cast<WireU32>(WellKnownMessage::kVersionOk));
}

MODERN_TEST(Batching_UndersizedTailStillFlushesAtEndOfTick)
{
	// 1000 is a flush TRIGGER, not a cap. A 999-byte tail is still transmitted when
	// SendClientFinal-equivalent Flush runs. An implementation that treated 1000 as
	// a maximum would deadlock this batch forever.
	ServerBatchEncoder encoder(Lzo());

	std::vector<WireU8> message = MakeMessage(WellKnownMessage::kVersionInfo,
	                                          std::vector<WireU8>(999 - kMessageHeaderSize, 0x41));
	CHECK_EQ(message.size(), std::size_t{ 999 });

	std::vector<WireU8> pending;
	CHECK(encoder.Add(message, pending) == BatchAction::Buffered);
	CHECK(pending.empty());

	CHECK(encoder.Flush(pending));
	CHECK(!pending.empty());
	CHECK_EQ(encoder.Buffered(), std::size_t{ 0 });
}

MODERN_TEST(Batching_FlushOnEmptyBatchIsNotAnError)
{
	ServerBatchEncoder encoder(Lzo());
	std::vector<WireU8> pending;
	CHECK(!encoder.Flush(pending));
	CHECK(pending.empty());
}

MODERN_TEST(Batching_ResetDiscardsPendingBatch)
{
	ServerBatchEncoder encoder(Lzo());
	std::vector<WireU8> pending;
	CHECK(encoder.Add(MakeMessage(WellKnownMessage::kVersionOk, {}), pending) ==
	      BatchAction::Buffered);

	encoder.Reset();
	CHECK_EQ(encoder.Buffered(), std::size_t{ 0 });
	CHECK_EQ(encoder.Count(), std::size_t{ 0 });
	CHECK(!encoder.Flush(pending));
}

MODERN_TEST(Batching_FragmentIsIgnoredRatherThanCorrupted)
{
	// Less than a header: nothing to batch, and certainly not something to flush.
	ServerBatchEncoder encoder(Lzo());
	std::vector<WireU8> fragment(4, 0x99);
	std::vector<WireU8> pending;

	CHECK(encoder.Add(fragment, pending) == BatchAction::Buffered);
	CHECK_EQ(encoder.Buffered(), std::size_t{ 0 });
	CHECK(pending.empty());
}

// ---------------------------------------------------------------------------
// Direction
// ---------------------------------------------------------------------------

MODERN_TEST(Direction_ClientToServerStaysUnwrapped)
{
	// The asymmetry, pinned as a test so it cannot be "helpfully" fixed later.
	//
	// A client->server message is a bare NET_MSG_GENERIC. It goes to the framer
	// directly. No envelope, no LZO, no batcher - the shipped client does
	// CNetClient::SendBuffer2 -> ::send(dwSize), wrapping nothing.
	std::vector<WireU8> login = MakeMessage(2049, std::vector<WireU8>(68, 0x00));

	// 2049 is NET_MSG_LOGIN_2 per V029.
	CHECK_EQ(static_cast<WireU32>(Codec::ReadU32(login.data() + 4)), WireU32{ 2049 });

	// Not an envelope.
	CHECK(!NetCompress::IsEnvelope(Codec::ReadU32(login.data() + 4)));

	// The framer takes it as-is.
	ConnectionFramer framer;
	CHECK(framer.Feed(login.data(), login.size()) == FrameStatus::Ok);
	Message out;
	CHECK(framer.Next(out) == FrameStatus::Ok);
	CHECK_EQ(static_cast<WireU32>(out.header.type), WireU32{ 2049 });
	CHECK_EQ(out.PayloadSize(), std::size_t{ 68 });
}

MODERN_TEST(Direction_ServerToClientAlwaysEmitsAnEnvelope)
{
	// Whether the batch compressed or not, exactly one envelope goes out. Legacy
	// has no raw server->client path: every send reaches WSASend through
	// getSendSize(), which always wraps.
	ServerBatchEncoder encoder(Lzo());
	std::vector<WireU8> pending;

	// Compressible.
	CHECK(encoder.Add(MakeMessage(WellKnownMessage::kVersionOk,
	                              std::vector<WireU8>(256, 0x41)), pending) ==
	      BatchAction::Buffered);
	CHECK(encoder.Flush(pending));
	CompressEnvelope envelope;
	CHECK(NetCompress::DecodeEnvelope(pending.data(), pending.size(), envelope).IsOk());
	CHECK(envelope.compressed);

	// Incompressible: still an envelope, flag clear.
	encoder.Reset();
	std::vector<WireU8> noise = MakeMessage(WellKnownMessage::kVersionInfo, Incompressible(256));
	CHECK(encoder.Add(noise, pending) == BatchAction::Buffered);
	CHECK(encoder.Flush(pending));
	CHECK(NetCompress::DecodeEnvelope(pending.data(), pending.size(), envelope).IsOk());
	CHECK(!envelope.compressed);
}