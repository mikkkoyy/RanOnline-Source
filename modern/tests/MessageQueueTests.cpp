// TOOLS-002: headless message queue tests.
//
// These tests exercise the modern message-processing boundary
// (modern/tools/emulator/MessageQueue.h / .cpp) -- a modern equivalent of the
// legacy emulator's CEMULMSG / g_vecMsg buffer. They prove queue, delay,
// dispatch, ordering, and ownership behavior without MFC, D3D9, GLGaeaServer,
// or any compiled legacy library.
//
// A passing test proves the queue boundary works. It does not prove that
// legacy DxGlobalStage::_update_emul_msg() or GLGaeaServer::MsgProcess()
// works -- those remain behind the MFC/D3D9/legacy wall.

#include "TestHarness.h"

#include "MessageQueue.h"

#include <cstdint>
#include <cstring>
#include <vector>

using namespace Modern::Tools::Emulator;

namespace
{

// TOOLS-002: an empty queue delivers nothing and accepts updates safely.
MODERN_TEST(MessageQueue_EmptyQueueIsSafe)
{
	MessageQueue queue;
	CHECK_EQ(queue.pending(), size_t{0});

	std::size_t delivered =
	    queue.update(1.0f, [](const std::uint8_t*, std::size_t) {});
	CHECK_EQ(delivered, size_t{0});

	queue.clear();
	CHECK_EQ(queue.pending(), size_t{0});
}

// TOOLS-002: a message enqueued with a delay is not delivered before that
// delay expires.
MODERN_TEST(MessageQueue_NotDeliveredBeforeDelay)
{
	MessageQueue queue;
	const std::uint8_t payload[] = {0x01, 0x02, 0x03};

	CHECK(queue.enqueue(payload, sizeof(payload), 0.5f));
	CHECK_EQ(queue.pending(), size_t{1});

	bool delivered = false;
	queue.update(0.3f, [&](const std::uint8_t* data, std::size_t size) {
		delivered = true;
		CHECK_EQ(size, sizeof(payload));
		CHECK(std::memcmp(data, payload, sizeof(payload)) == 0);
	});
	CHECK(!delivered);
	CHECK_EQ(queue.pending(), size_t{1});
}

// TOOLS-002: a message is delivered when its delay reaches zero.
MODERN_TEST(MessageQueue_DeliveredAfterDelay)
{
	MessageQueue queue;
	const std::uint8_t payload[] = {0x0A, 0x0B, 0x0C, 0x0D};

	CHECK(queue.enqueue(payload, sizeof(payload), 0.5f));

	// Advance to exactly the delay boundary: now due.
	bool delivered = false;
	queue.update(0.5f, [&](const std::uint8_t* data, std::size_t size) {
		delivered = true;
		CHECK_EQ(size, sizeof(payload));
		CHECK(std::memcmp(data, payload, sizeof(payload)) == 0);
	});
	CHECK(delivered);
	CHECK_EQ(queue.pending(), size_t{0});
}

// TOOLS-002: multiple messages with the same delay deliver in FIFO order.
MODERN_TEST(MessageQueue_FifoOrdering)
{
	MessageQueue queue;
	std::vector<std::uint8_t> order;

	std::uint8_t a = 1, b = 2, c = 3;
	CHECK(queue.enqueue(&a, 1, 0.1f));
	CHECK(queue.enqueue(&b, 1, 0.1f));
	CHECK(queue.enqueue(&c, 1, 0.1f));
	CHECK_EQ(queue.pending(), size_t{3});

	queue.update(0.1f, [&](const std::uint8_t* data, std::size_t) {
		order.push_back(*data);
	});

	CHECK_EQ(order.size(), size_t{3});
	CHECK_EQ(order[0], std::uint8_t{1});
	CHECK_EQ(order[1], std::uint8_t{2});
	CHECK_EQ(order[2], std::uint8_t{3});
	CHECK_EQ(queue.pending(), size_t{0});
}

// TOOLS-002: a delivered message is never delivered again, even on later
// updates.
MODERN_TEST(MessageQueue_NoDoubleDelivery)
{
	MessageQueue queue;
	int          deliveryCount = 0;
	std::uint8_t payload       = 42;

	CHECK(queue.enqueue(&payload, 1, 0.0f));

	queue.update(0.0f, [&](const std::uint8_t*, std::size_t) {
		++deliveryCount;
	});
	CHECK_EQ(deliveryCount, 1);
	CHECK_EQ(queue.pending(), size_t{0});

	// Later updates must not deliver the same message again.
	queue.update(100.0f, [&](const std::uint8_t*, std::size_t) {
		++deliveryCount;
	});
	CHECK_EQ(deliveryCount, 1);
}

// TOOLS-002: enqueue rejects null data and zero-size payloads.
MODERN_TEST(MessageQueue_InvalidEnqueueRejected)
{
	MessageQueue queue;

	CHECK(!queue.enqueue(nullptr, 10, 0.1f));
	CHECK(!queue.enqueue(nullptr, 0, 0.1f));
	CHECK(!queue.enqueue(nullptr, 0, 0.0f));

	std::uint8_t payload = 1;
	CHECK(!queue.enqueue(&payload, 0, 0.1f));

	CHECK_EQ(queue.pending(), size_t{0});
}

// TOOLS-002: clear() destroys all pending messages without delivering them.
MODERN_TEST(MessageQueue_ClearDestroysPending)
{
	MessageQueue queue;
	std::uint8_t payload = 7;

	CHECK(queue.enqueue(&payload, 1, 10.0f));
	CHECK_EQ(queue.pending(), size_t{1});

	queue.clear();
	CHECK_EQ(queue.pending(), size_t{0});

	int delivered = 0;
	queue.update(100.0f, [&](const std::uint8_t*, std::size_t) {
		++delivered;
	});
	CHECK_EQ(delivered, 0);
}

// TOOLS-002: messages with different delays deliver at their own times;
// earlier-due messages deliver first even if enqueued later.
MODERN_TEST(MessageQueue_IndependentDelays)
{
	MessageQueue queue;
	std::vector<int> deliveryOrder;

	std::uint8_t late  = 1; // enqueued first, delay 1.0s
	std::uint8_t early = 2; // enqueued second, delay 0.1s
	CHECK(queue.enqueue(&late, 1, 1.0f));
	CHECK(queue.enqueue(&early, 1, 0.1f));
	CHECK_EQ(queue.pending(), size_t{2});

	// 0.05s: neither due (early needs 0.1s).
	queue.update(0.05f, [&](const std::uint8_t*, std::size_t) {});
	CHECK_EQ(queue.pending(), size_t{2});

	// 0.06s more (0.11s total): early is due, late is not.
	queue.update(0.06f, [&](const std::uint8_t* data, std::size_t) {
		deliveryOrder.push_back(*data);
	});
	CHECK_EQ(deliveryOrder.size(), size_t{1});
	CHECK(deliveryOrder.size() > 0);
	CHECK_EQ(deliveryOrder[0], 2);
	CHECK_EQ(queue.pending(), size_t{1});

	// 0.9s more (1.0s total): late is due.
	queue.update(0.9f, [&](const std::uint8_t* data, std::size_t) {
		deliveryOrder.push_back(*data);
	});
	CHECK_EQ(deliveryOrder.size(), size_t{2});
	CHECK(deliveryOrder.size() > 1);
	CHECK_EQ(deliveryOrder[1], 1);
	CHECK_EQ(queue.pending(), size_t{0});
}

// TOOLS-002: a null handler is accepted (no delivery calls, message still
// consumed and released).
MODERN_TEST(MessageQueue_NullHandlerIsSafe)
{
	MessageQueue queue;
	std::uint8_t payload = 99;
	CHECK(queue.enqueue(&payload, 1, 0.0f));
	CHECK_EQ(queue.pending(), size_t{1});

	std::size_t delivered = queue.update(1.0f, nullptr);
	CHECK_EQ(delivered, size_t{1});
	CHECK_EQ(queue.pending(), size_t{0});
}

// TOOLS-002: move constructor transfers pending messages.
MODERN_TEST(MessageQueue_MoveTransfersPending)
{
	MessageQueue source;
	std::uint8_t payload = 55;
	CHECK(source.enqueue(&payload, 1, 0.5f));
	CHECK_EQ(source.pending(), size_t{1});

	MessageQueue moved(std::move(source));
	CHECK_EQ(moved.pending(), size_t{1});
	CHECK_EQ(source.pending(), size_t{0});

	int delivered = 0;
	moved.update(1.0f, [&](const std::uint8_t*, std::size_t) {
		++delivered;
	});
	CHECK_EQ(delivered, 1);
	CHECK_EQ(moved.pending(), size_t{0});
}

} // namespace

int main()
{
	std::printf("Modern message queue tests (TOOLS-002)\n\n");

	const int failedCases = ModernTests::RunAll();

	if (failedCases == 0)
	{
		std::printf("\nAll %d test cases passed.\n", static_cast<int>(ModernTests::Registry().size()));
		return 0;
	}

	std::printf("\n%d test case(s) failed.\n", failedCases);
	return 1;
}
