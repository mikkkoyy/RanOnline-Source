
// GameClient2.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

#define NO_GAMEGUARD

#define MAX_DUMP_OFFSETS 477
#define MAX_DUMP_SIZE 32
#define MAX_PROCESS_DUMP 477

typedef struct ANTIHACK_PROCDUMP
{
	unsigned int m_aOffset;
	unsigned char m_aMemDump[MAX_DUMP_SIZE];
	char m_progName[100];

} *PANTIHACK_PROCDUMP;

extern ANTIHACK_PROCDUMP g_ProcessesDumps[MAX_PROCESS_DUMP];

void SystemProcessesScan();
bool ScanProcessMemory(HANDLE hProcess);

class CGameClient2Wnd;

// CGameClient2App:
// See GameClient2.cpp for the implementation of this class
//

class CGameClient2App : public CWinApp
{
public:
	CGameClient2App();

protected:
	CGameClient2Wnd*	m_pWnd;
	BOOL		m_bIsActive;    
	BOOL		m_bRedrawScreen;
	BOOL		m_bShutDown;

public:
	CString m_strAppPath;
	TCHAR	m_szAppPath[MAX_PATH];

public:
	void SetActive(BOOL bActive);
	void ShutDown() { m_bShutDown = TRUE; };
	void SetAppPath();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual int Run();

// Implementation

	DECLARE_MESSAGE_MAP()



public:
	/*hackshield implementation, Juver, 2018/06/18 */
	static const char* hs_dll_name;
	static const char* hs_folder;
	static int hs_gamecode;
	static const char* hs_key;
	static DWORD hs_option;
	static DWORD hs_sh_option;

	static void hs_output( long code, long param_size, void* pparam );
	BOOL hs_start();
	BOOL hs_stop();
	BOOL hs_start_service();
	BOOL hs_stop_service();
};

extern CGameClient2App theApp;