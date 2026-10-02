#include "LoginProtocol.h"

#include "NetworkCodec.h"

#include <cstring>

namespace Modern::Network
{
	namespace LoginProtocol
	{
		// RcvMsgBuffer.cpp:11-12, verbatim including the two commented-out entries.
		// Lengths: 7, 6, 6, 6, 9. The client never reuses the immediately previous
		// token (GetGarbageMsg, s_NetClient.cpp:882-888), which does not matter on
		// the wire because the receiver matches content, not position.
		const std::array<const char*, 5>& GarbageTokens()
		{
			static const std::array<const char*, 5> tokens = {
				"K9IHANA",    // 7
				"L8IDUL",     // 6
				"M7HSET",     // 6
				"N6GNET",     // 6
				"O5FDASEOT",  // 9
			};
			return tokens;
		}
	}

	namespace
	{
		// Field offsets WITHIN the NET_LOGIN_DATA payload, i.e. after the 8-byte
		// header. Derived from the struct declaration order
		// (s_NetGlobal.h:2918-2923); nChannel is 4 bytes then the char arrays pack
		// at 1-byte alignment.
		constexpr std::size_t kOffsetChannel      = 0;
		constexpr std::size_t kOffsetRandomPass   = 4;
		constexpr std::size_t kOffsetPassword     = 4 + LoginProtocol::kRandomPasswordFieldSize; // 11
		constexpr std::size_t kOffsetUserId       = kOffsetPassword + LoginProtocol::kPasswordFieldSize;   // 32
		constexpr std::size_t kOffsetEncrypt      = kOffsetUserId + LoginProtocol::kUserIdFieldSize;       // 53
		constexpr std::size_t kOffsetPadding      = kOffsetEncrypt + LoginProtocol::kEncryptFieldSize;    // 66
		constexpr std::size_t kPaddingSize        = LoginProtocol::kLoginPayloadSize - kOffsetPadding;      // 2

		static_assert(kOffsetPadding + kPaddingSize == LoginProtocol::kLoginPayloadSize,
		              "NET_LOGIN_DATA payload layout must sum to 68 bytes");
		static_assert(kOffsetPadding == 66, "encrypt field ends at byte 66 of the payload");

		// Legacy builds the field with SecureZeroMemory then StringCchCopy, so a
		// NUL-terminated string plus NUL padding. This reproduces that exactly,
		// including refusing to overflow: StringCchCopy returns STRSAFE_E_INSUFFICIENT_BUFFER
		// rather than truncating silently, and legacy ignores that return.
		//
		// Reproducing the buffer contents matters: minTea strips TRAILING NULs to
		// find the string length, so an un-terminated field would encrypt a
		// different number of bytes and produce a different wire packet.
		void WriteField(std::vector<WireU8>& field, const std::string& value)
		{
			std::fill(field.begin(), field.end(), static_cast<WireU8>(0));
			if (value.size() < field.size())
			{
				std::memcpy(field.data(), value.data(), value.size());
			}
			else
			{
				// Legacy would keep the first field.size()-1 characters and
				// NUL-terminate. Matching that keeps a too-long value on the wire
				// instead of rejecting it, which is what the shipped client does.
				const std::size_t copy = field.size() - 1;
				std::memcpy(field.data(), value.data(), copy);
			}
		}

		std::string ReadField(const std::vector<WireU8>& field)
		{
			const auto end = std::find(field.begin(), field.end(), static_cast<WireU8>(0));
			return std::string(field.begin(), end);
		}

		// Legacy's StringCchCopy from a possibly non-terminated source into a
		// zeroed destination of `capacity` bytes.
		std::string CopyOut(const WireU8* source, std::size_t available, std::size_t capacity)
		{
			std::size_t n = 0;
			while (n < available && n < capacity && source[n] != 0)
			{
				++n;
			}
			return std::string(reinterpret_cast<const char*>(source), n);
		}
	}

	bool LoginProtocol::IsLoginMessage(const WireU8* frame, std::size_t frameSize) noexcept
	{
		if (frame == nullptr || frameSize < kMessageHeaderSize)
		{
			return false;
		}
		return Codec::ReadU32(frame + 4) == LoginProtocol::kLoginMessageId;
	}

