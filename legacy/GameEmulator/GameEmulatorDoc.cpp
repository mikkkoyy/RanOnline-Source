
// GameEmulatorDoc.cpp : implementation of the CGameEmulatorDoc class
//

#include "stdafx.h"
#include "GameEmulator.h"
#include "GameEmulatorDoc.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CGameEmulatorDoc

IMPLEMENT_DYNCREATE(CGameEmulatorDoc, CDocument)

BEGIN_MESSAGE_MAP(CGameEmulatorDoc, CDocument)
END_MESSAGE_MAP()


// CGameEmulatorDoc construction/destruction

CGameEmulatorDoc::CGameEmulatorDoc()
{
	// TODO: add one-time construction code here

}

CGameEmulatorDoc::~CGameEmulatorDoc()
{
}

BOOL CGameEmulatorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CGameEmulatorDoc serialization

void CGameEmulatorDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CGameEmulatorDoc diagnostics

#ifdef _DEBUG
void CGameEmulatorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CGameEmulatorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CGameEmulatorDoc commands
