#pragma once

// Minimal MFC/legacy stubs for headless item exporter
// Provides only the types/functions actually used by GLItemMan/GLItem/SITEM

#ifndef _MSC_VER
#error "This file requires MSVC"
#endif

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <memory>
#include <stdexcept>
#include <cstdarg>
#include <cassert>

// ============================================================
// Basic type definitions (normally from MFC/Windows headers)
// ============================================================

typedef int8_t INT8;
typedef uint8_t UINT8;
typedef int16_t INT16;
typedef uint16_t UINT16;
typedef int32_t INT32;
typedef uint32_t UINT32;
typedef int64_t INT64;
typedef uint64_t UINT64;

typedef int8_t BYTE;
typedef uint8_t UCHAR;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint64_t QWORD;
typedef int32_t LONG;
typedef uint32_t ULONG;
typedef int32_t BOOL;
typedef int32_t INT;
typedef uint32_t UINT;
typedef float FLOAT;
typedef double DOUBLE;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

// ============================================================
// CString minimal stub (MFC CString)
// ============================================================

class CString
{
public:
    CString() = default;
    CString(const char* psz) : m_str(psz ? psz : "") {}
    CString(const wchar_t* psz) { if (psz) m_str = std::string(psz, psz + wcslen(psz)); }
    CString(const CString& other) = default;
    CString(CString&& other) = default;
    CString& operator=(const CString& other) = default;
    CString& operator=(CString&& other) = default;

    CString(const std::string& s) : m_str(s) {}
    CString(std::string&& s) : m_str(std::move(s)) {}

    operator const char*() const { return m_str.c_str(); }
    operator std::string() const { return m_str; }

    const char* GetString() const { return m_str.c_str(); }
    int GetLength() const { return static_cast<int>(m_str.length()); }
    bool IsEmpty() const { return m_str.empty(); }

    void Empty() { m_str.clear(); }

    CString& operator+=(const CString& other) { m_str += other.m_str; return *this; }
    CString& operator+=(const char* psz) { if (psz) m_str += psz; return *this; }

    int Find(char ch, int nStart = 0) const {
        auto pos = m_str.find(ch, nStart);
        return pos == std::string::npos ? -1 : static_cast<int>(pos);
    }

    CString Mid(int nFirst, int nCount = -1) const {
        if (nFirst < 0 || static_cast<size_t>(nFirst) >= m_str.size()) return CString();
        if (nCount < 0) return CString(m_str.substr(nFirst));
        return CString(m_str.substr(nFirst, nCount));
    }

    CString Left(int nCount) const {
        if (nCount <= 0) return CString();
        if (nCount >= static_cast<int>(m_str.size())) return *this;
        return CString(m_str.substr(0, nCount));
    }

    CString Right(int nCount) const {
        if (nCount <= 0) return CString();
        if (nCount >= static_cast<int>(m_str.size())) return *this;
        return CString(m_str.substr(m_str.size() - nCount));
    }

    void MakeUpper() { for (auto& c : m_str) c = static_cast<char>(toupper(static_cast<unsigned char>(c))); }
    void MakeLower() { for (auto& c : m_str) c = static_cast<char>(tolower(static_cast<unsigned char>(c))); }

    int Compare(const CString& other) const { return m_str.compare(other.m_str); }
    int CompareNoCase(const CString& other) const {
        // Simple case-insensitive compare
        for (size_t i = 0; i < m_str.size() && i < other.m_str.size(); ++i) {
            char a = static_cast<char>(tolower(static_cast<unsigned char>(m_str[i])));
            char b = static_cast<char>(tolower(static_cast<unsigned char>(other.m_str[i])));
            if (a != b) return a < b ? -1 : 1;
        }
        if (m_str.size() < other.m_str.size()) return -1;
        if (m_str.size() > other.m_str.size()) return 1;
        return 0;
    }