	Status LoginProtocol::Encode(const LoginRequest& request,
	                             const std::string& garbageText,
	                             const MinTea& tea,
	                             std::vector<WireU8>& out)
	{
		out.clear();

		// Only the five legacy tokens exist on this path. The server identifies the
		// token by content, so an invented one produces a packet no RAN server
		// would accept - better to refuse than to emit it.
		bool known = false;
		for (const char* token : GarbageTokens())
		{
			if (garbageText == token)
			{
				known = true;
				break;
			}
		}
		if (!known)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		const std::size_t garbageLength = garbageText.size();
		if (garbageLength == 0 || garbageLength > Protocol::kMaxPacketSize)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// dwSize counts the garbage: 76 + g. Legacy does
		// `pNmg->dwSize += nGarbageLen` (s_NetClient.cpp:909).
		const std::size_t totalSize = LoginProtocol::kLoginBodySize + garbageLength;
		if (totalSize > Protocol::kMaxPacketSize)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		out.assign(totalSize, 0);

		// ---- header ----------------------------------------------------------
		// Written at fixed offsets, not appended: `out` is already sized and the
		// garbage token and payload follow immediately after the header.
		const WireU32 declaredSize = static_cast<WireU32>(totalSize);
		for (int i = 0; i < 4; ++i)
		{
			out[i] = static_cast<WireU8>((declaredSize >> (8 * i)) & 0xFFu);
		}
		const WireU32 declaredType = LoginProtocol::kLoginMessageId;
		for (int i = 0; i < 4; ++i)
		{
			out[4 + i] = static_cast<WireU8>((declaredType >> (8 * i)) & 0xFFu);
		}

		// ---- garbage ---------------------------------------------------------
		// Sits immediately after the header, exactly where SendMsgAddGarbageValue
		// puts it and exactly where SetGarbageNum looks for it.
		std::memcpy(out.data() + kMessageHeaderSize, garbageText.data(), garbageLength);

		// ---- payload ---------------------------------------------------------
		// Everything below is offset by the garbage length.
		WireU8* payload = out.data() + kMessageHeaderSize + garbageLength;

		const WireU32 channel = static_cast<WireU32>(request.channel);
		for (int i = 0; i < 4; ++i)
		{
			payload[kOffsetChannel + i] = static_cast<WireU8>((channel >> (8 * i)) & 0xFFu);
		}

		std::vector<WireU8> fieldRandom(kRandomPasswordFieldSize, 0);
		std::vector<WireU8> fieldPassword(kPasswordFieldSize, 0);
		std::vector<WireU8> fieldUserId(kUserIdFieldSize, 0);
		std::vector<WireU8> fieldEncrypt(kEncryptFieldSize, 0);

		WriteField(fieldRandom, request.randomPassword);
		WriteField(fieldPassword, request.password);
		WriteField(fieldUserId, request.userId);
		WriteField(fieldEncrypt, request.encryptKey);

		// minTea, in place, over the full fixed field - 7, 21, 21, 13.
		//
		// Legacy IGNORES the return of these calls. For the 7-byte random password
		// the cipher declines (its ciphertext would be 8 bytes) and the field is
		// transmitted untouched. Reproducing that is mandatory: see MinTeaCodec.h,
		// "THIS IS WHY THE RANDOM PASSWORD IS NOT ENCRYPTED". A genuine internal
		// failure would also have been ignored by legacy, but silently shipping an
		// unencrypted user id would be indefensible, so only the length-decline is
		// tolerated.
		auto encryptField = [&tea](std::vector<WireU8>& field) -> Status {
			if (!tea.CanEncryptInPlace(field, field.size()))
			{
				return Ok(); // legacy declines and leaves the field alone
			}
			return tea.EncryptInPlace(field, field.size());
		};

		if (const Status s = encryptField(fieldRandom); s.IsError()) { return s; }
		if (const Status s = encryptField(fieldPassword); s.IsError()) { return s; }
		if (const Status s = encryptField(fieldUserId); s.IsError()) { return s; }
		if (const Status s = encryptField(fieldEncrypt); s.IsError()) { return s; }

		std::memcpy(payload + kOffsetRandomPass, fieldRandom.data(), fieldRandom.size());
		std::memcpy(payload + kOffsetPassword, fieldPassword.data(), fieldPassword.size());
		std::memcpy(payload + kOffsetUserId, fieldUserId.data(), fieldUserId.size());
		std::memcpy(payload + kOffsetEncrypt, fieldEncrypt.data(), fieldEncrypt.size());

		// Trailing struct padding stays zero. Legacy leaves it indeterminate and
		// never reads it; the server's dwSize check is 76 either way.
		(void) kPaddingSize;

		return Ok();
	}

	Status LoginProtocol::Decode(const WireU8* frame, std::size_t frameSize,
	                              const MinTea& tea, LoginRequest& out)
	{
		return Decode(frame, frameSize, tea, PasswordDecryptWidth::Full21, out);
	}

