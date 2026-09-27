#include "Result.h"

namespace Modern
{
	const char* ToString(ErrorCode code) noexcept
	{
		switch (code)
		{
			case ErrorCode::None:           return "None";
			case ErrorCode::InvalidArgument: return "InvalidArgument";
			case ErrorCode::NotFound:        return "NotFound";
			case ErrorCode::AlreadyExists:   return "AlreadyExists";
			case ErrorCode::InvalidState:    return "InvalidState";
			case ErrorCode::NotAllowed:      return "NotAllowed";
		}

		return "Unknown";
	}
}
