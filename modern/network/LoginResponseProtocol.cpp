#include "LoginResponseProtocol.h"

#include "NetworkCodec.h"

#include <cstring>

namespace Modern::Network
{
	const char* ToString(LoginFeedbackResult result) noexcept
	{
		switch (result)
		{
		case LoginFeedbackResult::Ok:                 return "Ok";
		case LoginFeedbackResult::Fail:               return "Fail";
		case LoginFeedbackResult::System:             return "System";
		case LoginFeedbackResult::Usage:              return "Usage";
		case LoginFeedbackResult::Duplicate:          return "Duplicate";
		case LoginFeedbackResult::Incorrect:          return "Incorrect";
		case LoginFeedbackResult::IpBan:              return "IpBan";
		case LoginFeedbackResult::Block:              return "Block";
		case LoginFeedbackResult::Uncon:              return "Uncon";
		case LoginFeedbackResult::Expired:            return "Expired";
		case LoginFeedbackResult::GidError:           return "GidError";
		case LoginFeedbackResult::UidError:           return "UidError";
		case LoginFeedbackResult::Unknown:            return "Unknown";
		case LoginFeedbackResult::SsnHead:            return "SsnHead";
		case LoginFeedbackResult::Adult:              return "Adult";
		case LoginFeedbackResult::ChannelFull:        return "ChannelFull";
		case LoginFeedbackResult::ThaiUnder18Time:    return "ThaiUnder18Time";
		case LoginFeedbackResult::ThaiUnder18ThreeHour: return "ThaiUnder18ThreeHour";
		case LoginFeedbackResult::ThaiOver18Time:     return "ThaiOver18Time";
		case LoginFeedbackResult::RandomPass:         return "RandomPass";
		case LoginFeedbackResult::PassOk:             return "PassOk";
		case LoginFeedbackResult::AlreadyOffline:     return "AlreadyOffline";
		case LoginFeedbackResult::SecIdAlready:       return "SecIdAlready";
		case LoginFeedbackResult::RequireTime:        return "RequireTime";
		}
		return "Unrecognised";
	}

	namespace
	{
		// Appends a NUL-terminated fixed field, matching legacy's
		// `memset` then `StringCchCopy(dst, CAP-1, src)` pattern.
		//
		// `maxChars` is the caller's proven StringCchCopy limit, passed explicitly
		// because the two fields do NOT share one - see kEmailMaxChars and
		// kDaumGidMaxChars. Truncating rather than failing is what legacy does, and
		// what keeps an over-long value from shifting every later field.
		void AppendField(std::vector<WireU8>& out, std::size_t offset,
		                 const std::string& value, std::size_t maxChars)
		{
			const std::size_t copy = value.size() < maxChars ? value.size() : maxChars;
			if (copy > 0)
			{
				std::memcpy(out.data() + offset, value.data(), copy);
			}
			// Terminator and any trailing bytes stay zero from the resize above.
		}

		void PutU16(std::vector<WireU8>& batch, std::size_t offset, std::uint16_t value)
		{
			batch[offset + 0] = static_cast<WireU8>(value & 0xFFu);
			batch[offset + 1] = static_cast<WireU8>((value >> 8) & 0xFFu);
		}

		void PutU32(std::vector<WireU8>& batch, std::size_t offset, std::uint32_t value)
		{
			for (int i = 0; i < 4; ++i)
			{
				batch[offset + i] = static_cast<WireU8>((value >> (8 * i)) & 0xFFu);
			}
		}

		std::string ReadField(const WireU8* data, std::size_t size)
		{
			const auto end = static_cast<const WireU8*>(
				std::memchr(data, 0, size));
			if (end == nullptr)
			{
				return std::string(reinterpret_cast<const char*>(data), size);
			}
			return std::string(reinterpret_cast<const char*>(data),
			                   static_cast<std::size_t>(end - data));
		}
	}

	bool LoginResponse::IsLoginFeedback(const WireU8* frame, std::size_t frameSize) noexcept
	{
		if (frame == nullptr || frameSize < kMessageHeaderSize)
		{
			return false;
		}
		return Codec::ReadU32(frame + 4) == LoginResponse::kLoginFeedbackMessageId;
	}

