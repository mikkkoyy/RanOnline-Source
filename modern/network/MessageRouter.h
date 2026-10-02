#pragma once

// VERTICAL-027: message routing.
//
// Routing maps a validated MessageId to a handler. It owns neither transport,
// nor framing, nor session state, nor gameplay.
//
// Handlers receive a decoded `Message`, never a raw buffer. That is the property
// worth defending: it means a handler cannot accidentally read past the end of
// a packet, and it means the security boundary (Codec::DecodeMessage) has
// already run before any gameplay code is reached.

#include "NetworkCodec.h"
#include "NetworkTypes.h"

#include "types/Result.h"

#include <functional>
#include <unordered_map>
#include <vector>

namespace Modern::Network
{
	// A handler's result. `Handled` false means the message was valid but the
	// handler chose not to act - distinct from an error, which means something
	// was wrong.
	struct DispatchResult
	{
		Status status  = Status(ErrorCode::None);
		bool   handled = false;

		static DispatchResult Handled() noexcept { return { Ok(), true }; }
		static DispatchResult Ignored() noexcept { return { Ok(), false }; }
		static DispatchResult Failed(Status s) noexcept { return { s, false }; }
	};

	using MessageHandler = std::function<DispatchResult(const Message&)>;

	// A handler bound to a session, so it can reach that session's state without
	// the router knowing what a session is.
	using BoundMessageHandler = std::function<DispatchResult(void* owner, const Message&)>;

	// id -> handler, per connection.
	//
	// Per-connection rather than global on purpose: two clients on one server have
	// different session state, and a shared handler table is how one client ends
	// up acting on another's session.
	class MessageRouter
	{
	public:
		using Owner = void*;

		// Registers a handler for one message id on one owner.
		//
		// Returns AlreadyExists rather than overwriting, because a silently
		// replaced handler is indistinguishable from a working one until it
		// misroutes in production.
		Status Register(Owner owner, MessageId id, BoundMessageHandler handler)
		{
			if (owner == nullptr || handler == nullptr || id == 0)
			{
				return Status(ErrorCode::InvalidArgument);
			}
			const Key key{ owner, id };
			if (m_handlers.find(key) != m_handlers.end())
			{
				return Status(ErrorCode::AlreadyExists);
			}
			m_handlers.emplace(key, std::move(handler));
			return Ok();
		}

		bool Has(Owner owner, MessageId id) const
		{
			return m_handlers.find(Key{ owner, id }) != m_handlers.end();
		}

		// Routes one validated message.
		//
		// An unknown id is NOT an error: RAN has hundreds of message constants and
		// a build will legitimately see ids it does not implement. It is reported
		// as Ignored so the caller can log and carry on, which is also what a real
		// client needs when the server is newer than it is.
		DispatchResult Dispatch(Owner owner, const Message& message)
		{
			const auto it = m_handlers.find(Key{ owner, message.header.type });
			if (it == m_handlers.end())
			{
				return DispatchResult::Ignored();
			}
			return it->second(owner, message);
		}

		// Drops every handler for one owner. Called when a session ends, so a
		// long-lived server does not accumulate registrations for dead sessions.
		std::size_t Clear(Owner owner)
		{
			std::size_t removed = 0;
			for (auto it = m_handlers.begin(); it != m_handlers.end();)
			{
				if (it->first.owner == owner)
				{
					it = m_handlers.erase(it);
					++removed;
				}
				else
				{
					++it;
				}
			}
			return removed;
		}

		std::size_t Size() const noexcept { return m_handlers.size(); }

	private:
		struct Key
		{
			Owner     owner;
			MessageId id;

			bool operator==(const Key& other) const noexcept
			{
				return owner == other.owner && id == other.id;
			}
		};

		struct KeyHash
		{
			std::size_t operator()(const Key& key) const noexcept
			{
				const auto a = std::hash<const void*>{}(key.owner);
				const auto b = std::hash<WireU32>{}(key.id);
				return a ^ (b + 0x9e3779b9u + (a << 6) + (a >> 2));
			}
		};

		std::unordered_map<Key, BoundMessageHandler, KeyHash> m_handlers;
	};
}
