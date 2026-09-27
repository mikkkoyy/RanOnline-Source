// DxWindowsTime.h
//
// Windows implementation of the portable GameTime::ITimeProvider
// interface. This is the ONLY file that may reference Win32 timing
// APIs for the portable time seam.

#pragma once

#include "../Common/GameTime.h"

class DxWindowsTime : public GameTime::ITimeProvider
{
public:
    DxWindowsTime();
    ~DxWindowsTime();

    // Inherited from GameTime::ITimeProvider
    virtual GameUInt32 GetMonotonicMilliseconds() const override;
};
