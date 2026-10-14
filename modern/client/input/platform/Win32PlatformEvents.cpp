// CLIENT-RENDER-001: Win32 platform events implementation.

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#define _UNICODE
#include <Windows.h>
#include <windowsx.h>

#include "Win32PlatformEvents.h"
#include "Win32InputSource.h"

#include <cassert>

namespace Modern::Client
{

namespace
{
// Window class name - unique enough to avoid collisions.
constexpr const wchar_t* kWindowClassName = L"RANOnlineModernWindowClass";
}

Win32PlatformEvents::Win32PlatformEvents(
    const char* windowTitle,
    uint32_t clientWidth,
    uint32_t clientHeight)
    : m_windowTitle(windowTitle ? windowTitle : "RAN Online Modern")
    , m_clientWidth(clientWidth)
    , m_clientHeight(clientHeight)
    , m_inputSource(nullptr)
{
}

Win32PlatformEvents::~Win32PlatformEvents()
{
    Shutdown();
}

bool Win32PlatformEvents::Initialize()
{
    if (m_initialized)
    {
        return true;
    }

    m_hinstance = GetModuleHandleW(nullptr);
    if (!m_hinstance)
    {
        return false;
    }

    if (!RegisterWindowClass())
    {
        return false;
    }

    if (!CreateWindowInstance())
    {
        UnregisterWindowClass();
        return false;
    }

    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);

    m_initialized = true;
    return true;
}

void Win32PlatformEvents::Shutdown()
{
    if (m_hwnd)
    {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }

    UnregisterWindowClass();
    m_initialized = false;
}

bool Win32PlatformEvents::RegisterWindowClass()
{
    if (m_classRegistered)
    {
        return true;
    }

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = &Win32PlatformEvents::WndProcStatic;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_hinstance;
    wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr; // We paint everything ourselves
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = kWindowClassName;
    wc.hIconSm = LoadIconW(nullptr, IDI_APPLICATION);

    if (!RegisterClassExW(&wc))
    {
        return false;
    }

    m_classRegistered = true;
    return true;
}

void Win32PlatformEvents::UnregisterWindowClass()
{
    if (m_classRegistered && m_hinstance)
    {
        UnregisterClassW(kWindowClassName, m_hinstance);
        m_classRegistered = false;
    }
}

