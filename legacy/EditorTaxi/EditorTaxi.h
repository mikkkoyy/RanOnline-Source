
// EditorTaxi.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CEditorTaxiApp:
// See EditorTaxi.cpp for the implementation of this class
//

class CEditorTaxiApp : public CWinApp
{
public:
	CEditorTaxiApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CEditorTaxiApp theApp;