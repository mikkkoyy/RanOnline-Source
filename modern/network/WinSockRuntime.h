#pragma once

// NETWORK-001: Winsock lifetime, and the ONLY place WSAStartup/WSACleanup appear
// outside the socket classes themselves.
//
// WHY THIS IS A SEPARATE TYPE RATHER THAN A HELPER CALL INSIDE TcpTransport.
//
// Three requirements pull in three different directions, and folding them into
// the transport constructor is the mistake this type exists to prevent:
//
//   1. WSAStartup must be called before ANY Winsock call and its cost is
//      process-wide, so calling it per transport - or, absurdly, per packet -
//      is wrong. But Winsock is reference counted, and the OS decrements a
//      matching count per WSACleanup, so a program that opens two sockets and
//      closes one must NOT clean up.
//
//   2. The cleanup must happen when the OWNING network subsystem shuts down, not
//      when some transport happens to be destroyed. A client that tears down and
//      rebuilds its connection would otherwise cycle Winsock down and up
//      mid-session, which is a real and very confusing class of bug.
//
//   3. Core must never learn about it. This header is in ModernNetwork, it links
//      ws2_32, and nothing in modern/core can reach it.
//
// So: reference counted, with an explicit `Acquire` for a subsystem that wants to
// hold Winsock up across transport lifetimes, and an automatic acquire per socket
// for the case that needs nothing from the application.
//
// A thread may not race Winsock shutdown against its own use of it, so the
// reference count is guarded by a mutex. That is the one lock in the modern tree
// that exists because of a platform rather than because of a design, and it is
// held for a handful of instructions.

#include "types/Result.h"

namespace Modern::Network::WinSock
{
	// The number of live references. Zero means Winsock is not initialised.
	//
	// Exposed so a test can assert that acquiring and releasing is actually
	// balanced - a leak here is invisible in every other way, because a process
	// that calls WSACleanup one time too few still appears to work.
	// Thread-safe; a snapshot, not a synchronisation point.
	std::size_t ReferenceCount() noexcept;

	// True when Winsock is initialised right now.
	bool IsReady() noexcept;

	// Starts Winsock if it is not already running, and takes a reference.
	//
	// Reference counted and idempotent in effect: N successful calls require N
	// calls to Release. Returns InvalidState only if the platform refused
	// WSAStartup, in which case no reference is taken and every socket call
	// would fail - so a caller should treat an error here as fatal for the whole
	// network layer rather than retrying.
	Status Acquire() noexcept;

	// Drops a reference, calling WSACleanup when the last one goes.
	//
	// Calling this more times than Acquire succeeded is a programming error and
	// is ignored: the alternative is decrementing a counter below zero and
	// calling WSACleanup when Winsock is not ours to close.
	void Release() noexcept;
}
