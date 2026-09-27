
// GameEmulator.h : main header file for the GameEmulator application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CGameEmulatorApp:
// See GameEmulator.cpp for the implementation of this class
//

class CGameEmulatorApp : public CWinApp
{
public:
	void  SetActive(BOOL bActive) { m_bIsActive = bActive; };

public:
	BOOL  m_bIsActive;

	CString	m_strAppPath;
	char	m_szAppPath[MAX_PATH];

public:
	CGameEmulatorApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	virtual int Run();
};

extern CGameEmulatorApp theApp;
