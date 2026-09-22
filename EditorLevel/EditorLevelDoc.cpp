
// EditorLevelDoc.cpp : implementation of the CEditorLevelDoc class
//

#include "stdafx.h"
#include "EditorLevel.h"

#include "EditorLevelDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CEditorLevelDoc

IMPLEMENT_DYNCREATE(CEditorLevelDoc, CDocument)

BEGIN_MESSAGE_MAP(CEditorLevelDoc, CDocument)
END_MESSAGE_MAP()


// CEditorLevelDoc construction/destruction

CEditorLevelDoc::CEditorLevelDoc()
{
	// TODO: add one-time construction code here

}

CEditorLevelDoc::~CEditorLevelDoc()
{
}

BOOL CEditorLevelDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CEditorLevelDoc serialization

void CEditorLevelDoc::Serialize(CArchive& ar)
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


// CEditorLevelDoc diagnostics

#ifdef _DEBUG
void CEditorLevelDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CEditorLevelDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CEditorLevelDoc commands
