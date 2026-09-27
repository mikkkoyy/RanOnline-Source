// DxWindowsFileSystem.h
//
// Windows implementation of the portable GameFileSystem::IFileSystem
// interface. This is the ONLY file that may reference Windows/MFC
// filesystem APIs for the portable filesystem seam.

#pragma once

#include "../Common/GameFileSystem.h"

class DxWindowsFileSystem : public GameFileSystem::IFileSystem
{
public:
    DxWindowsFileSystem();
    virtual ~DxWindowsFileSystem();

    // Inherited from GameFileSystem::IFileSystem
    virtual bool Exists(const std::string& path) const override;
};
