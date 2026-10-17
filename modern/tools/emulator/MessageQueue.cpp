// TOOLS-002: headless message-processing boundary implementation.
//
// See MessageQueue.h for the contract. This component mirrors the legacy
// emulator's emulation message buffer (CEMULMSG / g_vecMsg in
// legacy/Lib_Client/DxGlobalStage.cpp) -- delayed, single-delivery -- but
// supports per-message delays and an injected handler, with no legacy
// dependencies.

#include "MessageQueue.h"

#include <algorithm>
#include <utility>

namespace Modern::Tools::Emulator
{

	MessageQueue::~MessageQueue()
	{
		clear();
	}

	MessageQueue::MessageQueue(MessageQueue&& other) noexcept
		: m_clock(other.m_clock)
		, m_queue(std::move(other.m_queue))
	{
	}

	MessageQueue& MessageQueue::operator=(MessageQueue&& other) noexcept
	{
		if (this != &other)
		{
			clear();
			m_clock = other.m_clock;
			m_queue = std::move(other.m_queue);
		}
		return *this;
	}

	bool MessageQueue::enqueue(const std::uint8_t* data,
	                           std::size_t         size,
	                           float               delaySeconds) noexcept
	{
		if (data == nullptr || size == 0u)
		{
			return false;
		}

		PendingMessage msg;
		msg.dueTime   = m_clock + delaySeconds;
		msg.size      = size;
		msg.data      = std::make_unique<std::uint8_t[]>(size);
		if (msg.data == nullptr)
		{
			return false;
		}
		std::copy_n(data, size, msg.data.get());

		m_queue.push_back(std::move(msg));
		return true;
	}

	std::size_t MessageQueue::update(float       elapsedSeconds,
	                                 Handler     handler) noexcept
	{
		m_clock += elapsedSeconds;

		std::size_t delivered = 0u;

		// Scan all messages and deliver any whose due time has arrived.
		// Unlike the legacy buffer (which only checked the front because
		// every message shared the same fixed delay), this scan handles
		// per-message delays correctly: a message enqueued later with a
		// shorter delay delivers before an earlier message with a longer
		// delay.
		auto it = m_queue.begin();
		while (it != m_queue.end())
		{
			if (it->dueTime <= m_clock)
			{
				if (handler)
				{
					handler(it->data.get(), it->size);
				}
				++delivered;
				it = m_queue.erase(it);
			}
			else
			{
				++it;
			}
		}

		return delivered;
	}

	std::size_t MessageQueue::pending() const noexcept
	{
		return m_queue.size();
	}

	void MessageQueue::clear() noexcept
	{
		m_queue.clear();
	}

} // namespace Modern::Tools::Emulator
