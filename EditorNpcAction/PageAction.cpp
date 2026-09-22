// PageAction.cpp : implementation file
//

#include "stdafx.h"
#include "EditorNpcAction.h"
#include "PageAction.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"

#include "NpcTalkControl.h"
#include "NpcTalk.h"
// CPageAction dialog

IMPLEMENT_DYNAMIC(CPageAction, CPropertyPage)

CPageAction::CPageAction( LOGFONT logfont )
	: CPropertyPage(CPageAction::IDD)
	, m_pFont ( NULL )
{
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageAction::~CPageAction()
{
	SAFE_DELETE ( m_pFont );
}

void CPageAction::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_STATEMENT, m_listctrlCase);
	DDX_Control(pDX, IDC_LIST_ANSWER, m_listctrlAnswer);
}


BEGIN_MESSAGE_MAP(CPageAction, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_OK, OnBnClickedButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnBnClickedButtonCancel)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_STATEMENT, OnLvnItemchangedListStatement)
	ON_BN_CLICKED(IDC_BUTTON_DATA_NEW, OnBnClickedButtonDataNew)
	ON_BN_CLICKED(IDC_BUTTON_DATA_EDIT, OnBnClickedButtonDataEdit)
	ON_BN_CLICKED(IDC_BUTTON_DATA_COPY, OnBnClickedButtonDataCopy)
	ON_BN_CLICKED(IDC_BUTTON_DATA_DELETE, OnBnClickedButtonDataDelete)
	ON_BN_CLICKED(IDC_BUTTON_DATA_UP, OnBnClickedButtonDataUp)
	ON_BN_CLICKED(IDC_BUTTON_DATA_DOWN, OnBnClickedButtonDataDown)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_STATEMENT, OnNMDblclkListStatement)
END_MESSAGE_MAP()


