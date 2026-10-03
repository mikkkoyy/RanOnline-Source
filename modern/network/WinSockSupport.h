#pragma once

// NETWORK-001: the Winsock calls shared by every socket class in this directory.
//
// WHY THESE ARE NOT SIMPLY REPEATED IN TcpTransport.cpp AND TcpListener.cpp.
//
// The transport and the listener both need the same six things: the platform's
// per-thread error, the one sanctioned mapping from a Winsock code to a
// TransportFault, the Status that fault implies, a bounded readiness wait, a
// blocking-mode switch, and the handle width. A listener written by copying the
// transport's private helpers would be a second mapping table - and the first
// silent divergence would be a fault code that means "the peer is gone" on a
// connected socket and "there was nothing to accept" on a listening one.
//
// So the mapping lives here, once. `TcpTransport` and `TcpListener` both call it,
// which is what makes TransportFault a genuinely closed set rather than a
// per-file convention.
//
// NOT A PUBLIC HEADER IN SPIRIT, ONLY IN LOCATION. It has no include guard
// against being used from outside modern/network, but nothing outside that
// directory should need it: an application asks for an Endpoint and gets a
// Status, and the handle width is an implementation detail of this layer. The
// No HEADER in this directory includes <winsock2.h> - only the .cpp files do - so this
// header can name neither SOCKET nor a Winsock error code. WinSockSupport.cpp,
//
// THE HANDLE IS uintptr_t, NOT SOCKET. SOCKET is UINT_PTR on Windows, so the
// width is exact rather than approximate, and keeping the platform type out of
// the signature is what lets a public header of this directory be included by
// something that must not inherit Winsock's declaration order or macro set.

#include "NetworkTransport.h"
#include "types/Result.h"

#include <cstdint>

namespace Modern::Network::WinSockDetail
{
	// The platform's per-thread error code.
	//
	// Must be called immediately after a failing Winsock call and nowhere else:
	// the value is per-thread, and any subsequent Winsock call may overwrite it.
	int LastSocketError() noexcept;

	// Winsock error -> TransportFault. The ONLY place platform codes are
	// interpreted; everything above reasons about the returned fault.
	TransportFault FromSocketError(int error) noexcept;

	// TransportFault -> the ErrorCode an operation reports by value.
	//
	// WouldBlock maps to None, because the INetworkTransport::Receive contract
	// says zero bytes available is not an error. PeerClosed appears here too for
	// completeness, though it is never produced by a socket error - a clean close
	// is a zero-byte read.
	ErrorCode ErrorCodeFor(TransportFault fault) noexcept;

	// Status for a raw Winsock error, via FromSocketError then ErrorCodeFor.
	Status StatusFor(int socketError) noexcept;

	// Waits until `handle` is ready in the requested direction, or the deadline
	// passes.
	//
	// Returns WouldBlock on timeout - deliberately the same value a socket would
	// report - so a caller's "nothing yet" path does not have to distinguish
	// "select said no" from "select was not consulted". A real select() failure
	// returns the mapped fault.
	//
	// select() rather than relying on SO_RCVTIMEO / SO_SNDTIMEO alone, because
	// the per-call time-out overloads need a one-off deadline, and rewriting a
	// socket option around every call would be both slow and unsafe if two threads
	// shared the socket. The SO_*TIMEO options are still applied by TcpTransport
	// as a backstop for the window between select() reporting ready and the
	// blocking call actually starting.
	TransportFault WaitReady(std::uintptr_t handle, bool forWrite, int timeoutMilliseconds) noexcept;

	// Switches a socket between blocking and non-blocking mode.
	//
	// Returns false if the platform refused, which the caller must treat as a
	// fault: a socket left in the wrong mode makes every recv() report
	// WSAEWOULDBLOCK immediately, and a half-non-blocking connect is a subtle
	// source of send/recv surprises.
	bool SetBlocking(std::uintptr_t handle, bool blocking) noexcept;
}