	Status LoginResponse::Append(std::vector<WireU8>& batch, const LoginFeedback& feedback)
	{
		// Reject a value that cannot be represented rather than truncating it: a
		// silently wrapped nResult would be read as a different error entirely.
		const auto rawResult = static_cast<std::uint32_t>(feedback.result);
		if (rawResult > 0xFFFFu)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// A chaRemain above MAX_CHAR_LENGTH is a server-side data error. Legacy
		// clamps it to zero and logs (s_CAgentServerMsgLogin.cpp:1096-1108) rather
		// than refusing the response, so the clamp is reproduced here instead of
		// failing the whole send.
		if (feedback.chaRemain > LoginResponse::kMaxChaRemain)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		const std::size_t start = batch.size();
		batch.resize(start + LoginResponse::kFeedbackBodySize, 0);

		PutU32(batch, start + 0, static_cast<WireU32>(LoginResponse::kFeedbackBodySize));
		PutU32(batch, start + 4, LoginResponse::kLoginFeedbackMessageId);

		AppendField(batch, start + LoginResponse::kOffsetDaumGid,
		            feedback.daumGid, LoginResponse::kDaumGidMaxChars);

		PutU16(batch, start + LoginResponse::kOffsetResult, static_cast<WireU16>(rawResult));
		PutU16(batch, start + LoginResponse::kOffsetChaRemain, feedback.chaRemain);

		PutU32(batch, start + LoginResponse::kOffsetExtremeM,
		       static_cast<WireU32>(feedback.extremeM));
		PutU32(batch, start + LoginResponse::kOffsetExtremeW,
		       static_cast<WireU32>(feedback.extremeW));
		PutU32(batch, start + LoginResponse::kOffsetCheckFlag,
		       static_cast<WireU32>(feedback.checkFlag));
		PutU32(batch, start + LoginResponse::kOffsetPatchProgramVer,
		       static_cast<WireU32>(feedback.patchProgramVer));
		PutU32(batch, start + LoginResponse::kOffsetGameProgramVer,
		       static_cast<WireU32>(feedback.gameProgramVer));

		PutU32(batch, start + LoginResponse::kOffsetGameTime, feedback.gameTime);
		PutU32(batch, start + LoginResponse::kOffsetPremiumPoint, feedback.premiumPoint);
		PutU32(batch, start + LoginResponse::kOffsetCombatPoint, feedback.combatPoint);

		AppendField(batch, start + LoginResponse::kOffsetEmail,
		            feedback.email, LoginResponse::kEmailMaxChars);

		// Padding at offsets 29, 34-35 and 119 stays zero. Legacy never initialises
		// it explicitly - its memset(this,0,sizeof(...)) in the constructor does,
		// so zero is exactly what a freshly constructed legacy struct sends.
		return Ok();
	}

	Status LoginResponse::Decode(const WireU8* frame, std::size_t frameSize,
	                            LoginFeedback& out)
	{
		out = LoginFeedback{};

		if (frame == nullptr || frameSize < kMessageHeaderSize)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		const WireU32 declaredSize = Codec::ReadU32(frame);
		if (Codec::ReadU32(frame + 4) != LoginResponse::kLoginFeedbackMessageId)
		{
			return Status(ErrorCode::InvalidArgument);
		}
		if (declaredSize != LoginResponse::kFeedbackBodySize)
		{
			return Status(ErrorCode::InvalidArgument);
		}
		if (frameSize < declaredSize)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		out.daumGid = ReadField(frame + LoginResponse::kOffsetDaumGid,
		                        LoginResponse::kDaumGidFieldSize);

		out.result = static_cast<LoginFeedbackResult>(
			Codec::ReadU16(frame + LoginResponse::kOffsetResult));
		out.chaRemain = Codec::ReadU16(frame + LoginResponse::kOffsetChaRemain);

		out.extremeM = static_cast<WireI32>(Codec::ReadU32(frame + LoginResponse::kOffsetExtremeM));
		out.extremeW = static_cast<WireI32>(Codec::ReadU32(frame + LoginResponse::kOffsetExtremeW));
		out.checkFlag = static_cast<WireI32>(Codec::ReadU32(frame + LoginResponse::kOffsetCheckFlag));
		out.patchProgramVer =
			static_cast<WireI32>(Codec::ReadU32(frame + LoginResponse::kOffsetPatchProgramVer));
		out.gameProgramVer =
			static_cast<WireI32>(Codec::ReadU32(frame + LoginResponse::kOffsetGameProgramVer));

		out.gameTime = Codec::ReadU32(frame + LoginResponse::kOffsetGameTime);
		out.premiumPoint = Codec::ReadU32(frame + LoginResponse::kOffsetPremiumPoint);
		out.combatPoint = Codec::ReadU32(frame + LoginResponse::kOffsetCombatPoint);

		out.email = ReadField(frame + LoginResponse::kOffsetEmail,
		                      LoginResponse::kEmailFieldSize);

		return Ok();
	}
}