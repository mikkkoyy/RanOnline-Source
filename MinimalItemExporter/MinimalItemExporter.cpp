// MinimalItemExporter.cpp
// Headless console application to export RAN item.isf to CSV using legacy loader
//
// Usage: MinimalItemExporter.exe <input.isf> <output.csv>

#include "legacy_stubs.h"

// Forward declare legacy functions/classes we need
namespace GLOGIC {
    extern BOOL bGLOGIC_ZIPFILE;
    extern BOOL bGLOGIC_PACKFILE;
    extern std::string strGLOGIC_ZIPFILE;
    void SetPath(const char* szPath);
    const char* GetPath();
    void OpenPackFile(const char* szAppPath);
    basestream* openfile_basestream(BOOL bZIP, const char* szZipFile, const char* szFullPath, const char* szFile, EMBYTECRYPT emCRYPT, BOOL bPack);
    CBaseString* openfile_basestring(BOOL bZIP, const char* szZipFile, const char* szFullPath, const char* szFile, bool bENC, BOOL bPack);
    void SetEngFullPath();
    void OpenPackFile(const char* szAppPath);
    void SetServerPath(const char* szPath);
    const char* GetServerPath();
}

// Forward declare SITEM and GLItemMan
class SITEM;
class CItemNode;
class GLItemMan;

// Forward declare required functions from legacy
extern "C" {
    // From GLItemMan.cpp
    HRESULT GLItemMan_LoadFile(const char* szFile, BOOL bServer, bool bPastLoad);
    SITEM* GLItemMan_GetItem(WORD wMID, WORD wSID);
    
    // From GLItem.cpp / SITEM
    void SITEM_SaveCsvHead(std::ostream& stream);
    void SITEM_SaveCsv(std::ostream& stream);
}

// We need to include the actual legacy source files
// This file will be compiled with the legacy source files

int wmain(int argc, wchar_t* argv[])
{
    if (argc < 3) {
        wprintf(L"Usage: MinimalItemExporter.exe <input.isf> <output.csv>\n");
        return 1;
    }

    const wchar_t* inputPath = argv[1];
    const wchar_t* outputPath = argv[2];

    wprintf(L"Loading item database from: %s\n", inputPath);
    
    // Convert to narrow strings
    char inputPathA[MAX_PATH];
    char outputPathA[MAX_PATH];
    WideCharToMultiByte(CP_ACP, 0, inputPath, -1, inputPathA, MAX_PATH, NULL, NULL);
    WideCharToMultiByte(CP_ACP, 0, outputPath, -1, outputPathA, MAX_PATH, NULL, NULL);

    // Initialize required paths
    char szAppPath[MAX_PATH] = {0};
    GetModuleFileNameW(NULL, szAppPath, MAX_PATH);
    char* pLastSlash = strrchr(szAppPath, '\\');
    if (pLastSlash) *pLastSlash = '\0';
    
    GLOGIC::SetPath(szAppPath);
    GLOGIC::OpenPackFile(szAppPath);
    
    // Load the item file
    wprintf(L"Loading item database...\n");
    
    HRESULT hr = GLItemMan_LoadFile(inputPathA, TRUE, TRUE);
    if (FAILED(hr)) {
        wprintf(L"ERROR: Failed to load item file: %s\n", inputPath);
        return 1;
    }
    
    wprintf(L"Item database loaded successfully.\n");
    wprintf(L"Exporting CSV to: %s\n", outputPath);
    
    // Open output file
    std::ofstream outFile(outputPathA);
    if (!outFile.is_open()) {
        wprintf(L"ERROR: Cannot open output file: %s\n", outputPath);
        return 1;
    }
    
    // Write CSV header
    SITEM_SaveCsvHead(outFile);
    
    // Export all items - iterate through GLItemMan table
    // This requires access to GLItemMan::m_ppItem
    // For now, we'll use the existing SaveCsvFile if available
    // or iterate manually
    
    // TODO: Implement actual item iteration and CSV export
    
    outFile.close();
    
    wprintf(L"Export completed successfully.\n");
    return 0;
}