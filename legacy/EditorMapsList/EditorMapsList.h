
// EditorMapsList.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CEditorMapsListApp:
// See EditorMapsList.cpp for the implementation of this class
//

class CEditorMapsListApp : public CWinApp
{
public:
	CEditorMapsListApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CEditorMapsListApp theApp;