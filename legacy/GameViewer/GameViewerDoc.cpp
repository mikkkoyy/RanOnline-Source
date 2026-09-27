
// GameViewerDoc.cpp : implementation of the CGameViewerDoc class
//

#include "stdafx.h"
#include "GameViewer.h"

#include "GameViewerDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGameViewerDoc

IMPLEMENT_DYNCREATE(CGameViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(CGameViewerDoc, CDocument)
END_MESSAGE_MAP()


// CGameViewerDoc construction/destruction

CGameViewerDoc::CGameViewerDoc()
{
	// TODO: add one-time construction code here

}

CGameViewerDoc::~CGameViewerDoc()
{
}

BOOL CGameViewerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CGameViewerDoc serialization

void CGameViewerDoc::Serialize(CArchive& ar)
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


// CGameViewerDoc diagnostics

#ifdef _DEBUG
void CGameViewerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CGameViewerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CGameViewerDoc commands