    void Format(const char* pszFormat, ...) {
        va_list args;
        va_start(args, pszFormat);
        char buffer[4096];
        vsnprintf(buffer, sizeof(buffer), pszFormat, args);
        va_end(args);
        m_str = buffer;
    }

    friend CString operator+(const CString& a, const CString& b) {
        return CString(a.m_str + b.m_str);
    }
    friend CString operator+(const CString& a, const char* b) {
        return CString(a.m_str + (b ? b : ""));
    }
    friend CString operator+(const char* a, const CString& b) {
        return CString((a ? a : "") + b.m_str);
    }

    CString& operator=(const char* psz) { m_str = psz ? psz : ""; return *this; }

private:
    std::string m_str;
};

// CStringA alias
typedef CString CStringA;
typedef CString CStringW;

// ============================================================
// CDebugSet minimal stub
// ============================================================

class CDebugSet
{
public:
    static void ToLogFile(const char* pszFormat, ...) {
        va_list args;
        va_start(args, pszFormat);
        vfprintf(stderr, pszFormat, args);
        fputc('\n', stderr);
        va_end(args);
    }
};

// ============================================================
// GASSERT / VERIFY / VERIFY
// ============================================================

#define GASSERT(expr) assert(expr)
#define VERIFY(expr) assert(expr)
#define VERIFY_RET(expr, ret) do { if(!(expr)) return ret; } while(0)

// ============================================================
// SAFE_DELETE / SAFE_DELETE_ARRAY
// ============================================================

#define SAFE_DELETE(p) do { if(p) { delete p; p = NULL; } } while(0)
#define SAFE_DELETE_ARRAY(p) do { if(p) { delete[] p; p = NULL; } } while(0)

// ============================================================
// CRijndael minimal stub (used by CBaseString)
// ============================================================

class CRijndael
{
public:
    CRijndael() = default;
    ~CRijndael() = default;
    void Initialize(const char*, const char*) {}
    void Encrypt(const unsigned char*, unsigned char*) const {}
    void Decrypt(const unsigned char*, unsigned char*) const {}
};

// ============================================================
// CFile minimal stub
// ============================================================

class CFile
{
public:
    enum OpenFlags {
        modeRead = 0x0001,
        modeWrite = 0x0002,
        modeReadWrite = 0x0003,
        modeCreate = 0x0010,
        modeNoTruncate = 0x0020,
        typeText = 0x0040,
        typeBinary = 0x0080,
    };

    CFile() : m_fp(nullptr) {}
    ~CFile() { Close(); }

    BOOL Open(const char* pszFileName, UINT nOpenFlags) {
        const char* mode = "rb";
        if (nOpenFlags & modeWrite) mode = "wb";
        else if (nOpenFlags & modeReadWrite) mode = "wb+";
        m_fp = fopen(m_strFileName.c_str(), mode);
        return m_fp != NULL;
    }

    void Close() {
        if (m_fp) { fclose(m_fp); m_fp = nullptr; }
    }

    UINT Read(void* lpBuf, UINT nCount) {
        return m_fp ? static_cast<UINT>(fread(lpBuf, 1, nCount, m_fp)) : 0;
    }

    void Write(const void* lpBuf, UINT nCount) {
        if (m_fp) fwrite(lpBuf, 1, nCount, m_fp);
    }

    void Seek(LONG lOff, UINT nFrom) {
        if (m_fp) fseek(m_fp, lOff, nFrom);
    }

    ULONG GetLength() const {
        if (!m_fp) return 0;
        long cur = ftell(m_fp);
        fseek(m_fp, 0, SEEK_END);
        long len = ftell(m_fp);
        fseek(m_fp, cur, SEEK_SET);
        return static_cast<ULONG>(len);
    }

private:
    FILE* m_fp = nullptr;
    CString m_strFileName;
};

// ============================================================
// CArchive minimal stub
// ============================================================

class CArchive
{
public:
    enum { load, store };

    CArchive(CFile* pFile, int nMode, int nBufSize = 4096, void* lpBuf = NULL)
        : m_pFile(pFile), m_bStoring(nMode == store) {}