bool Win32PlatformEvents::CreateWindowInstance()
{
    // Convert window title to wide string. The size query with cbMultiByte = -1
    // returns the required size INCLUDING the terminating null, so the buffer
    // must be titleLen wide characters: passing titleLen - 1 as the capacity
    // while telling the API the buffer holds titleLen writes the null one
    // wchar_t past the end.
    int titleLen = MultiByteToWideChar(CP_UTF8, 0, m_windowTitle.c_str(), -1, nullptr, 0);
    if (titleLen <= 0)
    {
        return false;
    }
    std::wstring wideTitle(titleLen, L'\0');
    const int converted = MultiByteToWideChar(
        CP_UTF8, 0, m_windowTitle.c_str(), -1, wideTitle.data(), titleLen);
    if (converted != titleLen)
    {
        return false;
    }

    // Calculate window rect for desired client area.
    RECT windowRect = { 0, 0, static_cast<LONG>(m_clientWidth), static_cast<LONG>(m_clientHeight) };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;

    m_hwnd = CreateWindowExW(
        0,
        kWindowClassName,
        wideTitle.c_str(),
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        CW_USEDEFAULT, CW_USEDEFAULT,
        windowWidth, windowHeight,
        nullptr,
        nullptr,
        m_hinstance,
        this); // Pass 'this' as lpCreateParams

    if (!m_hwnd)
    {
        return false;
    }

    // Store 'this' pointer in window user data for static WndProc.
    SetWindowLongPtrW(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    return true;
}

LRESULT CALLBACK Win32PlatformEvents::WndProcStatic(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_NCCREATE)
    {
        CREATESTRUCTW* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
        Win32PlatformEvents* self = static_cast<Win32PlatformEvents*>(createStruct->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    Win32PlatformEvents* self = reinterpret_cast<Win32PlatformEvents*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (self)
    {
        return self->WndProc(hwnd, msg, wParam, lParam);
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT Win32PlatformEvents::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
        m_shouldClose = true;
        if (m_inputSource)
        {
            m_inputSource->FeedWindowClosed();
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED)
        {
            m_pendingWidth = static_cast<uint32_t>(LOWORD(lParam));
            m_pendingHeight = static_cast<uint32_t>(HIWORD(lParam));
            m_resizePending = true;
            m_clientWidth = m_pendingWidth;
            m_clientHeight = m_pendingHeight;
            if (m_inputSource)
            {
                m_inputSource->FeedWindowResized(m_pendingWidth, m_pendingHeight);
            }
        }
        return 0;

    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* mmi = reinterpret_cast<MINMAXINFO*>(lParam);
        mmi->ptMinTrackSize.x = 320;
        mmi->ptMinTrackSize.y = 240;
        return 0;
    }

    case WM_ACTIVATE:
        // No activation handling yet: give Windows its default processing
        // (focus, caption state) rather than claiming the message.
        return DefWindowProcW(hwnd, msg, wParam, lParam);

    // Keyboard messages
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (m_inputSource)
        {
            bool repeated = (lParam & (1 << 30)) != 0;
            m_inputSource->FeedKeySample(static_cast<uint32_t>(wParam), true, repeated);
        }
        return 0;

    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (m_inputSource)
        {
            m_inputSource->FeedKeySample(static_cast<uint32_t>(wParam), false, false);
        }
        return 0;

    // Mouse messages
    case WM_MOUSEMOVE:
    {
        if (m_inputSource)
        {
            int32_t x = static_cast<int32_t>(GET_X_LPARAM(lParam));
            int32_t y = static_cast<int32_t>(GET_Y_LPARAM(lParam));
            m_inputSource->FeedMouseSample(x, y, 0,
                (wParam & MK_LBUTTON) != 0, false,
                (wParam & MK_RBUTTON) != 0, false,
                (wParam & MK_MBUTTON) != 0, false,
                true);
        }
        return 0;
    }

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    {
        if (m_inputSource)
        {
            int32_t x = static_cast<int32_t>(GET_X_LPARAM(lParam));
            int32_t y = static_cast<int32_t>(GET_Y_LPARAM(lParam));
            
            bool leftPressed = (wParam & MK_LBUTTON) != 0;
            bool rightPressed = (wParam & MK_RBUTTON) != 0;
            bool middlePressed = (wParam & MK_MBUTTON) != 0;
            
            bool leftChanged = (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP);
            bool rightChanged = (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP);
            bool middleChanged = (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONUP);
            
            m_inputSource->FeedMouseSample(x, y, 0,
                leftPressed, leftChanged,
                rightPressed, rightChanged,
                middlePressed, middleChanged,
                false);
        }
        return 0;
    }

    case WM_MOUSEWHEEL:
    {
        if (m_inputSource)
        {
            int32_t x = static_cast<int32_t>(GET_X_LPARAM(lParam));
            int32_t y = static_cast<int32_t>(GET_Y_LPARAM(lParam));
            // Convert screen coordinates to client coordinates
            POINT pt = { x, y };
            ScreenToClient(hwnd, &pt);
            int32_t wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            m_inputSource->FeedMouseSample(pt.x, pt.y, wheelDelta,
                (wParam & MK_LBUTTON) != 0, false,
                (wParam & MK_RBUTTON) != 0, false,
                (wParam & MK_MBUTTON) != 0, false,
                false);
        }
        return 0;
    }

    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}

bool Win32PlatformEvents::PumpEvents()
{
    if (!m_initialized)
    {
        return false;
    }

    MSG msg = {};
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            m_shouldClose = true;
            return false;
        }

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    // Clear resize pending flag after processing
    if (m_resizePending)
    {
        m_resizePending = false;
    }

    return !m_shouldClose;
}

} // namespace Modern::Client