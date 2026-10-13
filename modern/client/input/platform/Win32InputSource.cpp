// CLIENT-RENDER-001: Win32 input source implementation.

#include "Win32InputSource.h"

#include <algorithm>

namespace Modern::Client
{

Win32InputSource::Win32InputSource(const Win32PlatformEvents* platformEvents)
    : m_platformEvents(platformEvents)
{
}

std::vector<InputEvent> Win32InputSource::PollEvents()
{
    std::vector<InputEvent> events = std::move(m_pendingEvents);
    m_pendingEvents.clear();
    return events;
}

bool Win32InputSource::RequestsClose() const
{
    return m_windowClosed;
}

void Win32InputSource::FeedKeySample(uint32_t virtualKey, bool pressed, bool repeated)
{
    Win32KeySample sample;
    sample.virtualKey = virtualKey;
    sample.pressed = pressed;
    sample.repeated = repeated;
    Win32InputAdapter::PushKeySample(m_pendingEvents, sample);
}

void Win32InputSource::FeedMouseSample(
    int32_t x, int32_t y, int32_t wheelDelta,
    bool leftPressed, bool leftChanged,
    bool rightPressed, bool rightChanged,
    bool middlePressed, bool middleChanged,
    bool moved)
{
    // Track mouse movement
    bool actuallyMoved = moved || (m_firstMouse) || (x != m_lastX) || (y != m_lastY);
    
    Win32MouseSample sample;
    sample.x = x;
    sample.y = y;
    sample.wheelDelta = wheelDelta;
    sample.leftPressed = leftPressed;
    sample.leftChanged = leftChanged;
    sample.rightPressed = rightPressed;
    sample.rightChanged = rightChanged;
    sample.middlePressed = middlePressed;
    sample.middleChanged = middleChanged;
    sample.moved = actuallyMoved;

    Win32InputAdapter::PushMouseSample(m_pendingEvents, sample);

    m_lastX = x;
    m_lastY = y;
    m_firstMouse = false;
}

void Win32InputSource::FeedWindowResized(uint32_t width, uint32_t height)
{
    m_pendingEvents.push_back(InputEvent::MakeWindowResized(width, height));
}

void Win32InputSource::FeedWindowClosed()
{
    m_windowClosed = true;
    m_pendingEvents.push_back(InputEvent::MakeWindowClosed());
}

} // namespace Modern::Client