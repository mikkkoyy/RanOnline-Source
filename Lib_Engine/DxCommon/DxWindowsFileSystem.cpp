// DxWindowsFileSystem.cpp
//
// Windows implementation of the portable GameFileSystem::IFileSystem.
// Uses PathFileExists (Shlwapi) internally, matching the legacy behavior
// in RANPARAM_OPTION.cpp exactly.

#include "stdafx.h"
#include "DxWindowsFileSystem.h"

#include <shlwapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DxWindowsFileSystem::DxWindowsFileSystem()
{
}

DxWindowsFileSystem::~DxWindowsFileSystem()
{
}

bool DxWindowsFileSystem::Exists(const std::string& path) const
{
    // PathFileExists returns TRUE for both files and directories,
    // matching the legacy behavior in RANPARAM_OPTION.cpp.
    return PathFileExistsA(path.c_str()) == TRUE;
}
