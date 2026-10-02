#include "NetworkTypes.h"
#include "ServerSession.h"

namespace Modern::Network
{
	const char* ToString(SessionState state) noexcept
	{
		switch (state)
		{
		case SessionState::Connected:        return "Connected";
		case SessionState::Authenticating:   return "Authenticating";
		case SessionState::Authenticated:    return "Authenticated";
		case SessionState::CharacterSelected: return "CharacterSelected";
		case SessionState::InWorld:          return "InWorld";
		case SessionState::Closed:           return "Closed";
		}
		return "Unknown";
	}
}
