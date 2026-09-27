// PageTree.cpp : implementation file
//

#include "stdafx.h"
#include "EditorItem.h"
#include "PageTree.h"

#include "SheetWithTab.h"
// CPageTree dialog

IMPLEMENT_DYNAMIC(CPageTree, CPropertyPage)

CPageTree::CPageTree(LOGFONT logfont)
	: CPropertyPage(CPageTree::IDD),
	m_pFont ( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pDummyItemNode = new CItemNode;	
	m_bDummyHasItem = FALSE;
}

CPageTree::~CPageTree()
{
	SAFE_DELETE ( m_pDummyItemNode );
	SAFE_DELETE ( m_pFont );
	m_MapsList.CleanUp();
}

void CPageTree::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE, m_ctrlTree);
}


BEGIN_MESSAGE_MAP(CPageTree, CPropertyPage)
	ON_NOTIFY(NM_RCLICK, IDC_TREE, OnNMRClickTree)
END_MESSAGE_MAP()


// CPageTree message handlers
BOOL CPageTree::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	m_MapsList.LoadMapsListFile( "mapslist.mst" );

	UpdateTree ();

	return TRUE;  
}

BOOL CPageTree::UpdateTree ()
{	
	if ( !m_bDlgInit )	return FALSE;

	m_ctrlTree.DeleteAllItems ();
	m_ctrlTree.SetRedraw( FALSE );

	m_TreeRoot = m_ctrlTree.InsertItem ( "Root Node" );
		
	for ( int MID = 0; MID < GLItemMan::MAX_MID; MID++ )
	{
		HTREEITEM hMainItem = NULL;
		HTREEITEM hSubItem  = NULL;
		for ( int SID = 0; SID < GLItemMan::MAX_SID; SID++ )
		{        		
			SITEM* pItem = GLItemMan::GetInstance().GetItem ( MID, SID );
			if ( pItem )
			{
				CString strItem = GetFormatName ( pItem );
				if ( !hMainItem )
				{
					hMainItem = m_ctrlTree.InsertItem ( strItem, m_TreeRoot );
					m_ctrlTree.SetItemData ( hMainItem, (DWORD_PTR)pItem );
					m_TreeList[MID][SID] = hMainItem;
				}else{
					hSubItem = m_ctrlTree.InsertItem ( strItem, hMainItem );
					m_ctrlTree.SetItemData ( hSubItem, (DWORD_PTR)pItem );
					m_TreeList[MID][SID] = hSubItem;
				}				
			}
		}
	}

	m_ctrlTree.SetRedraw( TRUE );

	return TRUE;
}

BOOL CPageTree::PreTranslateMessage(MSG* pMsg)
{
	switch ( pMsg->message )
	{
	case WM_KEYDOWN:
		{
			if ( pMsg->wParam == VK_RETURN )
			{
				HTREEITEM hItem = m_ctrlTree.GetSelectedItem ();
				if ( !hItem ) return TRUE;

				PITEMNODE pItemNode = (PITEMNODE)m_ctrlTree.GetItemData ( hItem );
				if ( !pItemNode )	return TRUE;

				m_pSheetTab->ActivePageEdit1 ( PAGE_TREE, &pItemNode->m_sItem );
			}

			if ( ::GetKeyState ( VK_CONTROL ) < 0 )
			{
				if ( pMsg->wParam == 'C' )
				{
					RunSelectMenu ( ID_TOOL_COPY );
				}
			}

			if ( ::GetKeyState ( VK_CONTROL ) < 0 )
			{
				if ( pMsg->wParam == 'V' )
				{
					HTREEITEM hItem = m_ctrlTree.GetSelectedItem ();
					PITEMNODE pItemNode = (PITEMNODE)m_ctrlTree.GetItemData ( hItem );
					SetSelectItem ( pItemNode );
					RunSelectMenu ( ID_TOOL_PASTE_NEW );
				}
			}

			if ( ::GetKeyState ( VK_CONTROL ) < 0 )
			{
				if ( pMsg->wParam == 'B' )
				{
					HTREEITEM hItem = m_ctrlTree.GetSelectedItem ();
					PITEMNODE pItemNode = (PITEMNODE)m_ctrlTree.GetItemData ( hItem );
					SetSelectItem ( pItemNode );
					RunSelectMenu ( ID_TOOL_PASTE_VAL );
				}
			}

			if ( ::GetKeyState ( VK_DELETE ) < 0 )
			{
				HTREEITEM hItem = m_ctrlTree.GetSelectedItem ();
				PITEMNODE pItemNode = (PITEMNODE)m_ctrlTree.GetItemData ( hItem );
				SetSelectItem ( pItemNode );
				RunSelectMenu ( ID_TOOL_DELETE  );
			}
		}
		break;
	}
	return CPropertyPage::PreTranslateMessage(pMsg);
}

void CPageTree::OnNMRClickTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;

	*pResult = 0;	

	POINT OriginPoint;
	GetCursorPos ( &OriginPoint );
	ForceSelectOnRightClick ( OriginPoint );
	RunSelectMenu ( GetSelectMenu ( OriginPoint ) );
}

