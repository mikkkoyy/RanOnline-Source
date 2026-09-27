// paddoc.cpp : implementation of the CPadDoc class
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.


#include "stdafx.h"
#include "EditorText.h"
#include "paddoc.h"
#include "paditem.h"
#include "linkitem.h"
#include ".\paddoc.h"
#include "../Lib_Engine/Common/StringFile.h"
#include "../Lib_Engine/Common/Rijndael.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CPadDoc

IMPLEMENT_DYNCREATE(CPadDoc, COleServerDoc)

BEGIN_MESSAGE_MAP(CPadDoc, COleServerDoc)
	//{{AFX_MSG_MAP(CPadDoc)
	ON_COMMAND(ID_VIEW_UPDATENOW, OnViewUpdatenow)
	ON_COMMAND(ID_CANCEL_INPLACE, OnCancelInplace)
//	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
//	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
//	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPadDoc delegation to CEditView

CPadDoc::CPadDoc()
{
}

CPadDoc::~CPadDoc()
{
}

void CPadDoc::DeleteContents()
{
	COleServerDoc::DeleteContents();
	if (m_viewList.IsEmpty())
		return;
	CEditView* pView = (CEditView*)m_viewList.GetHead();
	ASSERT_KINDOF(CEditView, pView);
	pView->DeleteContents();
}

void CPadDoc::Serialize(CArchive& ar)
{
	CEditView* pView = (CEditView*)m_viewList.GetHead();
	ASSERT_KINDOF(CEditView, pView);
	pView->SerializeRaw(ar);
}

COleServerItem* CPadDoc::OnGetEmbeddedItem()
{
	CEmbeddedItem* pItem = new CEmbeddedItem(this);
	ASSERT_VALID(pItem);
	return pItem;
}

COleServerItem* CPadDoc::OnGetLinkedItem(LPCTSTR lpszItemName)
{
	CPadLinkItem *pItem =
		(CPadLinkItem*)COleServerDoc::OnGetLinkedItem(lpszItemName);
	if (pItem == NULL)
		pItem = new CPadLinkItem(this, lpszItemName);

	ASSERT_VALID(pItem);
	return pItem;
}

/////////////////////////////////////////////////////////////////////////////

void CPadDoc::OnViewUpdatenow()
{
	UpdateAllItems(NULL);
}

// Note: both the server and the container should have a keyboard method
//  of deactivating an active in-place item.

void CPadDoc::OnCancelInplace()
{
	if (IsInPlaceActive())
		OnDeactivateUI(FALSE);
}

void CPadDoc::SetSelection(int nBeg, int nEnd)
{
	CEditView* pView = (CEditView*)m_viewList.GetHead();
	ASSERT_KINDOF(CEditView, pView);
	pView->GetEditCtrl().SetSel(nBeg, nEnd);
}

BOOL CPadDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!COleServerDoc::OnOpenDocument(lpszPathName))
		return FALSE;

	char ext[_MAX_EXT] = {0};
	_splitpath( lpszPathName, NULL, NULL, NULL, ext );

	CString strExt(ext);
	strExt.MakeUpper();

	if( strExt != _T(".CHARSET") && strExt != _T(".CHARCLASS") &&
		strExt != _T(".CLASSCONST") && strExt != _T(".CROWSALE") &&
		strExt != _T(".GENITEM") && strExt != _T(".BIN") )
	{
		if( IDNO == AfxMessageBox( "Decode File?", MB_YESNO ) )
			return TRUE;
	}

	CStringFile StrFile;
	if ( !StrFile.Open ( lpszPathName, TRUE ) )
		return FALSE;

	CString strTemp, strLine;
	while ( StrFile.GetNextLine(strLine) )
	{
		strLine += "\r\n";
		strTemp += strLine;
	}

	strTemp.SetAt(strTemp.GetLength()-2, NULL); 

	reinterpret_cast<CEditView*>(m_viewList.GetHead())->SetWindowText(strTemp.GetString());

	return TRUE;
}

BOOL CPadDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	char ext[_MAX_EXT] = {0};
	_splitpath( lpszPathName, NULL, NULL, NULL, ext );

	CString strExt(ext);
	strExt.MakeUpper();

	if( strExt != _T(".CHARSET") && strExt != _T(".CHARCLASS") &&
		strExt != _T(".CLASSCONST") && strExt != _T(".CROWSALE") &&
		strExt != _T(".GENITEM") && strExt != _T(".BIN") )
	{
		if( IDNO == AfxMessageBox( "Encode File?", MB_YESNO ) )
			return COleServerDoc::OnSaveDocument(lpszPathName);
	}

	CString strTemp;
	reinterpret_cast<CEditView*>(m_viewList.GetHead())->GetWindowText(strTemp);

	if ( strTemp.GetLength() == 0 )
	{
		strTemp += ";RanOnline Logic Text File..";
		strTemp += "\n";
	}

	CRijndael oRijndael;
	oRijndael.Initialize( CRijndael::VERSION, CRijndael::sm_Version[CRijndael::VERSION-1].c_str(), CRijndael::sm_KeyLength[CRijndael::VERSION-1] );

	int nLen = oRijndael.GetEncryptStringLength( strTemp );
	char * szBuffer = new char[nLen];

	oRijndael.EncryptEx( strTemp.GetString(), szBuffer, strTemp.GetLength() );
	int nVersion = CRijndael::VERSION;

	CFile file;
	file.Open( lpszPathName, CFile::modeCreate|CFile::modeWrite );
	file.Write( &nVersion, sizeof(int) );
	file.Write( szBuffer, nLen );
	file.Close();

	delete [] szBuffer;

	SetModifiedFlag(FALSE);
	return TRUE;
}