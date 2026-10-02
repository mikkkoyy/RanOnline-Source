#include "MinTeaCodec.h"

#include <cstring>

namespace Modern::Network
{
	const char* const kLegacyTeaKey = "Steven Seagal Neck Break";

	namespace
	{
		// TEA delta, Wheeler & Needham Oct 1998. minTea.cpp, both directions.
		constexpr WireU32 kTeaDelta = 0x9E3779B9u;

		// Legacy computes `floor(6 + 52.0f/n)` in floating point. Because 6 is an
		// integer, that equals `6 + floor(52/n)` exactly, so integer division
		// reproduces it without the float.
		constexpr std::size_t RoundsFor(std::size_t blocks) noexcept
		{
			return blocks == 0 ? 0 : 6u + (52u / blocks);
		}

		// Scans backwards while the byte is NUL - legacy's "recover the string
		// length from a NUL-padded fixed field" step. Returns 0 for an all-NUL
		// buffer, which legacy also degenerates to.
		std::size_t EffectiveLength(const std::vector<WireU8>& buffer, std::size_t length) noexcept
		{
			std::size_t n = length;
			while (n > 0 && buffer[n - 1] == 0)
			{
				--n;
			}
			return n;
		}
	}

	MinTea::MinTea() noexcept
		: MinTea(kLegacyTeaKey)
	{
	}

	MinTea::MinTea(const std::string& key)
	{
		// Legacy setKey (minTea.cpp:29-44) zeroes the buffer then fills it by
		// repeating the key. It asserts strlen(key) > TEA_KEY_LENGTH, which is a
		// release-mode no-op, so a short key here wraps rather than aborts - the
		// same outcome legacy would produce if asserts were compiled out.
		WireU8 repeated[kTeaKeyLength] = {};
		std::memset(repeated, 0, sizeof(repeated));

		const std::size_t keyLength = key.size();
		if (keyLength > 0)
		{
			for (std::size_t n = 0; n < kTeaKeyLength; ++n)
			{
				repeated[n] = static_cast<WireU8>(key[n % keyLength]);
			}
		}

		// Legacy casts a char[16] to UINT*, so each 32-bit word is four key bytes in
		// host order. The wire is little-endian, and the modern layer is
		// little-endian throughout, so reading them as four little-endian bytes
		// reproduces x86 exactly.
		for (std::size_t w = 0; w < kTeaKeyLength / 4; ++w)
		{
			m_key[w] = static_cast<WireU32>(repeated[w * 4]) |
			           (static_cast<WireU32>(repeated[w * 4 + 1]) << 8) |
			           (static_cast<WireU32>(repeated[w * 4 + 2]) << 16) |
			           (static_cast<WireU32>(repeated[w * 4 + 3]) << 24);
		}
	}

	std::size_t MinTea::CipherLengthFor(std::size_t bufferLength) noexcept
	{
		// The caller-supplied length stands in for nMaxLength. Legacy then strips
		// trailing NULs, which this pure function cannot know, so it models the
		// "no trailing NUL" case: pad to the floor and to a multiple of 4.
		std::size_t n = bufferLength;
		while (n <= kTeaMinimumPlaintextLength)
		{
			++n;
		}
		while ((n & 3u) != 0u)
		{
			++n;
		}
		return n;
	}

	void MinTea::EncryptBlocks(WireU32* data, std::size_t blocks, const WireU32* key) noexcept
	{
		if (blocks < 2)
		{
			return; // legacy returns -1 for n < 2 and the caller ignores it
		}

		WireU32 z = data[blocks - 1];
		WireU32 y = data[0];
		WireU32 sum = 0;

		std::size_t rounds = RoundsFor(blocks);
		std::size_t p = 0;

		while (rounds-- > 0)
		{
			sum += kTeaDelta;
			const WireU32 e = (sum >> 2) & 3u;
			for (p = 0; p < blocks - 1; ++p)
			{
				y = data[p + 1];
				const WireU32 mx = ((z >> 5) ^ (y << 2)) + ((y >> 3) ^ (z << 4)) ^ (sum ^ y) +
				                   (key[(p & 3u) ^ e] ^ z);
				z = (data[p] += mx);
			}
			y = data[0];
			const WireU32 mx = ((z >> 5) ^ (y << 2)) + ((y >> 3) ^ (z << 4)) ^ (sum ^ y) +
			                   (key[(p & 3u) ^ e] ^ z);
			z = (data[blocks - 1] += mx);
		}
	}

