#pragma once

// VERTICAL-027: transport boundary.
//
// The transport owns bytes moving between two endpoints. It knows about
// endpoints, timeouts and disconnects. It does NOT know about messages,
// framing, sessions, or gameplay.
//
// Deliberately NOT part of this header: Character, Stats, Equipment, Skills,
// Damage, HP, MP, SP, or any legacy type. If a change needs one of those, it
// belongs above the transport.
//
// The interface is an interface because legacy proves two different transports
// must exist - a real socket server (CServer, IOCP, s_NetClient) and the
// loopback pair below - and because the tests need the second one to stay
// deterministic. That is a concrete reason, not abstraction for its own sake.
//
// Legacy: Lib_Network/s_CServer.h owns IOCP receive/send overlapped, worker
// threads (S_HEURISTIC_NUM per CPU), an accept thread and an update thread.
// s_NetGlobal.h:122-123 note "This version not support UDP protocol", and
// NET_DEFAULT_PORT is 5001.
//
// NETWORK-001 adds the second implementation the comment above anticipated:
// TcpTransport (TcpTransport.h) and TcpListener (TcpListener.h), which are real
// Winsock sockets. The loopback pair stays, because a test that controls its own
// chunk boundaries is still worth having - a socket decides its own.

#include "NetworkTypes.h"

// NETWORK-001: Status is named in this header's own interface - Send and Receive
// return it - so this header must include its declaration rather than relying on
// whoever includes this one to have included types/Result.h first. That reliance
// held only because every existing consumer happened to do so; a translation unit
// reaching TcpTransport.h, which includes NetworkTransport.h before Result.h,
// broke it. An interface that names a type includes that type.
#include "types/Result.h"

#include <cstdint>
#include <string>

namespace Modern::Network
{
	// Where an endpoint lives. Legacy resolves these from configuration files
	// (Lib_Network/s_CCfg.cpp) and from the Login server's channel table.
	struct Endpoint
	{
		std::string host;
		WireU16     port = 0;

		bool operator==(const Endpoint& other) const noexcept
		{
			return host == other.host && port == other.port;
		}
	};

	enum class TransportState : uint8_t
	{
		Closed = 0,
		Open   = 1,
	};

	// Why a transport stopped.
	//
	// Distinguishing the two matters: `Closed` is an ordinary, expected end (the
	// peer finished), while `Faulted` means something went wrong and the
	// session must not be silently reused.
	enum class TransportError : uint8_t
	{
		None = 0,
		Closed,
		Faulted,
	};

	// NETWORK-001: why a socket operation did not do what was asked.
	//
	// This is the modern replacement for a raw WSAGetLastError() value reaching
	// application code. It is a CLOSED set, and that is the point: a caller can
	// switch over it exhaustively, and adding a platform value later is a
	// deliberate, reviewable change rather than an accident. The underlying
	// platform code is still available for diagnostics through
	// `INetworkTransport::NativeError()`, but no application decision is ever
	// supposed to be made on it.
	//
	// The split that matters most:
	//
	//   WouldBlock  NOTHING AVAILABLE, NOT A FAILURE. The peer's data has not
	//               arrived yet within the caller's timeout. See the `Receive`
	//               contract below.
	//   PeerClosed  the peer sent an orderly FIN. Also not a socket failure: the
	//               connection ended the way connections are supposed to end.
	//
	// Everything else is a genuine fault and means the connection must be
	// dropped rather than reused.
	enum class TransportFault : uint8_t
	{
		None = 0,

		// The caller's timeout elapsed with nothing available. The transport is
		// still healthy; only this call learned nothing.
		WouldBlock,

		// A blocking operation could not finish inside the deadline it was given.
		Timeout,

		// recv() returned 0: the peer closed its side cleanly.
		PeerClosed,

		// The peer actively refused the connection (RST or an empty backlog).
		ConnectionRefused,

		// No route to the host, or the host is not reachable from here.
		HostUnreachable,

		// The connection was reset mid-stream. Unrecoverable: any partially sent
		// message is now unresynchronisable.
		ConnectionReset,

		// The operation was issued on a transport that is not connected.
		NotConnected,

		// The address could not be parsed, resolved, or bound.
		AddressInvalid,

		// The local network stack is down or unavailable.
		NetworkUnavailable,

		// The platform refused the operation as unsupported.
		NotSupported,

		// send() could not hand over the whole buffer inside the deadline. The
		// stream is desynchronised, so the connection must be dropped.
		PartialSend,

		// Anything the mapping did not recognise. Kept distinct from None so an
		// unhandled platform error is visible rather than silently treated as
		// success.
		Unexpected,
	};

	// Stable name for logs and test output.
	const char* ToString(TransportFault fault) noexcept;

	// The one thing a transport must be able to do.
	//
	// `Send` appends to the outbound queue. `Receive` DRAINS up to
	// `maxBytes` and returns the count, so a partial message accumulates in the
	// framing layer rather than being reassembled here. Framing is a codec
	// concern; a transport that understood message boundaries would be doing the
	// codec's job.
	class INetworkTransport
	{
	public:
		virtual ~INetworkTransport() = default;

		virtual TransportState State() const noexcept = 0;

		// Bytes the transport has queued but not yet handed to the peer. A real
		// socket transport flushes on its own; the loopback transport lets a test
		// assert that a partial flush is possible, which is the case that breaks
		// framing code in the field.
		virtual std::size_t PendingBytes() const noexcept = 0;

		virtual Status Send(const WireU8* data, std::size_t size) = 0;

		// Returns the number of bytes copied into `out`. 0 means "nothing
		// available yet", which is NOT an error.
		virtual Status Receive(WireU8* out, std::size_t maxBytes, std::size_t& received) = 0;

		virtual void Disconnect() noexcept = 0;

		virtual Endpoint LocalEndpoint() const noexcept = 0;
		virtual Endpoint RemoteEndpoint() const noexcept = 0;

		// ---- NETWORK-001: diagnosis -------------------------------------------

		// Why the most recent operation did not do what was asked, or None if it
		// did. A real transport overwrites this on every operation, so it is a
		// "what just happened" register rather than a sticky error: read it
		// immediately after the call it explains.
		//
		// Has a default implementation so the loopback pair - which has no
		// failures to report beyond a closed channel - needs no change.
		virtual TransportFault Fault() const noexcept { return TransportFault::None; }

		// The raw platform error code behind the last fault, for logs.
		//
		// This is the ONE place a Windows value may be surfaced, and it exists so
		// that a bug report can be diagnosed. Nothing above the transport may
		// branch on it: application behaviour goes through Fault() and the Status
		// the operation returned. Always 0 for a transport that has no platform.
		virtual int NativeError() const noexcept { return 0; }

		// True when bytes may be sent and received.
		//
		// Not virtual: it is exactly State() == Open, and a transport that
		// disagreed with its own state would be a bug rather than a variation.
		bool IsConnected() const noexcept { return State() == TransportState::Open; }
	};
}