	Status LoginProtocol::Decode(const WireU8* frame, std::size_t frameSize,
	                              const MinTea& tea, PasswordDecryptWidth width,
	                              LoginRequest& out)
	{
		out = LoginRequest{};

		if (frame == nullptr || frameSize < kMessageHeaderSize)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		const WireU32 declaredSize = Codec::ReadU32(frame);
		const MessageId declaredType = Codec::ReadU32(frame + 4);

		if (declaredType != LoginProtocol::kLoginMessageId)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// Framing guarantees frameSize >= declaredSize; re-checking here keeps the
		// decoder safe when called directly on a transport buffer.
		if (declaredSize < kMessageHeaderSize || declaredSize > frameSize)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// ---- STEP 1: identify the garbage token, BEFORE any size validation ---
		//
		// SetGarbageNum (RcvMsgBuffer.cpp:187-211) matches the bytes at offset 8
		// against GARBAGE_DATA and returns the length, or -1. It does not guess a
		// length and it does not accept an unknown token.
		const std::size_t available = declaredSize - kMessageHeaderSize;
		const char* matchedToken = nullptr;
		for (const char* token : GarbageTokens())
		{
			const std::size_t length = std::strlen(token);
			if (length <= available &&
			    std::memcmp(frame + kMessageHeaderSize, token, length) == 0)
			{
				matchedToken = token;
				break;
			}
		}
		if (matchedToken == nullptr)
		{
			// Unknown or absent token. Legacy would treat this as a malformed
			// message; silently accepting the body without stripping would hand the
			// garbage bytes to the field decryptor as if they were ciphertext.
			return Status(ErrorCode::InvalidArgument);
		}

		const std::size_t garbageLength = std::strlen(matchedToken);

		// ---- STEP 2: strip, then the size is 76 ------------------------------
		if (available < garbageLength + kLoginPayloadSize)
		{
			return Status(ErrorCode::InvalidArgument);
		}
		if (declaredSize != LoginProtocol::kLoginBodySize + garbageLength)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		const WireU8* payload = frame + kMessageHeaderSize + garbageLength;

		out.garbage.text = matchedToken;
		out.garbage.length = garbageLength;

		// ---- STEP 3: channel, then decrypt the fields ------------------------
		WireU32 channel = 0;
		for (int i = 0; i < 4; ++i)
		{
			channel |= static_cast<WireU32>(payload[kOffsetChannel + i]) << (8 * i);
		}
		out.channel = static_cast<WireI32>(channel);

		std::vector<WireU8> fieldRandom(payload + kOffsetRandomPass,
		                                payload + kOffsetRandomPass + kRandomPasswordFieldSize);
		std::vector<WireU8> fieldPassword(payload + kOffsetPassword,
		                                  payload + kOffsetPassword + kPasswordFieldSize);
		std::vector<WireU8> fieldUserId(payload + kOffsetUserId,
		                                payload + kOffsetUserId + kUserIdFieldSize);
		std::vector<WireU8> fieldEncrypt(payload + kOffsetEncrypt,
		                                 payload + kOffsetEncrypt + kEncryptFieldSize);

		// The password decrypt window is the ONLY effect bFeatureRegisterUseMD5 has
		// (s_CAgentServerMsgLogin.cpp:637-642). Everything else is identical, which
		// is why both widths produce the same result for well-formed input.
		const std::size_t passwordWindow =
			(width == PasswordDecryptWidth::Truncated20) ? kPasswordLength : kPasswordFieldSize;

		// Legacy ignores the return of each m_Tea.decrypt. A field the cipher
		// declines is therefore left as transmitted - which for the 7-byte random
		// password means it arrives and leaves in plaintext. Same tolerance as the
		// encoder, for the same reason.
		auto decryptField = [&tea](std::vector<WireU8>& field, std::size_t window) -> Status {
			if (!tea.CanDecryptInPlace(field, window))
			{
				return Ok(); // legacy declines; the field was never transformed
			}
			return tea.DecryptInPlace(field, window);
		};

		if (const Status s = decryptField(fieldUserId, fieldUserId.size()); s.IsError()) { return s; }
		if (const Status s = decryptField(fieldPassword, passwordWindow); s.IsError()) { return s; }
		if (const Status s = decryptField(fieldRandom, fieldRandom.size()); s.IsError()) { return s; }
		if (const Status s = decryptField(fieldEncrypt, fieldEncrypt.size()); s.IsError()) { return s; }

		out.userId         = CopyOut(fieldUserId.data(), fieldUserId.size(), kUserIdLength);
		out.password       = CopyOut(fieldPassword.data(), passwordWindow, kPasswordLength);
		out.randomPassword = ReadField(fieldRandom);
		out.encryptKey     = CopyOut(fieldEncrypt.data(), fieldEncrypt.size(), kEncryptKeyLength);

		// ---- STEP 4: structural validation -----------------------------------
		// A login with no user id cannot be looked up; legacy would query the DB
		// with an empty string and fail there. Catching it here keeps the failure at
		// the protocol boundary instead of in persistence.
		if (out.userId.empty())
		{
			return Status(ErrorCode::InvalidArgument);
		}
		if (out.userId.size() > kUserIdLength)
		{
			return Status(ErrorCode::InvalidArgument);
		}
		if (out.password.size() > kPasswordLength)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		return Ok();
	}
}