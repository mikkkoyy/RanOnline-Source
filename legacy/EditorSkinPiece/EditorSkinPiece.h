
// EditorSkinPiece.h : main header file for the EditorSkinPiece application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CEditorSkinPieceApp:
// See EditorSkinPiece.cpp for the implementation of this class
//

class CEditorSkinPieceApp : public CWinApp
{
public:
	CEditorSkinPieceApp();

public:
	BOOL		m_bIsActive;
	CString		m_strAppPath;
	char		m_szAppPath[MAX_PATH];

public:
	void		SetActive(BOOL bActive) { m_bIsActive = bActive; };

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int Run();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CEditorSkinPieceApp theApp;
