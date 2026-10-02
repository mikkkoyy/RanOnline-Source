#include "CompressionCodec.h"

// Vendored third-party C. See third_party/minilzo/VENDOR.md for provenance and the
// open licensing question. minilzo.h pulls in lzoconf.h/lzodefs.h from the same
// directory, which is why this target carries it as a private include.
#include "third_party/minilzo/minilzo.h"

#include <cstdlib>
#include <cstring>

namespace Modern::Network
{
	MinLzo1xCodec::MinLzo1xCodec()
	{
		// lzo_init() checks the CPU for the instruction patterns the library
		// depends on. RAN logs a diagnostic and gives up when it fails
		// (MinLzo.cpp:62-67), which would mean every subsequent compress call
		// operated on an uninitialised library. Here the codec simply reports
		// itself unready and refuses work.
		if (lzo_init() != LZO_E_OK)
		{
			m_ready = false;
			return;
		}

		m_workmem = std::malloc(LZO1X_1_MEM_COMPRESS);
		m_ready   = m_workmem != nullptr;
	}

	MinLzo1xCodec::~MinLzo1xCodec()
	{
		if (m_workmem != nullptr)
		{
			std::free(m_workmem);
			m_workmem = nullptr;
		}
	}

	CompressionOutcome MinLzo1xCodec::Compress(const std::vector<uint8_t>& input,
	                                           std::vector<uint8_t>& out)
	{
		out.clear();

		if (!m_ready)
		{
			return CompressionOutcome::Failed;
		}
		if (input.empty())
		{
			return CompressionOutcome::Failed;
		}

		// LZO1X-1 worst case is the input plus a small overrun. The legacy wrapper
		// never sized this itself - CSendMsgBuffer passes a 6144-byte send buffer -
		// so the bound is derived here rather than assumed.
		out.resize(input.size() + (input.size() / 16) + 64 + 3);

		lzo_uint outLen = 0;
		const int result = lzo1x_1_compress(input.data(),
		                                     static_cast<lzo_uint>(input.size()),
		                                     out.data(),
		                                     &outLen,
		                                     m_workmem);

		if (result != LZO_E_OK)
		{
			out.clear();
			return CompressionOutcome::Failed;
		}

		out.resize(outLen);

		// Legacy's MINLZO_CAN_NOT_COMPRESS rule (MinLzo.cpp:120-125): a result that
		// did not actually shrink is not a compressed payload. Reporting success here
		// would set the envelope's bCompress flag for bytes that are not compressed,
		// and the peer would hand them to the LZO decoder.
		if (outLen >= input.size())
		{
			out.clear();
			return CompressionOutcome::NotCompressible;
		}

		return CompressionOutcome::Compressed;
	}

	Status MinLzo1xCodec::Decompress(const std::vector<uint8_t>& input,
	                                 std::size_t maxOutput,
	                                 std::vector<uint8_t>& out)
	{
		out.clear();

		if (!m_ready)
		{
			return Status(ErrorCode::InvalidState);
		}
		if (input.empty() || maxOutput == 0 || maxOutput > kMaxDecompressedBatch)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		out.resize(maxOutput);

		// IN/OUT: on entry the destination capacity, on exit the bytes written.
		// See the header note - this is the whole reason "safe" is in the name.
		lzo_uint outLen = static_cast<lzo_uint>(maxOutput);

		const int result = lzo1x_decompress_safe(input.data(),
		                                         static_cast<lzo_uint>(input.size()),
		                                         out.data(),
		                                         &outLen,
		                                         nullptr);

		if (result != LZO_E_OK)
		{
			out.clear();
			return Status(ErrorCode::InvalidArgument);
		}

		out.resize(outLen);
		return Ok();
	}
}