	void MinTea::DecryptBlocks(WireU32* data, std::size_t blocks, const WireU32* key) noexcept
	{
		if (blocks < 2)
		{
			return;
		}

		WireU32 z = data[blocks - 1];
		WireU32 y = data[0];

		std::size_t rounds = RoundsFor(blocks);
		WireU32 sum = static_cast<WireU32>(rounds) * kTeaDelta;
		std::size_t p = 0;

		while (sum != 0)
		{
			const WireU32 e = (sum >> 2) & 3u;
			for (p = blocks - 1; p > 0; --p)
			{
				z = data[p - 1];
				const WireU32 mx = ((z >> 5) ^ (y << 2)) + ((y >> 3) ^ (z << 4)) ^ (sum ^ y) +
				                   (key[(p & 3u) ^ e] ^ z);
				y = (data[p] -= mx);
			}
			z = data[blocks - 1];
			const WireU32 mx = ((z >> 5) ^ (y << 2)) + ((y >> 3) ^ (z << 4)) ^ (sum ^ y) +
			                   (key[(p & 3u) ^ e] ^ z);
			y = (data[0] -= mx);
			sum -= kTeaDelta;
		}
	}

	bool MinTea::CanEncryptInPlace(const std::vector<WireU8>& buffer,
	                              std::size_t length) const noexcept
	{
		if (length > buffer.size())
		{
			return false;
		}
		return CipherLengthFor(EffectiveLength(buffer, length)) <= length;
	}

	bool MinTea::CanDecryptInPlace(const std::vector<WireU8>& buffer,
	                               std::size_t length) const noexcept
	{
		if (length > buffer.size())
		{
			return false;
		}
		std::size_t cipherLength = EffectiveLength(buffer, length);
		while ((cipherLength & 3u) != 0u)
		{
			++cipherLength;
		}
		// An all-NUL buffer degenerates to 0, which legacy treats as a no-op.
		return cipherLength == 0 || cipherLength <= length;
	}

	Status MinTea::EncryptInPlace(std::vector<WireU8>& buffer, std::size_t length) const
	{
		if (length > buffer.size())
		{
			return Status(ErrorCode::InvalidArgument);
		}

		const std::size_t effective = EffectiveLength(buffer, length);
		const std::size_t cipherLength = CipherLengthFor(effective);

		if (cipherLength > length)
		{
			// Legacy's "encrypted length error" branch: the ciphertext would not fit,
			// so nothing is written. Reachable when the field is nearly full.
			return Status(ErrorCode::InvalidArgument);
		}

		// Legacy copies `effective` bytes into a scratch buffer and zero-pads up to
		// cipherLength before transforming. Anything past `effective` is NUL.
		std::vector<WireU8> scratch(cipherLength, 0);
		if (effective > 0)
		{
			std::memcpy(scratch.data(), buffer.data(), effective);
		}

		EncryptBlocks(reinterpret_cast<WireU32*>(scratch.data()),
		              cipherLength / 4,
		              m_key);

		// Only the ciphertext is written back. Bytes past cipherLength keep whatever
		// the field held - which for NET_LOGIN_DATA is NUL from the constructor.
		std::memcpy(buffer.data(), scratch.data(), cipherLength);
		return Ok();
	}

	Status MinTea::DecryptInPlace(std::vector<WireU8>& buffer, std::size_t length) const
	{
		if (length > buffer.size())
		{
			return Status(ErrorCode::InvalidArgument);
		}

		std::size_t cipherLength = EffectiveLength(buffer, length);

		// Decrypt rounds UP to a multiple of 4 (minTea.cpp, `while (keyLen & 3)`),
		// unlike Encrypt which pads up to a minimum first. An all-NUL buffer
		// degenerates to 0, which legacy leaves as a no-op.
		while ((cipherLength & 3u) != 0u)
		{
			++cipherLength;
		}

		if (cipherLength == 0)
		{
			return Ok(); // nothing encrypted here
		}
		if (cipherLength > length)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		std::vector<WireU8> scratch(cipherLength, 0);
		std::memcpy(scratch.data(), buffer.data(), cipherLength);

		DecryptBlocks(reinterpret_cast<WireU32*>(scratch.data()),
		              cipherLength / 4,
		              m_key);

		std::memcpy(buffer.data(), scratch.data(), cipherLength);
		return Ok();
	}
}