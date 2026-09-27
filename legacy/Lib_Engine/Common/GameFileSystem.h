// GameFileSystem.h
//
// Portable game-filesystem contract for the RAN Online modernization.
//
// This header is intentionally platform-neutral. It must be includable
// from Windows, Android, and iOS projects without pulling in Windows.h,
// MFC, CFile, or any DirectX headers.
//
// The interface represents what the game needs to know about the local
// filesystem, not how the underlying OS provides it. Platform-specific
// implementations live in the platform layer (e.g.
// DxCommon/DxWindowsFileSystem.cpp) and are the ONLY place that may
// reference CreateFile, FindFirstFile, GetFileAttributes, PathFileExists,
// etc.
//
// Paths passed through this interface are UTF-8 encoded. The platform
// implementations are responsible for any necessary encoding conversion.

#pragma once

#include <string>

namespace GameFileSystem
{
    // ---------------------------------------------------------------------------
    // Portable filesystem contract.
    //
    // Implementations own ALL platform-specific details. Consumers call only
    // portable methods with portable types.
    // ---------------------------------------------------------------------------
    class IFileSystem
    {
    public:
        virtual ~IFileSystem() = default;

        // Returns true if a regular file or directory exists at the given path.
        // The behavior matches the legacy PathFileExists call: TRUE for both
        // files and directories, FALSE for nonexistent paths.
        virtual bool Exists(const std::string& path) const = 0;
    };
}
