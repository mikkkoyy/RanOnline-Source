
// EditorSkinPieceDoc.cpp : implementation of the CEditorSkinPieceDoc class
//

#include "stdafx.h"
#include "EditorSkinPiece.h"

#include "EditorSkinPieceDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CEditorSkinPieceDoc

IMPLEMENT_DYNCREATE(CEditorSkinPieceDoc, CDocument)

BEGIN_MESSAGE_MAP(CEditorSkinPieceDoc, CDocument)
END_MESSAGE_MAP()


// CEditorSkinPieceDoc construction/destruction

CEditorSkinPieceDoc::CEditorSkinPieceDoc()
{
	// TODO: add one-time construction code here

}

CEditorSkinPieceDoc::~CEditorSkinPieceDoc()
{
}

BOOL CEditorSkinPieceDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CEditorSkinPieceDoc serialization

void CEditorSkinPieceDoc::Serialize(CArchive& ar)
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


// CEditorSkinPieceDoc diagnostics

#ifdef _DEBUG
void CEditorSkinPieceDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CEditorSkinPieceDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CEditorSkinPieceDoc commands
