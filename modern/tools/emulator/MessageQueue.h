#pragma once

// TOOLS-002: headless message-processing boundary.
//
// A delayed message queue with deterministic, elapsed-time-driven delivery.
// This is a modern equivalent of the legacy emulator's emulation message
// buffer (CEMULMSG / g_vecMsg in legacy/Lib_Client/DxGlobalStage.cpp),
// extracted so it can be unit-tested without MFC, D3D9, GLGaeaServer, or any
// compiled legacy library.
//
// The queue owns each message until it is delivered or the queue is destroyed.
// Delivery is driven by elapsed seconds supplied to update() -- no real-time
// clock, no sleeps, no threads. A message whose delay has not yet expired is
// held; once the delay reaches zero it is delivered to the handler exactly
// once. Messages with shorter delays deliver before messages with longer
// delays, regardless of enqueue order.
//
// No sockets, no authentication, no field-server routing, no broadcast
// networking. Just queue, delay, dispatch, release.

#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>

namespace Modern::Tools::Emulator
{

// A bounded, delayed message queue.
//
// Messages are enqueued as byte blobs with a delivery delay. Each call to
// update(elapsedSeconds, handler) advances the simulated clock; messages whose
// delay has reached zero are handed to `handler` exactly once each. The queue
// owns the message memory until delivery.
//
// Unlike the legacy buffer (which used a fixed 0.1s delay for every message
// and only checked the front of the deque), this queue supports per-message
// delays: a message enqueued later with a shorter delay delivers before an
// earlier message with a longer delay.
//
// This component is renderer-independent, platform-independent, and
// legacy-independent: it includes no Windows.h, no d3d11.h, no legacy
// headers, and no MFC types. It links only the C++ standard library.
class MessageQueue
{
public:
    // Called when a message is delivered. `data` points to the message
    // payload; `size` is its byte count. The pointer is valid only for the
    // duration of the call -- the queue releases ownership immediately after.
    using Handler = std::function<void(const std::uint8_t* data,
                                       std::size_t size)>;

    MessageQueue() = default;
    ~MessageQueue();

    // Non-copyable: a queue owns its pending messages and cannot be safely
    // duplicated while messages are in flight.
    MessageQueue(const MessageQueue&) = delete;
    MessageQueue& operator=(const MessageQueue&) = delete;

    // Movable: ownership transfers, pending messages move with it.
    MessageQueue(MessageQueue&& other) noexcept;
    MessageQueue& operator=(MessageQueue&& other) noexcept;

    // Enqueues a deep copy of `data` (size bytes) for delivery after
    // `delaySeconds` of simulated time. Returns false when `data` is null or
    // `size` is zero; nothing is enqueued in that case.
    bool enqueue(const std::uint8_t* data,
                 std::size_t     size,
                 float           delaySeconds) noexcept;

    // Advances the queue by `elapsedSeconds`. Delivers any messages whose
    // delay has reached zero, calling `handler` for each. Returns the number
    // of messages delivered in this call.
    //
    // Messages deliver in due-time order, not strictly enqueue order: a later
    // message with a shorter delay delivers before an earlier message with a
    // longer delay.
    std::size_t update(float         elapsedSeconds,
                       Handler       handler) noexcept;

    // Number of messages waiting to be delivered.
    std::size_t pending() const noexcept;

    // Removes and destroys every pending message without delivering it.
    // After this call, pending() returns 0 and no handler invocations occur
    // for the cleared messages.
    void clear() noexcept;

private:
    struct PendingMessage
    {
        float              dueTime;        // simulated time at which due
        std::size_t        size;
        std::unique_ptr<std::uint8_t[]> data;
    };

    // Simulated clock, advanced only by update(). Messages store their due
    // time as an absolute value on this clock.
    float m_clock = 0.0f;

    std::deque<PendingMessage> m_queue;
};

} // namespace Modern::Tools::Emulator
