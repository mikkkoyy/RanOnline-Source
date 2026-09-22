// PageTree.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkill.h"
#include "PageTree.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "GLSkill.h"
// CPageTree dialog

IMPLEMENT_DYNAMIC(CPageTree, CPropertyPage)

CPageTree::CPageTree(LOGFONT logfont)
	: CPropertyPage(CPageTree::IDD),
	m_pFont ( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pDummySkill = new GLSKILL;	
	m_bDummyHasSkill = FALSE;
}

CPageTree::~CPageTree()
{
	SAFE_DELETE ( m_pDummySkill );
	SAFE_DELETE ( m_pFont );
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

	UpdateTree ();

	return TRUE;  
}

BOOL CPageTree::UpdateTree ()
{	
	if ( !m_bDlgInit )	return FALSE;

	m_ctrlTree.DeleteAllItems();
	m_ctrlTree.SetRedraw( FALSE );

	for ( int MID = 0; MID < EMSKILLCLASS_NSIZE; MID++ )
	{
		HTREEITEM hMainItem = m_ctrlTree.InsertItem ( COMMENT::SKILLCLASS[MID].c_str() );

		for ( int SID = 0; SID < GLSkillMan::MAX_CLASSSKILL; SID++ )
		{        		
			PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( MID, SID );
			if ( pSKILL )
			{
				HTREEITEM hSubItem = m_ctrlTree.InsertItem ( GetFormatName ( pSKILL ), hMainItem );
				m_ctrlTree.SetItemData ( hSubItem, (DWORD_PTR)pSKILL );
				m_TreeList[MID][SID] = hSubItem;			
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

				PGLSKILL pSKILL = (PGLSKILL)m_ctrlTree.GetItemData ( hItem );
				if ( !pSKILL )	return TRUE;

				m_pSheetTab->ActivePageEdit1 ( PAGE_TREE, pSKILL );
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
					PGLSKILL pSKILL = (PGLSKILL)m_ctrlTree.GetItemData ( hItem );
					SetSelectSkill ( pSKILL );
					RunSelectMenu ( ID_TOOL_PASTE_NEW );
				}
			}

			if ( ::GetKeyState ( VK_CONTROL ) < 0 )
			{
				if ( pMsg->wParam == 'B' )
				{
					HTREEITEM hItem = m_ctrlTree.GetSelectedItem ();
					PGLSKILL pSKILL = (PGLSKILL)m_ctrlTree.GetItemData ( hItem );
					SetSelectSkill ( pSKILL );
					RunSelectMenu ( ID_TOOL_PASTE_VAL );
				}
			}

			if ( ::GetKeyState ( VK_DELETE ) < 0 )
			{
				HTREEITEM hItem = m_ctrlTree.GetSelectedItem ();
				PGLSKILL pSKILL = (PGLSKILL)m_ctrlTree.GetItemData ( hItem );
				SetSelectSkill ( pSKILL );
				RunSelectMenu ( ID_TOOL_DELETE  );
			}
		}
		break;
	}
	return CPropertyPage::PreTranslateMessage(pMsg);
}

void CPageTree::ForceSelectOnRightClick ( POINT pt )
{		
	m_ctrlTree.ScreenToClient( &pt );
	UINT uFlag = 0;
	HTREEITEM hItem = m_ctrlTree.HitTest ( CPoint(pt), &uFlag );

	SetSelectSkill( NULL );	
	if ( hItem )
	{
		m_ctrlTree.SelectItem ( hItem );
		if ( uFlag & TVHT_ONITEMLABEL )
		{			
			SetSelectSkill ( (PGLSKILL)m_ctrlTree.GetItemData ( hItem ) );
		}
	}
}

