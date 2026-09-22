// PageTable.cpp : implementation file
//

#include "stdafx.h"
#include "EditorNpcAction.h"
#include "PageTable.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"

#include "NpcDialogue.h"

// CPageTable dialog

IMPLEMENT_DYNAMIC(CPageTable, CPropertyPage)

CPageTable::CPageTable( LOGFONT logfont )
	: CPropertyPage(CPageTable::IDD)
	, m_pFont ( NULL )
{
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageTable::~CPageTable()
{
	SAFE_DELETE ( m_pFont );
}

void CPageTable::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_TALK, m_ListTalk);
}


BEGIN_MESSAGE_MAP(CPageTable, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_EDIT, OnBnClickedButtonEdit)
	ON_BN_CLICKED(IDC_BUTTON_COPY, OnBnClickedButtonCopy)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, OnBnClickedButtonDelete)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_TALK, OnNMDblclkListTalk)
END_MESSAGE_MAP()


// CPageTable message handlers
BOOL CPageTable::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT	rect;
	m_ListTalk.GetClientRect ( &rect );
	LONG	Width = rect.right - rect.left;

	m_ListTalk.SetExtendedStyle ( m_ListTalk.GetExtendedStyle() |LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

	LV_COLUMN	lvcolumn;
	const int	ColumnCount = 2;	
	char*		ColumnName[ColumnCount] = { "ID", "Basic Conversation" };
	int			ColumnWidthPercent[ColumnCount] = { 6, 94 };

	for ( int i = 0; i < ColumnCount; i++ )
	{		
		lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvcolumn.fmt = LVCFMT_LEFT;
		lvcolumn.pszText = ColumnName[i];
		lvcolumn.iSubItem = i;
		lvcolumn.cx = ( Width * ColumnWidthPercent[i] ) / 100;
		m_ListTalk.InsertColumn (i, &lvcolumn);		
	}


	return TRUE;  
}

CNpcDialogueSet*	CPageTable::GetDialogueSet ()
{
	return &m_DialogueSet;
}

void CPageTable::UpdateItems ()
{
	m_ListTalk.DeleteAllItems ();
	m_ListTalk.SetRedraw( FALSE );

	CString strTemp;
	CNpcDialogueSet::DIALOGMAP* pMap = GetDialogueSet()->GetDialogueMap ();
	CNpcDialogueSet::DIALOGMAP_IT iter = pMap->begin();
	CNpcDialogueSet::DIALOGMAP_IT iter_end = pMap->end();	

	for ( int nIndex = 0; iter!=iter_end; ++iter, nIndex++ )
	{
		CNpcDialogue* pDialogue = (*iter).second;

		DWORD dwIndex = pDialogue->FindNonCondition();
		if ( dwIndex==UINT_MAX )	dwIndex = 0;

		CNpcDialogueCase* pDlgCase = pDialogue->GetDlgCase(dwIndex);
		if ( !pDlgCase )
		{
			pMap->erase( iter++ );
			--nIndex;
			continue;			
		}

		strTemp.Format ( "%d", pDialogue->GetDlgNID() );
		int nItem = m_ListTalk.InsertItem ( nIndex, strTemp );		
		m_ListTalk.SetItemText ( nIndex, 1, pDlgCase->GetBasicTalk().c_str() );
		m_ListTalk.SetItemData ( nItem, (DWORD_PTR)pDialogue->GetDlgNID() );
	}

	m_ListTalk.SetRedraw( TRUE );
}

void CPageTable::OnBnClickedButtonAdd()
{
	m_pSheetTab->ActiveActionPage ( NULL );
}

void CPageTable::OnBnClickedButtonEdit()
{
	POSITION	pos = m_ListTalk.GetFirstSelectedItemPosition ();
	int nItem = m_ListTalk.GetNextSelectedItem ( pos );	
	if ( nItem == -1 )	return ;

	DWORD NativeID = (DWORD) m_ListTalk.GetItemData ( nItem );	
	m_pSheetTab->ActiveActionPage ( GetDialogueSet()->GetDialogue ( NativeID ) );
}

void CPageTable::OnBnClickedButtonCopy()
{
	POSITION	pos = m_ListTalk.GetFirstSelectedItemPosition ();
	int nItem = m_ListTalk.GetNextSelectedItem ( pos );	
	if ( nItem == -1 )	return ;

	DWORD dwNativeID = (DWORD) m_ListTalk.GetItemData ( nItem );
	CNpcDialogue* pDialogue = GetDialogueSet()->GetDialogue ( dwNativeID );
	if ( !pDialogue )	return;

	DWORD dwNID = GetDialogueSet()->MakeDlgNID ();

	CNpcDialogue* pNewDialogue = new CNpcDialogue;
	*pNewDialogue = *pDialogue;
	pNewDialogue->SetDlgNID ( dwNID );

	GetDialogueSet()->AddDialogue ( pNewDialogue );
	UpdateItems ();
}

void CPageTable::OnBnClickedButtonDelete()
{
	POSITION	pos = m_ListTalk.GetFirstSelectedItemPosition ();
	int nItem = m_ListTalk.GetNextSelectedItem ( pos );	
	if ( nItem == -1 )	return ;

	DWORD NativeID = (DWORD) m_ListTalk.GetItemData ( nItem );

	CNpcDialogue* pDialogue = GetDialogueSet()->GetDialogue ( NativeID );
	DWORD dwIndex = pDialogue->FindNonCondition();
	if ( dwIndex==UINT_MAX )	dwIndex = 0;

	CString strBasicTalk;
	CNpcDialogueCase* pDlgCase = pDialogue->GetDlgCase(dwIndex);
	if ( pDlgCase )		strBasicTalk = pDlgCase->GetBasicTalk().c_str();

	CString strTemp;
	strTemp.Format ( "Do you want to delete?\n[%03d] %s", NativeID, strBasicTalk );

	if ( MessageBox ( strTemp, "Question", MB_YESNO | MB_ICONASTERISK ) == IDYES )
	{
		GetDialogueSet()->DelDialogue ( NativeID );
		UpdateItems ();
	}
}

void CPageTable::OnNMDblclkListTalk(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if ( pNMItemActivate->iItem == -1 )
		return ;

	DWORD NativeID = (DWORD) m_ListTalk.GetItemData ( pNMItemActivate->iItem );	
	m_pSheetTab->ActiveActionPage ( GetDialogueSet()->GetDialogue ( NativeID ) );
}
