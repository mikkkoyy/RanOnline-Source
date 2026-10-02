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

#include "NetworkTypes.h"

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
	};
}