// CPageAction message handlers
BOOL CPageAction::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	LONG lnWidth;
	LV_COLUMN lvColumn;

	m_listctrlCase.SetExtendedStyle ( m_listctrlCase.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_listctrlCase.GetClientRect ( &rectCtrl );
	lnWidth = rectCtrl.right - rectCtrl.left;

	const int nColumnCount = 4;	
	char* szColumnName1[nColumnCount] = { "ID", "Conversation", "Condition", "Answer Num" };
	int nColumnWidthPercent1[nColumnCount] = { 5, 60, 25, 10 };

	for ( int i = 0; i < nColumnCount; i++ )
	{		
		lvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvColumn.fmt = LVCFMT_LEFT;
		lvColumn.pszText = szColumnName1[i];
		lvColumn.iSubItem = i;
		lvColumn.cx = ( lnWidth*nColumnWidthPercent1[i] ) / 100;
		m_listctrlCase.InsertColumn (i, &lvColumn );
	}

	char* szColumnName2[nColumnCount] = { "ID", "Answer", "Action Type", "Data" };
	int nColumnWidthPercent2[nColumnCount] = { 5, 45, 15, 35 };

	m_listctrlAnswer.SetExtendedStyle ( m_listctrlAnswer.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_listctrlAnswer.GetClientRect ( &rectCtrl );
	lnWidth = rectCtrl.right - rectCtrl.left;

	for ( int i = 0; i < nColumnCount; i++ )
	{		
		lvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvColumn.fmt = LVCFMT_LEFT;
		lvColumn.pszText = szColumnName2[i];
		lvColumn.iSubItem = i;
		lvColumn.cx = ( lnWidth*nColumnWidthPercent2[i] ) / 100;
		m_listctrlAnswer.InsertColumn (i, &lvColumn );
	}


	return TRUE;  
}

void CPageAction::OnBnClickedButtonOk()
{
	DWORD NativeID = GetWin_Num_int( this, IDC_EDIT_STATEMENT_ID );
	m_DummyDialogue.SetDlgNID ( NativeID );

	if ( m_pDialogue )
	{
		*m_pDialogue = m_DummyDialogue;
	}
	else
	{
		DWORD dwIndex = m_DummyDialogue.FindNonCondition();
		if ( dwIndex==UINT_MAX )	dwIndex = 0;	
		if ( !m_DummyDialogue.GetDlgCase(dwIndex) )
		{
			m_pSheetTab->ActiveActionTable ();
			return;
		}

		CNpcDialogue* pNewDialogue = new CNpcDialogue;
		*pNewDialogue = m_DummyDialogue;

		CNpcDialogueSet* pDialogueSet = m_pSheetTab->GetDialogueSet ();
		if ( !pDialogueSet->AddDialogue ( pNewDialogue ) )
		{
			MessageBox ( "ID Already Exist" );
			SAFE_DELETE ( pNewDialogue );
			return;
		}
	}

	m_pSheetTab->ActiveActionTable ();
}

void CPageAction::OnBnClickedButtonCancel()
{
	m_pSheetTab->ActiveActionTable ();
}

void CPageAction::SetDialogue ( CNpcDialogueSet *pNpcDialogueSet, CNpcDialogue* pDialogue )
{	
	m_DummyDialogue.Reset ();

	m_pNpcDialogueSet = pNpcDialogueSet;
	m_pDialogue = pDialogue;
	if ( m_pDialogue )
	{
		m_DummyDialogue = *m_pDialogue;		
	}
	else
	{
		DWORD NewNativeID = m_pSheetTab->GetDialogueSet()->MakeDlgNID();
		m_DummyDialogue.SetDlgNID ( NewNativeID );
	}
}

void CPageAction::OnLvnItemchangedListStatement(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	
	InformationPreview();

	*pResult = 0;
}

void CPageAction::OnBnClickedButtonDataNew()
{
	CNpcDialogueCase sCase;
	DWORD dwSize = m_DummyDialogue.AddCase ( sCase );
	CNpcDialogueCase* pCase = m_DummyDialogue.GetDlgCase ( dwSize-1 );
	m_pSheetTab->ActiveActionAnswer ( m_pNpcDialogueSet, pCase );
}

void CPageAction::OnBnClickedButtonDataEdit()
{
	POSITION pos = m_listctrlCase.GetFirstSelectedItemPosition();
	if (pos == NULL)
	{
		MessageBox ( "Select the branching statement to edit.", "Information", MB_OK );
		return;
	}

	int nItem = m_listctrlCase.GetNextSelectedItem(pos);
	DWORD dwData = (DWORD) m_listctrlCase.GetItemData ( nItem );
	CNpcDialogueCase* pCase = m_DummyDialogue.GetDlgCase ( dwData );
	m_pSheetTab->ActiveActionAnswer ( m_pNpcDialogueSet, pCase );
}

void CPageAction::OnBnClickedButtonDataCopy()
{
	POSITION pos = m_listctrlCase.GetFirstSelectedItemPosition();
	if (pos == NULL)
	{
		MessageBox ( "Select the branching statement to copy.", "Information", MB_OK );
		return;
	}

	int nItem = m_listctrlCase.GetNextSelectedItem(pos);
	DWORD dwData = (DWORD) m_listctrlCase.GetItemData ( nItem );

	CNpcDialogueCase* pCase = m_DummyDialogue.GetDlgCase ( dwData );
	CNpcDialogueCase sCase;

	sCase = *pCase;		
	m_DummyDialogue.AddCase ( sCase );

	UpdateList ();
}

void CPageAction::OnBnClickedButtonDataDelete()
{
	POSITION pos = m_listctrlCase.GetFirstSelectedItemPosition();
	if (pos == NULL)
	{
		MessageBox ( "Select the branching statement to delete.", "Information", MB_OK );
		return;
	}

	if ( IDYES==MessageBox("Do you want to delete 'Branching Statement'?","Caution",MB_YESNO) )
	{
		int nItem = m_listctrlCase.GetNextSelectedItem(pos);
		DWORD dwData = (DWORD) m_listctrlCase.GetItemData ( nItem );
		m_DummyDialogue.DelCase ( dwData );

		UpdateList ();
	}
}

void CPageAction::OnBnClickedButtonDataUp()
{
	POSITION pos = m_listctrlCase.GetFirstSelectedItemPosition();
	if (pos == NULL)	return;

	int nItem = m_listctrlCase.GetNextSelectedItem(pos);
	DWORD dwData = (DWORD) m_listctrlCase.GetItemData ( nItem );
	DWORD dwNEW_ID = m_DummyDialogue.ToUp ( dwData );

	if ( dwNEW_ID!=dwData )
	{
		UpdateList ();
		m_listctrlCase.SetItemState( --nItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED ); 
	}
}

void CPageAction::OnBnClickedButtonDataDown()
{
	POSITION pos = m_listctrlCase.GetFirstSelectedItemPosition();
	if (pos == NULL)	return;

	int nItem = m_listctrlCase.GetNextSelectedItem(pos);
	DWORD dwData = (DWORD) m_listctrlCase.GetItemData ( nItem );
	DWORD dwNEW_ID = m_DummyDialogue.ToDown ( dwData );

	if ( dwNEW_ID!=dwData )
	{
		UpdateList ();
		m_listctrlCase.SetItemState( ++nItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED ); 
	}
}

void CPageAction::OnNMDblclkListStatement(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	NMITEMACTIVATE* pNMIA = (NMITEMACTIVATE*) pNMHDR;

	if ( pNMIA->iItem == -1 )
		return ;

	DWORD dwData = (DWORD) m_listctrlCase.GetItemData ( pNMIA->iItem );
	CNpcDialogueCase* pCase = m_DummyDialogue.GetDlgCase ( dwData );
	m_pSheetTab->ActiveActionAnswer ( m_pNpcDialogueSet, pCase );
}
