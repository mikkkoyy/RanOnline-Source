#pragma once

// CLIENT-RENDER-001: Win32 input source implementation.
//
// Concrete implementation of IInputSource that translates Win32
// messages into platform-independent InputEvents. Works in tandem
// with Win32PlatformEvents.

#include "../InputEvents.h"
#include "application/Application.h"  // For IInputSource
#include "Win32InputAdapter.h"

#include <vector>

namespace Modern::Client
{

class Win32PlatformEvents;

class Win32InputSource final : public IInputSource
{
public:
    // Constructs with a reference to the platform events handler
    // for accessing the window handle and dimensions.
    explicit Win32InputSource(const Win32PlatformEvents* platformEvents);

    ~Win32InputSource() override = default;

    Win32InputSource(const Win32InputSource&) = delete;
    Win32InputSource& operator=(const Win32InputSource&) = delete;
    Win32InputSource(Win32InputSource&&) = default;
    Win32InputSource& operator=(Win32InputSource&&) = default;

    // IInputSource implementation
    std::vector<InputEvent> PollEvents() override;
    bool RequestsClose() const override;

    // Called by Win32PlatformEvents to feed raw input samples.
    void FeedKeySample(uint32_t virtualKey, bool pressed, bool repeated);
    void FeedMouseSample(int32_t x, int32_t y, int32_t wheelDelta,
                         bool leftPressed, bool leftChanged,
                         bool rightPressed, bool rightChanged,
                         bool middlePressed, bool middleChanged,
                         bool moved);
    void FeedWindowResized(uint32_t width, uint32_t height);
    void FeedWindowClosed();

private:
    const Win32PlatformEvents* m_platformEvents = nullptr;
    std::vector<InputEvent> m_pendingEvents;

    // Mouse state tracking for edge detection.
    bool m_leftPressed = false;
    bool m_rightPressed = false;
    bool m_middlePressed = false;
    int32_t m_lastX = 0;
    int32_t m_lastY = 0;
    bool m_firstMouse = true;

    // Window close flag.
    bool m_windowClosed = false;
};

} // namespace Modern::Client