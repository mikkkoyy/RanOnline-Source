#pragma once

// VERTICAL-027: message codec.
//
// The codec owns encoding, decoding and VALIDATION. It does not execute
// gameplay, does not route, and does not hold session state. Its entire job is
// turning bytes into a validated message and back.
//
// Conceptual flow, and where each stage lives:
//
//     bytes
//       -> transport        (NetworkTransport.h)   socket or loopback
//       -> framing          (NetworkConnection.h)  message boundaries
//       -> codec            (NetworkCodec.h)       decode + validate
//       -> router           (MessageRouter.h)      id -> handler
//       -> session          (ServerSession.h)      state machine
//       -> modern core                          gameplay rules
//
// The separation matters: a malformed packet must be rejected before it can
// reach a handler, and a handler must never be handed a raw buffer.

#include "NetworkTypes.h"

#include "types/Result.h"

#include <cstring>
#include <string>
#include <vector>

namespace Modern::Network
{
	// A decoded, validated message: the header plus its payload, with the
	// payload already copied out of the transport buffer.
	//
	// Owning the payload is deliberate. A view into a reused receive buffer
	// would be cheaper, but it makes every handler responsible for noticing that
	// the buffer moved underneath it - the class of bug that only reproduces
	// under load.
	struct Message
	{
		MessageHeader header{};
		std::vector<WireU8> payload;

		// Payload length, i.e. total size minus the header.
		std::size_t PayloadSize() const noexcept
		{
			return header.size >= kMessageHeaderSize ? header.size - kMessageHeaderSize : 0;
		}

		const WireU8* PayloadData() const noexcept
		{
			return payload.empty() ? nullptr : payload.data();
		}

		bool operator==(const Message& other) const noexcept
		{
			return header.size == other.header.size && header.type == other.header.type &&
			       payload == other.payload;
		}
	};

	// Little-endian scalar encode/decode.
	//
	// Explicit, not a memcpy of a struct. See the header comment in
	// NetworkTypes.h for why the bytes still match legacy on x86 while the
	// guarantee stops being accidental.
	namespace Codec
	{
		inline void WriteU8(std::vector<WireU8>& out, WireU8 value)
		{
			out.push_back(value);
		}

		inline void WriteU16(std::vector<WireU8>& out, WireU16 value)
		{
			out.push_back(static_cast<WireU8>(value & 0xFFu));
			out.push_back(static_cast<WireU8>((value >> 8) & 0xFFu));
		}

		inline void WriteU32(std::vector<WireU8>& out, WireU32 value)
		{
			out.push_back(static_cast<WireU8>(value & 0xFFu));
			out.push_back(static_cast<WireU8>((value >> 8) & 0xFFu));
			out.push_back(static_cast<WireU8>((value >> 16) & 0xFFu));
			out.push_back(static_cast<WireU8>((value >> 24) & 0xFFu));
		}

		inline void WriteU64(std::vector<WireU8>& out, WireU64 value)
		{
			for (int i = 0; i < 8; ++i)
			{
				out.push_back(static_cast<WireU8>((value >> (8 * i)) & 0xFFu));
			}
		}

		inline void WriteF32(std::vector<WireU8>& out, float value)
		{
			// RAN transmits gameplay floats in their IEEE-754 single form. Bit
			// pattern, not decimal text, and not a platform-dependent conversion.
			WireU32 bits = 0;
			static_assert(sizeof(bits) == sizeof(value), "float must be 32-bit");
			std::memcpy(&bits, &value, sizeof(bits));
			WriteU32(out, bits);
		}

		// Strings are length-prefixed UTF-8 bytes with NO terminator: a count
		// followed by exactly that many bytes. RAN's own TCHAR fields are not
		// portable, so this is the modern form and is documented as such rather
		// than claimed as RAN-identical.
		//
		// `maxLength` bounds the count BEFORE any allocation, so a hostile length
		// prefix cannot ask for a huge buffer.
		inline Status WriteString(std::vector<WireU8>& out, const std::string& value,
		                          std::size_t maxLength)
		{
			if (value.size() > maxLength)
			{
				return Status(ErrorCode::InvalidArgument);
			}
			WriteU16(out, static_cast<WireU16>(value.size()));
			out.insert(out.end(), value.begin(), value.end());
			return Ok();
		}

		inline WireU8 ReadU8(const WireU8* data) noexcept
		{
			return data[0];
		}

		inline WireU16 ReadU16(const WireU8* data) noexcept
		{
			return static_cast<WireU16>(static_cast<WireU16>(data[0]) |
			                             (static_cast<WireU16>(data[1]) << 8));
		}

		inline WireU32 ReadU32(const WireU8* data) noexcept
		{
			return static_cast<WireU32>(data[0]) | (static_cast<WireU32>(data[1]) << 8) |
			       (static_cast<WireU32>(data[2]) << 16) |
			       (static_cast<WireU32>(data[3]) << 24);
		}

		inline WireU64 ReadU64(const WireU8* data) noexcept
		{
			WireU64 value = 0;
			for (int i = 7; i >= 0; --i)
			{
				value = (value << 8) | static_cast<WireU64>(data[i]);
			}
			return value;
		}

		inline float ReadF32(const WireU8* data) noexcept
		{
			const WireU32 bits = ReadU32(data);
			float value = 0.0f;
			std::memcpy(&value, &bits, sizeof(value));
			return value;
		}

