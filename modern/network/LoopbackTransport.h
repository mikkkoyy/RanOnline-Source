#pragma once

// VERTICAL-027: in-memory transport.
//
// A connected pair of byte queues. Both halves are the SAME type, which is what
// makes it useful: a test drives one side's Send and asserts on the other
// side's Receive, exactly as it would against a socket, with no timing and no
// OS.
//
// This exists because the framing layer needs a way to hand over data in
// arbitrary chunks. A real socket decides its own chunking, and the bug that
// costs the most time is a framer that only works when a whole message arrives
// at once. With this transport a test can send three bytes at a time.
//
// It is also genuinely useful outside tests: a single-process harness (the
// emulator, a future replay tool) can run client and server over it without a
// socket. That is why it lives here rather than in the test target.
//
// COPY SAFETY - and why this is shared_ptr rather than a raw back-pointer.
//
// The first version of this type held `LoopbackTransport* m_peer` and set the
// pointers inside CreatePair. That is wrong: `CreatePair` returns by value, so
// the returned halves are COPIES of the locals, and each copy's back-pointer
// still referenced a local that died at the return. The result was a silent
// use-after-free that showed up as an immediate access violation with no output
// at all - which is a miserable thing to debug.
//
// Both halves now share one heap channel, so a copy of either half is still
// wired to the right peer. That is a real requirement of a value type that is
// returned from a factory, not an optimisation.

#include "NetworkTransport.h"

#include "types/Result.h"

#include <deque>
#include <memory>
#include <utility>

namespace Modern::Network
{
	class LoopbackTransport final : public INetworkTransport
	{
	public:
		LoopbackTransport() = default;

		// Creates a connected pair. Either half may Send; whatever one writes
		// appears in the other half's Receive.
		static std::pair<LoopbackTransport, LoopbackTransport> CreatePair()
		{
			auto channel = std::make_shared<Channel>();
			LoopbackTransport client;
			LoopbackTransport server;
			client.Attach(channel, /*isClient=*/true);
			server.Attach(channel, /*isClient=*/false);
			return { client, server };
		}

		TransportState State() const noexcept override
		{
			return (m_channel && m_channel->open) ? TransportState::Open : TransportState::Closed;
		}

		// Bytes the peer has sent that this half has not read yet.
		std::size_t PendingBytes() const noexcept override
		{
			return m_channel ? Inbound().size() : 0;
		}

		Status Send(const WireU8* data, std::size_t size) override
		{
			if (data == nullptr && size != 0)
			{
				return Status(ErrorCode::InvalidArgument);
			}
			if (!m_channel || !m_channel->open)
			{
				return Status(ErrorCode::InvalidState);
			}
			if (size != 0)
			{
				std::deque<WireU8>& out = Outbound();
				out.insert(out.end(), data, data + size);
			}
			return Ok();
		}

		Status Receive(WireU8* out, std::size_t maxBytes, std::size_t& received) override
		{
			received = 0;
			if (out == nullptr && maxBytes != 0)
			{
				return Status(ErrorCode::InvalidArgument);
			}
			if (!m_channel || !m_channel->open)
			{
				return Status(ErrorCode::InvalidState);
			}

			std::deque<WireU8>& in = Inbound();
			const std::size_t take = in.size() < maxBytes ? in.size() : maxBytes;
			for (std::size_t i = 0; i < take; ++i)
			{
				out[i] = in.front();
				in.pop_front();
			}
			received = take;
			return Ok();
		}

		void Disconnect() noexcept override
		{
			if (m_channel)
			{
				// Both halves observe the same channel, so this closes the pair.
				m_channel->open = false;
			}
		}

		Endpoint LocalEndpoint() const noexcept override { return m_local; }
		Endpoint RemoteEndpoint() const noexcept override { return m_remote; }

	private:
		// One channel, two queues, one per direction.
		//
		// The distinction between Outbound and Inbound is the whole correctness
		// question in this type, and getting it wrong is silent: bytes land in a
		// queue nobody reads and every assertion reports zero. A half SENDS into
		// its Outbound and RECEIVES from its Inbound; with exactly two halves,
		// my Outbound is the peer's Inbound.
		struct Channel
		{
			std::deque<WireU8> clientToServer;
			std::deque<WireU8> serverToClient;
			bool open = true;
		};

		// Which side THIS half is. It lives on the half, not on the channel: the
		// channel is shared, so a flag there would be overwritten by whichever
		// half attached last.
		std::deque<WireU8>& Outbound() noexcept
		{
			return m_isClient ? m_channel->clientToServer : m_channel->serverToClient;
		}

		const std::deque<WireU8>& Outbound() const noexcept
		{
			return m_isClient ? m_channel->clientToServer : m_channel->serverToClient;
		}

		std::deque<WireU8>& Inbound() noexcept
		{
			return m_isClient ? m_channel->serverToClient : m_channel->clientToServer;
		}

		const std::deque<WireU8>& Inbound() const noexcept
		{
			return m_isClient ? m_channel->serverToClient : m_channel->clientToServer;
		}

		void Attach(const std::shared_ptr<Channel>& channel, bool isClient) noexcept
		{
			m_channel  = channel;
			m_isClient = isClient;
			m_local    = isClient ? Endpoint{ "loopback:client", 0 } : Endpoint{ "loopback:server", 0 };
			m_remote   = isClient ? Endpoint{ "loopback:server", 0 } : Endpoint{ "loopback:client", 0 };
		}

		std::shared_ptr<Channel> m_channel;
		bool                     m_isClient = true;
		Endpoint                 m_local;
		Endpoint                 m_remote;
	};
}
