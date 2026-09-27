// PageQuestTree.cpp : implementation file
//

#include "stdafx.h"
#include "EditorQuest.h"
#include "PageQuestTree.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"

// CPageQuestTree dialog

IMPLEMENT_DYNAMIC(CPageQuestTree, CPropertyPage)

CPageQuestTree::CPageQuestTree( LOGFONT logfont )
	: CPropertyPage(CPageQuestTree::IDD)
	, m_pFont ( NULL )
{
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);

	m_pDummyQuestNode = new QUESTNODE;
	m_pDummyQuestNode->pQUEST = new GLQUEST;
	m_bDummyHasQuest = FALSE;
}

CPageQuestTree::~CPageQuestTree()
{
	SAFE_DELETE ( m_pFont );
	SAFE_DELETE ( m_pDummyQuestNode->pQUEST );
	SAFE_DELETE ( m_pDummyQuestNode );	
}

void CPageQuestTree::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE, m_ctrlQuestTree);
}


BEGIN_MESSAGE_MAP(CPageQuestTree, CPropertyPage)
	ON_NOTIFY(NM_RCLICK, IDC_TREE, OnNMRClickTree)
END_MESSAGE_MAP()


// CPageQuestTree message handlers
BOOL CPageQuestTree::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_bDlgInit = TRUE;

	CheckInvalid();
	UpdateTree ();
	
	return TRUE;
}

BOOL CPageQuestTree::UpdateTree ()
{	
	if ( !m_bDlgInit )
		return FALSE;

	CleanAllQuest ();

	GLQuestMan::MAPQUEST& m_mapQuestMap = GLQuestMan::GetInstance().GetMap();

	GLQuestMan::MAPQUEST_ITER iter_pos;
	DWORD dwQuestID = 0;
	PQUESTNODE pQuestNode = NULL;
	CString strItem;

	m_ctrlQuestTree.SetRedraw( FALSE );

	for ( iter_pos = m_mapQuestMap.begin(); iter_pos != m_mapQuestMap.end(); iter_pos++ )
	{
		HTREEITEM hMainItem= NULL;

		dwQuestID = (DWORD) iter_pos->first;	
		pQuestNode = (PQUESTNODE) &iter_pos->second;

		if ( pQuestNode ) 
		{	
			strItem = GetFormatName ( pQuestNode );
			hMainItem = m_ctrlQuestTree.InsertItem ( strItem, m_TreeRoot );
			m_ctrlQuestTree.SetItemData ( hMainItem, (DWORD_PTR)pQuestNode );
			m_TreeList[pQuestNode->pQUEST->m_sNID.dwID] = hMainItem;
		}
	}

	m_ctrlQuestTree.SetRedraw( TRUE );

	return TRUE;
}

void CPageQuestTree::CleanAllQuest ()
{
	if ( !m_bDlgInit )
		return;

	m_ctrlQuestTree.DeleteAllItems ();
	m_TreeRoot = m_ctrlQuestTree.InsertItem ( "Root Node" );
}

const char* CPageQuestTree::GetFormatName ( PQUESTNODE pQuestNode )
{
	GASSERT ( pQuestNode->pQUEST );

	static	CString strItem;

	if ( pQuestNode->pQUEST )
		strItem.Format ( "[%04d][%02d]  [%s] [%s]", pQuestNode->pQUEST->m_sNID, (WORD)pQuestNode->pQUEST->m_emQuestArea, pQuestNode->pQUEST->m_strTITLE.c_str(), pQuestNode->strFILE.c_str() );

	return strItem.GetString ();
}

BOOL CPageQuestTree::UpdateItem ( PQUESTNODE pQuestNode, HTREEITEM hMainItem )
{
	CString strItem;
	strItem = GetFormatName ( pQuestNode );

	if ( !hMainItem )
	{
		hMainItem = m_ctrlQuestTree.InsertItem ( strItem, m_TreeRoot );
		m_ctrlQuestTree.SetItemData ( hMainItem, (DWORD_PTR)pQuestNode );
	}
	else
	{
		HTREEITEM hSubItem = m_ctrlQuestTree.InsertItem ( strItem, hMainItem );
		m_ctrlQuestTree.SetItemData ( hSubItem, (DWORD_PTR)pQuestNode );
	}

	return TRUE;
}

