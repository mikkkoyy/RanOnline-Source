#include "RanWirePrimitives.h"

#include <algorithm>
#include <cstring>

namespace Modern::Network::RanWire
{
	namespace
	{
		void PutU32(std::vector<WireU8>& out, std::size_t offset, WireU32 value) noexcept
		{
			for (int i = 0; i < 4; ++i)
			{
				out[offset + static_cast<std::size_t>(i)] =
				    static_cast<WireU8>((value >> (8 * i)) & 0xFFu);
			}
		}
	}

	Status PutFixedCharField(std::vector<WireU8>& out, std::size_t offset,
	                         std::size_t fieldSize, const std::string& value)
	{
		if (fieldSize == 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}
		// `>=` fieldSize, not `>`. The last byte of the field is reserved for the
		// terminator, so a value exactly fieldSize-1 characters long is the longest
		// that fits. Accepting one more would produce a field with no terminator, and
		// the decoder below rejects exactly that - so accepting it here would mean
		// encoding a packet our own decoder refuses.
		if (value.size() >= fieldSize)
		{
			return Status(ErrorCode::InvalidArgument);
		}
		if (offset + fieldSize > out.size())
		{
			return Status(ErrorCode::InvalidArgument);
		}

		std::fill(out.begin() + static_cast<std::ptrdiff_t>(offset),
		          out.begin() + static_cast<std::ptrdiff_t>(offset + fieldSize),
		          static_cast<WireU8>(0));

		if (!value.empty())
		{
			std::memcpy(out.data() + offset, value.data(), value.size());
		}
		return Ok();
	}

	Status ReadFixedCharField(const std::vector<WireU8>& frame, std::size_t offset,
	                          std::size_t fieldSize, std::string& out)
	{
		if (fieldSize == 0 || offset + fieldSize > frame.size())
		{
			return Status(ErrorCode::InvalidArgument);
		}

		const WireU8* begin  = frame.data() + offset;
		const void*    nul   = std::memchr(begin, 0, fieldSize);
		if (nul == nullptr)
		{
			// No terminator anywhere in the field. Legacy's readers would run off the
			// end of the struct here; refusing is the whole point of the modern
			// boundary.
			return Status(ErrorCode::InvalidArgument);
		}

		out.assign(reinterpret_cast<const char*>(begin),
		           static_cast<std::size_t>(static_cast<const WireU8*>(nul) - begin));
		return Ok();
	}

	bool IsRegionZeroed(const std::vector<WireU8>& frame, std::size_t offset,
	                    std::size_t length)
	{
		if (offset + length > frame.size())
		{
			return false;
		}
		for (std::size_t i = 0; i < length; ++i)
		{
			if (frame[offset + i] != 0)
			{
				return false;
			}
		}
		return true;
	}
}