void CPageTree::ForceSelectOnRightClick ( POINT pt )
{		
	m_ctrlTree.ScreenToClient( &pt );
	UINT uFlag = 0;
	PITEMNODE pItemNode = NULL;
	HTREEITEM hItem = m_ctrlTree.HitTest ( CPoint(pt), &uFlag );

	SetSelectItem ( NULL );	
	if ( hItem )
	{
		m_ctrlTree.SelectItem ( hItem );
		if ( uFlag & TVHT_ONITEMLABEL )
		{			
			SetSelectItem ( (PITEMNODE)m_ctrlTree.GetItemData ( hItem ) );
		}
	}
}

int	 CPageTree::GetSelectMenu ( POINT pt )
{
	int pMenuID = 0;
	CMenu Menu;
	int SelectionMade = -1;
	VERIFY ( Menu.LoadMenu ( IDR_MENU_EDIT ) );
	CMenu* pPopup = Menu.GetSubMenu ( pMenuID );
	ASSERT ( pPopup != NULL );	
	SelectionMade = pPopup->TrackPopupMenu ( TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, this );
	pPopup->DestroyMenu ();
	return SelectionMade;
}

void CPageTree::RunSelectMenu ( int nSelect )
{
	HTREEITEM hItem = m_ctrlTree.GetSelectedItem ();

	PITEMNODE pItemNode = NULL;
	if ( hItem )
	{
		pItemNode = (PITEMNODE)m_ctrlTree.GetItemData ( hItem );
	}

	switch ( nSelect )
	{
	case ID_TOOL_EDIT1:
		{
			if ( GetSelectItem () )	
			{
				if ( m_pSheetTab )
				{
					m_pSheetTab->ActivePageEdit1( PAGE_TREE, &pItemNode->m_sItem );	
				}
			}
		}break;

	case ID_TOOL_EDIT2:
		{
			if ( GetSelectItem () )	
			{
				if ( m_pSheetTab )
				{
					m_pSheetTab->ActivePageEdit2( PAGE_TREE, &pItemNode->m_sItem );	
				}
			}
		}break;

	case ID_TOOL_ADD:
		{
			AddItem ();
		}
		break;
	case ID_TOOL_DELETE:
		{
			if ( GetSelectItem () )
			{
				CString strTemp;
				strTemp.Format ( "Do you want to delete? [%03d/%03d] %s"
					,GetSelectItem ()->m_sItem.sBasicOp.sNativeID.wMainID
					,GetSelectItem ()->m_sItem.sBasicOp.sNativeID.wSubID
					,GetSelectItem ()->m_sItem.GetName() );

				if ( MessageBox ( strTemp, "Question", MB_YESNO ) == IDYES )	
					DelItem();
			}
		}break;
	case ID_TOOL_COPY:
		{
			CopyItem ();
		}break;
	case ID_TOOL_PASTE_NEW:
		{
			PasteItemNew ();
		}break;
	case ID_TOOL_PASTE_VAL:
		{
			if ( GetSelectItem () )
			{
				CString strTemp;
				strTemp.Format ( "Do you want to replace? [%03d/%03d] %s"
					,GetSelectItem ()->m_sItem.sBasicOp.sNativeID.wMainID
					,GetSelectItem ()->m_sItem.sBasicOp.sNativeID.wSubID
					,GetSelectItem ()->m_sItem.GetName() );

				if ( MessageBox ( strTemp, "Question", MB_YESNO ) == IDYES )	
					PasteItemValue ();
			}
		}break;
	};
}

void CPageTree::SelectTree( WORD wMID, WORD wSID )
{
	if ( wMID >=GLItemMan::MAX_MID )	return;
	if ( wSID >= GLItemMan::MAX_SID)	return;

	HTREEITEM hItem = m_TreeList[wMID][wSID];

	SetSelectItem ( NULL );	
	m_ctrlTree.SelectItem ( hItem );
	SetSelectItem ( (PITEMNODE)m_ctrlTree.GetItemData ( hItem ) );

	CWnd *pWnd = GetDlgItem(IDC_TREE);
	pWnd->SetFocus();		
}

BOOL CPageTree::UpdateItem ( SITEM* pItem, HTREEITEM hMainItem )
{
	CString strItem;
	strItem = GetFormatName ( pItem );

	if ( !hMainItem )
	{
		hMainItem = m_ctrlTree.InsertItem ( strItem, m_TreeRoot );
		m_ctrlTree.SetItemData ( hMainItem, (DWORD_PTR)pItem );
	}else{
		HTREEITEM hSubItem = m_ctrlTree.InsertItem ( strItem, hMainItem );
		m_ctrlTree.SetItemData ( hSubItem, (DWORD_PTR)pItem );
	}

	return TRUE;
}

void CPageTree::UpdateName ()
{
	HTREEITEM hItem = m_ctrlTree.GetSelectedItem ();
	GASSERT ( hItem );
	PITEMNODE pItemNode = (PITEMNODE)m_ctrlTree.GetItemData ( hItem );
	GASSERT ( pItemNode );
	m_ctrlTree.SetItemText ( hItem, GetFormatName ( &pItemNode->m_sItem ) );
}