		// A cursor for reading a payload field by field.
		//
		// Every read checks the remaining length first. That is the whole point of
		// the type: a truncated or hostile payload produces an error, never an
		// out-of-bounds read.
		class Reader
		{
		public:
			Reader(const WireU8* data, std::size_t size) noexcept
				: m_data(data), m_size(data != nullptr ? size : 0)
			{
			}

			std::size_t Remaining() const noexcept { return m_size - m_offset; }
			std::size_t Offset() const noexcept { return m_offset; }
			bool Exhausted() const noexcept { return m_offset >= m_size; }

			Status ReadU8(WireU8& out) noexcept
			{
				if (Remaining() < 1) { return Status(ErrorCode::InvalidArgument); }
				// Qualified: the member names these, so an unqualified call would
				// recurse into the member rather than reach the free function.
				out = Codec::ReadU8(m_data + m_offset);
				m_offset += 1;
				return Ok();
			}

			Status ReadU16(WireU16& out) noexcept
			{
				if (Remaining() < 2) { return Status(ErrorCode::InvalidArgument); }
				out = Codec::ReadU16(m_data + m_offset);
				m_offset += 2;
				return Ok();
			}

			Status ReadU32(WireU32& out) noexcept
			{
				if (Remaining() < 4) { return Status(ErrorCode::InvalidArgument); }
				out = Codec::ReadU32(m_data + m_offset);
				m_offset += 4;
				return Ok();
			}

			Status ReadU64(WireU64& out) noexcept
			{
				if (Remaining() < 8) { return Status(ErrorCode::InvalidArgument); }
				out = Codec::ReadU64(m_data + m_offset);
				m_offset += 8;
				return Ok();
			}

			Status ReadF32(float& out) noexcept
			{
				if (Remaining() < 4) { return Status(ErrorCode::InvalidArgument); }
				out = Codec::ReadF32(m_data + m_offset);
				m_offset += 4;
				return Ok();
			}

			Status ReadString(std::string& out, std::size_t maxLength) noexcept
			{
				WireU16 count = 0;
				if (const Status status = ReadU16(count); status.IsError())
				{
					return status;
				}
				if (count > maxLength || Remaining() < count)
				{
					return Status(ErrorCode::InvalidArgument);
				}
				out.assign(reinterpret_cast<const char*>(m_data + m_offset), count);
				m_offset += count;
				return Ok();
			}

			// A fixed-size run of bytes. `count` is validated against what is
			// actually left, so a declared length can never over-read.
			Status ReadBytes(std::vector<WireU8>& out, std::size_t count) noexcept
			{
				if (Remaining() < count) { return Status(ErrorCode::InvalidArgument); }
				out.assign(m_data + m_offset, m_data + m_offset + count);
				m_offset += count;
				return Ok();
			}

		private:
			const WireU8* m_data = nullptr;
			std::size_t    m_size = 0;
			std::size_t    m_offset = 0;
		};

		// ---- message level -------------------------------------------------

		// Encodes a header plus payload. `payload` must not itself contain a
		// header; the total size is computed here so a caller cannot disagree
		// with the wire about how long the message is.
		inline Status EncodeMessage(const MessageHeader& header,
		                            const std::vector<WireU8>& payload,
		                            std::vector<WireU8>& out)
		{
			const std::size_t total = kMessageHeaderSize + payload.size();
			if (total > Protocol::kMaxPacketSize)
			{
				return Status(ErrorCode::InvalidArgument);
			}
			if (header.type == 0)
			{
				return Status(ErrorCode::InvalidArgument);
			}

			out.clear();
			out.reserve(total);
			WriteU32(out, static_cast<WireU32>(total));
			WriteU32(out, header.type);
			out.insert(out.end(), payload.begin(), payload.end());
			return Ok();
		}

		// Validates and decodes exactly one message from the front of `bytes`.
		//
		// This is the security boundary. It rejects, without allocating:
		//   - a size field of zero
		//   - a size smaller than the header itself
		//   - a size larger than the protocol maximum
		//   - a size larger than the bytes actually available (truncation)
		//   - a zero type
		//
		// The first three mirror RcvMsgBuffer.cpp:112-118 exactly, including its
		// "packet crash fix" comment; the zero-type check is modern hardening that
		// legacy does not perform.
		inline Status DecodeMessage(const WireU8* bytes, std::size_t available,
		                            Message& out)
		{
			if (bytes == nullptr || available < kMessageHeaderSize)
			{
				return Status(ErrorCode::InvalidArgument);
			}

			MessageHeader header;
			header.size = ReadU32(bytes);
			header.type = ReadU32(bytes + 4);

			if (header.size < kMessageHeaderSize || header.size > Protocol::kMaxPacketSize)
			{
				return Status(ErrorCode::InvalidArgument);
			}
			if (header.type == 0)
			{
				return Status(ErrorCode::InvalidArgument);
			}
			if (header.size > available)
			{
				return Status(ErrorCode::InvalidArgument);
			}

			out.header = header;
			const std::size_t payloadSize = header.size - kMessageHeaderSize;
			out.payload.assign(bytes + kMessageHeaderSize, bytes + kMessageHeaderSize + payloadSize);
			return Ok();
		}
	}
}