void CPageQuestTree::UpdateName ()
{
	HTREEITEM hItem = m_ctrlQuestTree.GetSelectedItem ();
	GASSERT ( hItem );
	PQUESTNODE pQuestNode = (PQUESTNODE)m_ctrlQuestTree.GetItemData ( hItem );
	GASSERT ( pQuestNode );

	m_ctrlQuestTree.SetItemText ( hItem, GetFormatName ( pQuestNode ) );
}

void CPageQuestTree::OnNMRClickTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;

	POINT OriginPoint;
	GetCursorPos ( &OriginPoint );

	ForceSelectOnRightClick ( OriginPoint );
	RunSelectMenu ( GetSelectMenu ( OriginPoint ) );
}

void CPageQuestTree::ForceSelectOnRightClick ( POINT pt )
{		
	m_ctrlQuestTree.ScreenToClient( &pt );

	UINT uFlag = 0;
	PQUESTNODE pItemNode = NULL;
	HTREEITEM hItem = m_ctrlQuestTree.HitTest ( CPoint(pt), &uFlag );

	SetSelectItem ( NULL );	
	if ( hItem )
	{
		m_ctrlQuestTree.SelectItem ( hItem );
		if ( uFlag & TVHT_ONITEMLABEL )
		{			
			SetSelectItem ( (PQUESTNODE)m_ctrlQuestTree.GetItemData ( hItem ) );
		}
	}
}

void CPageQuestTree::RunSelectMenu ( int nSelect )
{
	HTREEITEM hItem = m_ctrlQuestTree.GetSelectedItem ();

	PQUESTNODE pQuestNode = NULL;
	if ( hItem )
	{
		pQuestNode = (PQUESTNODE)m_ctrlQuestTree.GetItemData ( hItem );
	}

	switch ( nSelect )
	{
	case ID_FILE_QUESTEDIT:
		{
			if ( GetSelectItem () )
			{				
				m_pSheetTab->ActiveQuestBasicPage ( QUEST_TREE_PAGE, pQuestNode );
			}			
		}break;

	case ID_FILE_QUESTSTEPEDIT:
		{
			if ( GetSelectItem () )
			{
				m_pSheetTab->ActiveQuestStepPage ( QUEST_TREE_PAGE, pQuestNode );
			}			
		}break;

	case ID_FILE_ADD:
		{
			AddQuest();
		}break;

	case ID_FILE_DEL:
		{
			if ( GetSelectItem () )
			{
				if ( MessageBox ( "Do you want to delete?", "Question", MB_YESNO ) == IDYES )
					DelQuest();
			}
		}break;

	case ID_FILE_COPY:
		{
			CopyQuest ();
		}break;

	case ID_FILE_PASTE:
		{
			PasteQuestNew ();
		}break;

	case ID_FILE_PASTEVALUE:
		{
			if ( MessageBox ( "Do you want to overwrite?", "Question", MB_YESNO ) == IDYES )
				PasteQuestValue ();
		}break;
	};
}

int	 CPageQuestTree::GetSelectMenu ( POINT pt )
{
	int pMenuID = 0;
	CMenu Menu;
	int SelectionMade = -1;

	VERIFY ( Menu.LoadMenu ( IDR_MENU_QUEST ) );
	CMenu* pPopup = Menu.GetSubMenu ( pMenuID );
	ASSERT ( pPopup != NULL );	

	SelectionMade = pPopup->TrackPopupMenu ( TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, this );
	pPopup->DestroyMenu ();

	return SelectionMade;
}

void CPageQuestTree::SetSelectItem ( PQUESTNODE pQuestNode )
{
	m_pQuestNode = pQuestNode;
}

PQUESTNODE CPageQuestTree::GetSelectItem ()
{
	return m_pQuestNode;
}

