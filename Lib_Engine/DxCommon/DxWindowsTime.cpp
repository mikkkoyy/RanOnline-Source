// DxWindowsTime.cpp
//
// Windows implementation of the portable GameTime::ITimeProvider.
// Uses timeGetTime (Win32 multimedia timer) internally, matching the
// legacy behavior in profile.cpp exactly.

#include "stdafx.h"
#include "DxWindowsTime.h"

#include <mmsystem.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DxWindowsTime::DxWindowsTime()
{
}

DxWindowsTime::~DxWindowsTime()
{
}

GameUInt32 DxWindowsTime::GetMonotonicMilliseconds() const
{
    return static_cast<GameUInt32>(timeGetTime());
}