void CPageTree::OnNMRClickTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;

	POINT OriginPoint;
	GetCursorPos ( &OriginPoint );
	ForceSelectOnRightClick ( OriginPoint );
	RunSelectMenu ( GetSelectMenu ( OriginPoint ) );
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

	PGLSKILL pSKILL = NULL;
	if ( hItem )
	{
		pSKILL = (PGLSKILL)m_ctrlTree.GetItemData ( hItem );
	}

	switch ( nSelect )
	{
	case ID_TOOL_EDIT1:
		{
			if ( GetSelectSkill () )	
			{
				if ( m_pSheetTab )
				{
					m_pSheetTab->ActivePageEdit1( PAGE_TREE, pSKILL );	
				}
			}
		}break;

	case ID_TOOL_EDIT2:
		{
			if ( GetSelectSkill () )	
			{
				if ( m_pSheetTab )
				{
					m_pSheetTab->ActivePageEdit2( PAGE_TREE, pSKILL );	
				}
			}
		}break;

	case ID_TOOL_ADD:
		{
			AddSkill ();
		}
		break;

	case ID_TOOL_DELETE:
		{
			if ( GetSelectSkill () )
			{
				CString strTemp;
				strTemp.Format ( "Do you want to delete? [%03d/%03d] %s"
					,GetSelectSkill()->m_sBASIC.sNATIVEID.wMainID
					,GetSelectSkill()->m_sBASIC.sNATIVEID.wSubID
					,GetSelectSkill()->GetName() );

				if ( MessageBox ( strTemp, "Question", MB_YESNO ) == IDYES )	
					DelSkill();
			}
		}break;

	case ID_TOOL_COPY:
		{
			CopySkill();
		}break;
	case ID_TOOL_PASTE_NEW:
		{
			PasteSkillNew();
		}break;

	case ID_TOOL_PASTE_VAL:
		{
			if ( GetSelectSkill() )
			{
				CString strTemp;
				strTemp.Format ( "Do you want to replace? [%03d/%03d] %s"
					,GetSelectSkill()->m_sBASIC.sNATIVEID.wMainID
					,GetSelectSkill()->m_sBASIC.sNATIVEID.wSubID
					,GetSelectSkill()->GetName() );

				if ( MessageBox ( strTemp, "Question", MB_YESNO ) == IDYES )	
					PasteSkillValue();
			}
		}break;
	};
}

void CPageTree::SelectTree( WORD wMID, WORD wSID )
{
	if ( wMID >= EMSKILLCLASS_NSIZE )	return;
	if ( wSID >= GLSkillMan::MAX_CLASSSKILL )	return;

	HTREEITEM hItem = m_TreeList[wMID][wSID];

	SetSelectSkill ( NULL );	
	m_ctrlTree.SelectItem ( hItem );
	SetSelectSkill ( (PGLSKILL)m_ctrlTree.GetItemData ( hItem ) );

	CWnd *pWnd = GetDlgItem(IDC_TREE);
	pWnd->SetFocus();		
}

BOOL CPageTree::UpdateItem ( PGLSKILL pSkill, HTREEITEM hITEM )
{
	if ( !pSkill )	return FALSE;
	if ( !hITEM )	return FALSE;

	CString strItem;
	strItem = GetFormatName ( pSkill );

	HTREEITEM hINSERT = hITEM;
	HTREEITEM hParentItem = m_ctrlTree.GetParentItem ( hITEM );
	if ( hParentItem ) hINSERT = hParentItem;

	HTREEITEM hSubItem = m_ctrlTree.InsertItem ( strItem, hINSERT );
	m_ctrlTree.SetItemData ( hSubItem, (DWORD_PTR)pSkill );

	return TRUE;
}

void CPageTree::UpdateName ()
{
	HTREEITEM hItem = m_ctrlTree.GetSelectedItem ();
	GASSERT ( hItem );
	PGLSKILL pSKILL = (PGLSKILL)m_ctrlTree.GetItemData ( hItem );
	GASSERT ( pSKILL );
	m_ctrlTree.SetItemText ( hItem, GetFormatName ( pSKILL ) );
}