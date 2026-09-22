
// EditorSkinCharDoc.cpp : implementation of the CEditorSkinCharDoc class
//

#include "stdafx.h"
#include "EditorSkinChar.h"
#include "EditorSkinCharDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CEditorSkinCharDoc

IMPLEMENT_DYNCREATE(CEditorSkinCharDoc, CDocument)

BEGIN_MESSAGE_MAP(CEditorSkinCharDoc, CDocument)
END_MESSAGE_MAP()


// CEditorSkinCharDoc construction/destruction

CEditorSkinCharDoc::CEditorSkinCharDoc()
{
	// TODO: add one-time construction code here

}

CEditorSkinCharDoc::~CEditorSkinCharDoc()
{
}

BOOL CEditorSkinCharDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CEditorSkinCharDoc serialization

void CEditorSkinCharDoc::Serialize(CArchive& ar)
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

// CEditorSkinCharDoc diagnostics

#ifdef _DEBUG
void CEditorSkinCharDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CEditorSkinCharDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CEditorSkinCharDoc commands