    CArchive& operator<<(const CString& str) { return *this; }
    CArchive& operator>>(CString& str) { return *this; }
    template<typename T> CArchive& operator<<(const T&) { return *this; }
    template<typename T> CArchive& operator>>(T&) { return *this; }

    CArchive& operator<<(const char* psz) { return *this; }
    CArchive& operator>>(char* psz) { return *this; }

    void Flush() {}

private:
    CFile* m_pFile;
    bool m_bStoring;
};

// ============================================================
// CTime / CTimeSpan minimal stub
// ============================================================

class CTime
{
public:
    CTime() = default;
    CTime(time_t t) : m_time(t) {}
    static CTime GetCurrentTime() { return CTime(time(nullptr)); }
    CString Format(const char* pszFormat) const {
        char buffer[256];
        struct tm tm;
        localtime_s(&tm, &m_time);
        strftime(buffer, sizeof(buffer), pszFormat, &tm);
        return CString(buffer);
    }
    time_t GetTime() const { return m_time; }
private:
    time_t m_time = 0;
};

class CTimeSpan
{
public:
    CTimeSpan() = default;
    CTimeSpan(time_t t) : m_time(t) {}
    long GetTotalSeconds() const { return static_cast<long>(m_time); }
private:
    time_t m_time = 0;
};

// ============================================================
// Common macros
// ============================================================

#define _T(x) x
#define _countof(arr) (sizeof(arr) / sizeof(arr[0]))
#define StringCchCopy(dest, size, src) (strncpy_s(dest, size, src, _TRUNCATE), 0)
#define StringCchCat(dest, size, src) (strncat_s(dest, size, src, _TRUNCATE), 0)

// ============================================================
// LPDIRECT3DDEVICEQ / DirectX stubs
// ============================================================

typedef void* LPDIRECT3DDEVICEQ;
typedef void* LPDIRECT3DDEVICE9;
typedef void* LPDIRECT3D9;

#define D3DFVF_XYZ 0x002
#define D3DFVF_NORMAL 0x010
#define D3DFVF_TEX1 0x100

// ============================================================
// GASSERT replacement
// ============================================================

#ifdef GASSERT
#undef GASSERT
#endif
#define GASSERT(expr) assert(expr)

#ifdef VERIFY
#undef VERIFY
#endif
#define VERIFY(expr) assert(expr)

#define SecureZeroMemory(ptr, size) memset(ptr, 0, size)

// ============================================================
// Include standard headers after stubs
// ============================================================

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cmath>
#include <ctime>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>

// ============================================================
// Forward declare needed legacy types
// ============================================================

class basestream;
class CBaseString;
class CStringFile;
class CStringMemory;
class CSerialFile;
class CSerialMemory;
class SFileSystem;
class CUnzipper;

namespace GLOGIC {
    extern BOOL bGLOGIC_ZIPFILE;
    extern BOOL bGLOGIC_PACKFILE;
    extern std::string strGLOGIC_ZIPFILE;
    void SetPath(const char* szPath);
    const char* GetPath();
    void OpenPackFile(const char* szAppPath);
    basestream* openfile_basestream(BOOL bZIP, const char* szZipFile, const char* szFullPath, const char* szFile, EMBYTECRYPT emCRYPT = EMBYTECRYPT_NONE, BOOL bPack = FALSE);
    CBaseString* openfile_basestring(BOOL bZIP, const char* szZipFile, const char* szFullPath, const char* szFile, bool bENC = false, BOOL bPack = FALSE);
    void SetEngFullPath();
    void OpenPackFile(const char* szAppPath);
    void SetServerPath(const char* szPath);
    const char* GetServerPath();
}

enum EMBYTECRYPT {
    EMBYTECRYPT_NONE = 0,
    EMBYTECRYPT_OLD = 1,
    EMBYTECRYPT_ITEM = 2,
    EMBYTECRYPT_SIZE
};

#endif // _LEGACY_STUBS_H_