BOOL CPageQuestTree::PreTranslateMessage(MSG* pMsg)
{
	switch ( pMsg->message )
	{
	case WM_KEYDOWN:
		{
			if ( pMsg->wParam == VK_RETURN )
			{
				HTREEITEM hItem = m_ctrlQuestTree.GetSelectedItem ();
				if ( !hItem ) 
					return TRUE;

				PQUESTNODE pQuestNode = (PQUESTNODE)m_ctrlQuestTree.GetItemData ( hItem );
				if ( !pQuestNode )
					return TRUE;

				m_pSheetTab->ActiveQuestBasicPage ( QUEST_BASIC_PAGE, pQuestNode );
			}

			if ( ::GetKeyState ( VK_CONTROL ) < 0 )
			{
				if ( pMsg->wParam == 'C' )
				{
					RunSelectMenu ( ID_FILE_COPY );
				}
			}

			if ( ::GetKeyState ( VK_CONTROL ) < 0 )
			{
				if ( pMsg->wParam == 'V' )
				{
					HTREEITEM hItem = m_ctrlQuestTree.GetSelectedItem ();
					PQUESTNODE pQuestNode = (PQUESTNODE)m_ctrlQuestTree.GetItemData ( hItem );
					SetSelectItem ( pQuestNode );
					RunSelectMenu ( ID_FILE_PASTE );
				}
			}

			if ( ::GetKeyState ( VK_CONTROL ) < 0 )
			{
				if ( pMsg->wParam == 'B' )
				{
					HTREEITEM hItem = m_ctrlQuestTree.GetSelectedItem ();
					PQUESTNODE pQuestNode = (PQUESTNODE)m_ctrlQuestTree.GetItemData ( hItem );
					SetSelectItem ( pQuestNode );
					RunSelectMenu ( ID_FILE_PASTEVALUE );
				}
			}

			if ( ::GetKeyState ( VK_DELETE ) < 0 )
			{
				HTREEITEM hItem = m_ctrlQuestTree.GetSelectedItem ();
				PQUESTNODE pQuestNode = (PQUESTNODE)m_ctrlQuestTree.GetItemData ( hItem );
				SetSelectItem ( pQuestNode );
				RunSelectMenu ( ID_FILE_DEL );
			}
		}break;
	}
	return CPropertyPage::PreTranslateMessage(pMsg);
}

void CPageQuestTree::SelectTree( DWORD dwID )
{
	if ( dwID >=MAXWORD )	return;

	HTREEITEM hItem = m_TreeList[dwID];

	SetSelectItem ( NULL );	
	m_ctrlQuestTree.SelectItem ( hItem );
	SetSelectItem ( (PQUESTNODE)m_ctrlQuestTree.GetItemData ( hItem ) );

	CWnd *pWnd = GetDlgItem(IDC_TREE);
	pWnd->SetFocus();		
}

void CPageQuestTree::CheckInvalid()
{
	GLQuestMan::MAPQUEST& m_mapQuestMap = GLQuestMan::GetInstance().GetMap();
	GLQuestMan::MAPQUEST_ITER iter_pos;
	std::vector<DWORD> vecDel;

	for ( iter_pos = m_mapQuestMap.begin(); iter_pos != m_mapQuestMap.end(); iter_pos++ )
	{
		DWORD dwQuestID = (DWORD) iter_pos->first;	
		PQUESTNODE pQuestNode = (PQUESTNODE) &iter_pos->second;

		if ( pQuestNode ) 
		{	
			if ( !pQuestNode->pQUEST )
			{
				CString strERROR;
				strERROR.Format( "Quest File: %s is not found! \n It is recommended to delete the file pointer! \n Invalid file pointer will cause unexpected crashes! \n Delete File Pointer?", pQuestNode->strFILE.c_str() );
				
				if ( MessageBox ( strERROR.GetString(), "Question", MB_YESNO ) == IDYES )
				{
					vecDel.push_back( dwQuestID );
				}
			}
		}
	}

	for ( size_t i = 0; i < vecDel.size(); ++ i )
	{
		GLQuestMan::GetInstance().Delete( vecDel[i] );
	}
}