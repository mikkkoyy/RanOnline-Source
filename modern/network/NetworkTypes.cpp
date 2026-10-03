#include "NetworkTypes.h"
#include "NetworkTransport.h"
#include "ServerSession.h"

namespace Modern::Network
{
	// NETWORK-001.
	//
	// Lives here rather than in TcpTransport.cpp because TransportFault is part of
	// the transport ABSTRACTION - it is declared in NetworkTransport.h, next to
	// the interface that reports it, and the loopback transport's absence of one
	// is a statement about the loopback, not about this file's dependencies. A
	// caller rendering a fault for a log must not have to link a Winsock backend
	// to obtain the name.
	//
	// Exhaustive, and the default is a visible answer rather than a silent one: an
	// unrecognised value prints as "Unrecognised" instead of borrowing a real
	// fault's name, so a stale caller is noticeable in a log.
	const char* ToString(TransportFault fault) noexcept
	{
		switch (fault)
		{
		case TransportFault::None:               return "None";
		case TransportFault::WouldBlock:         return "WouldBlock";
		case TransportFault::Timeout:            return "Timeout";
		case TransportFault::PeerClosed:         return "PeerClosed";
		case TransportFault::ConnectionRefused:  return "ConnectionRefused";
		case TransportFault::HostUnreachable:    return "HostUnreachable";
		case TransportFault::ConnectionReset:    return "ConnectionReset";
		case TransportFault::NotConnected:       return "NotConnected";
		case TransportFault::AddressInvalid:     return "AddressInvalid";
		case TransportFault::NetworkUnavailable: return "NetworkUnavailable";
		case TransportFault::NotSupported:       return "NotSupported";
		case TransportFault::PartialSend:        return "PartialSend";
		case TransportFault::Unexpected:         return "Unexpected";
		}
		return "Unrecognised";
	}

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
