#pragma once

// WORLD-001: minTea, the legacy login field cipher.
//
// This is a reimplementation, not a port. It reproduces the WIRE BEHAVIOUR of
// legacy/Lib_Network/minTea.cpp; where the legacy code cannot be compiled or read
// directly this header says exactly which observable rule each function encodes.
//
// Legacy provenance:
//
//   Key            minTea.cpp:16-24   default ctor, hardcoded, never overridden
//   Key schedule   minTea.cpp:29-44   setKey: repeat key chars to TEA_KEY_LENGTH
//   Cipher         minTea.cpp (int minTea::encrypt(UINT*, UINT, UINT*))
//                                      "TEA routine as per Wheeler & Needham, Oct 1998"
//   In-place API   minTea.h:272-273   bool encrypt(char*, int nMaxLength)
//
// ---------------------------------------------------------------------------
// THE ONE THING THAT IS NOT OBVIOUS
// ---------------------------------------------------------------------------
//
// The in-place API does NOT encrypt `nMaxLength` bytes. Reading
// `m_Tea.encrypt(nld.szPassword, USR_PASS_LENGTH+1)` as "TEA over 21 bytes" is
// wrong, and an implementation built on that reading would put different bytes
// on the wire than the shipped client.
//
// What actually happens, per field:
//
//   1. scan backwards from nMaxLength while the byte is NUL, yielding `len`
//      - i.e. recover the string length, ignoring trailing NUL padding
//   2. grow to a minimum of 5 bytes      (`while (len <= 4) ++len;`)
//   3. round up to a multiple of 4       (`while (len & 3) ++len;`)
//   4. TEA-encrypt exactly that many bytes
//   5. write the ciphertext back over the buffer; bytes past it are UNTOUCHED
//
// Consequences that the tests pin, because each one is a silent wire difference:
//
//   - A 1-character username occupies 8 ciphertext bytes, not 4 and not 21.
//   - The ciphertext length therefore varies per field and is NOT the field width.
//   - The remainder of the field keeps whatever it held. In NET_LOGIN_DATA the
//     constructor SecureZeroMemory's every field, so the tail is NUL - but a
//     decoder must not ASSUME NUL, it must re-derive the length the same way.
//
// Decryption is the mirror: scan back for the ciphertext length, round UP to a
// multiple of 4, and decrypt that many bytes.
//
// Legacy cannot actually be compiled in this repository: it includes "minlzo.h"
// (absent) and its acCArray container (also absent). The rules above are derived
// from the source that IS present, and are corroborated by tests that check
// lengths and round-trips rather than assuming them.

#include "NetworkTypes.h"
#include "types/Result.h"

#include <cstddef>
#include <string>
#include <vector>

namespace Modern::Network
{
	// Legacy TEA_KEY_LENGTH (minTea.h:257).
	constexpr std::size_t kTeaKeyLength = 16;

	// The legacy key, reproduced because it is part of the protocol. This is not a
	// secret and not a modernisation target: changing it changes the wire format and
	// makes the modern client unable to talk to any RAN server.
	extern const char* const kLegacyTeaKey;

	// The smallest plaintext block legacy will encrypt.
	//
	// `while (len <= 4) ++len` then round up to 4 means the floor is 5, and 5 rounds
	// up to 8. So in practice every field becomes a multiple of 8 bytes minimum.
	constexpr std::size_t kTeaMinimumPlaintextLength = 5;

	// The legacy cipher, applied to a byte buffer.
	//
	// Not thread-safe and not intended to be: instances are cheap, and one per
	// thread is cheaper than locking. Stateless in practice (the key is const), but
	// the type is non-copyable to make accidental sharing visible.
	class MinTea
	{
	public:
		// Uses the legacy key. This is the only constructor WORLD-001 needs; a
		// custom key exists because minTea.h exposes setKey, but no RAN path uses it.
		MinTea() noexcept;

		explicit MinTea(const std::string& key);

		// In-place encrypt of buffer[0, length). Mirrors legacy exactly, including
		// the NUL-strip and the padding rule.
		Status EncryptInPlace(std::vector<WireU8>& buffer, std::size_t length) const;

		// In-place decrypt of buffer[0, length). Mirrors legacy exactly.
		Status DecryptInPlace(std::vector<WireU8>& buffer, std::size_t length) const;

		// The number of bytes legacy would encrypt for this input, without doing it.
		// Exposed so tests and callers can reason about the variable ciphertext
		// length rather than assuming it equals the field width.
		static std::size_t CipherLengthFor(std::size_t bufferLength) noexcept;

		// Whether the legacy cipher would transform this buffer at all.
		//
		// ------------------------------------------------------------------
		// THIS IS WHY THE RANDOM PASSWORD IS NOT ENCRYPTED. Read it before
		// assuming a bug.
		// ------------------------------------------------------------------
		//
		// `minTea::encrypt` computes the ciphertext first and then checks
		//
		//     if (nEncryptedLength > nMaxLength) return false;   // minTea.cpp
		//
		// For szRandomPassword the buffer is USR_RAND_PASS_LENGTH+1 = 7 bytes, and
		// a 6-digit random number yields an effective length of 6, which pads to an
		// 8-byte ciphertext. 8 > 7, so legacy returns FALSE AND COPIES NOTHING.
		// CNetClient::SndLogin ignores that return value, so the field goes out as
		// the plaintext the constructor left there.
		//
		// The server mirrors it exactly: `m_Tea.decrypt(szRandomPassword, 7)` hits
		// the same guard, returns false, and leaves the field alone - so the
		// plaintext random password is what the Agent reads and checks. The failure
		// is symmetric, which is why this has never broken.
		//
		// So this is not a defect to fix; it is wire behaviour to reproduce. A
		// "correct" implementation that encrypted all 7 bytes would put different
		// bytes on the wire than the shipped client and would be rejected by every
		// RAN server.
		//
		// Encrypt and decrypt are asked separately because their padding rules
		// DIFFER: encrypt pads up to a 5-byte floor, decrypt just rounds up to a
		// multiple of 4. A buffer can therefore fit in one direction and not the
		// other - a 1-character random password, for instance - and legacy declines
		// in each direction on its own terms.
		bool CanEncryptInPlace(const std::vector<WireU8>& buffer,
		                       std::size_t length) const noexcept;

		bool CanDecryptInPlace(const std::vector<WireU8>& buffer,
		                       std::size_t length) const noexcept;

		// The raw TEA block transform, exposed for the byte-level tests. `blocks` is
		// the number of 32-bit words (so bytes/4).
		static void EncryptBlocks(WireU32* data, std::size_t blocks, const WireU32* key) noexcept;
		static void DecryptBlocks(WireU32* data, std::size_t blocks, const WireU32* key) noexcept;

	private:
		WireU32 m_key[kTeaKeyLength / 4] = {};
	};
}