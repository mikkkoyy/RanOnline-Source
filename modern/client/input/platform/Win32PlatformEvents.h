#pragma once

// CLIENT-RENDER-001: Win32 platform events implementation.
//
// Concrete implementation of IPlatformEvents that pumps Windows messages
// for a native Win32 window. Keeps Win32 types out of the platform-
// independent IPlatformEvents interface.

#include "../InputEvents.h"
#include "Application.h"  // For IPlatformEvents

#include <cstdint>
#include <string>

namespace Modern::Client
{

class Win32InputSource;

class Win32PlatformEvents final : public IPlatformEvents
{
public:
    // Creates the platform events handler with window parameters.
    // The window is created during Initialize().
    explicit Win32PlatformEvents(
        const char* windowTitle = "RAN Online Modern",
        uint32_t clientWidth = 1280,
        uint32_t clientHeight = 720);

    ~Win32PlatformEvents() override;

    Win32PlatformEvents(const Win32PlatformEvents&) = delete;
    Win32PlatformEvents& operator=(const Win32PlatformEvents&) = delete;
    Win32PlatformEvents(Win32PlatformEvents&&) = delete;
    Win32PlatformEvents& operator=(Win32PlatformEvents&&) = delete;

    // IPlatformEvents implementation
    bool PumpEvents() override;

    // Window handle access for renderer initialization.
    void* GetWindowHandle() const noexcept { return m_hwnd; }

    // Window dimensions.
    uint32_t GetClientWidth() const noexcept { return m_clientWidth; }
    uint32_t GetClientHeight() const noexcept { return m_clientHeight; }

    // Initializes the window class and creates the window.
    // Must be called before PumpEvents().
    bool Initialize();

    // Cleans up window resources.
    void Shutdown();

    // Sets the input source to feed events to.
    void SetInputSource(Win32InputSource* inputSource) noexcept { m_inputSource = inputSource; }

private:
    // Window class registration.
    bool RegisterWindowClass();
    void UnregisterWindowClass();

    // Window creation.
    bool CreateWindowInstance();

    // Message handling.
    static LRESULT CALLBACK WndProcStatic(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // Window parameters.
    std::string m_windowTitle;
    uint32_t m_clientWidth;
    uint32_t m_clientHeight;

    // Window state.
    HWND m_hwnd = nullptr;
    HINSTANCE m_hinstance = nullptr;
    bool m_classRegistered = false;
    bool m_shouldClose = false;
    bool m_initialized = false;

    // Resize tracking.
    bool m_resizePending = false;
    uint32_t m_pendingWidth = 0;
    uint32_t m_pendingHeight = 0;

    // Input source for feeding events.
    Win32InputSource* m_inputSource = nullptr;
};

} // namespace Modern::Client