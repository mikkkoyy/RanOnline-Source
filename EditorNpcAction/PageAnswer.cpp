// PageAnswer.cpp : implementation file
//

#include "stdafx.h"
#include "EditorNpcAction.h"
#include "PageAnswer.h"

#include "SheetWithTab.h"
#include "EtcFunction.h"

#include "NpcTalkControl.h"
#include "NpcTalk.h"

// CPageAnswer dialog

IMPLEMENT_DYNAMIC(CPageAnswer, CPropertyPage)

CPageAnswer::CPageAnswer( LOGFONT logfont )
	: CPropertyPage(CPageAnswer::IDD)
	, m_pFont ( NULL )
	, m_pOrgDialogueCase(NULL)
	, m_pNpcDialogueSet(NULL)
{
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageAnswer::~CPageAnswer()
{
	SAFE_DELETE ( m_pFont );
}

void CPageAnswer::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ANSWERS, m_ctrlAnswer);
}


BEGIN_MESSAGE_MAP(CPageAnswer, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_OK, OnBnClickedButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnBnClickedButtonCancel)
	ON_BN_CLICKED(IDC_BUTTON_ANSWER_ADD, OnBnClickedButtonAnswerAdd)
	ON_BN_CLICKED(IDC_BUTTON_ANSWER_EDIT, OnBnClickedButtonAnswerEdit)
	ON_BN_CLICKED(IDC_BUTTON_ANSWER_COPY, OnBnClickedButtonAnswerCopy)
	ON_BN_CLICKED(IDC_BUTTON_ANSWER_DELETE, OnBnClickedButtonAnswerDelete)
	ON_BN_CLICKED(IDC_BUTTON_ANSWER_DELETEALL, OnBnClickedButtonAnswerDeleteall)
	ON_BN_CLICKED(IDC_BUTTON_ANSWER_UP, OnBnClickedButtonAnswerUp)
	ON_BN_CLICKED(IDC_BUTTON_ANSWER_DOWN, OnBnClickedButtonAnswerDown)
	ON_BN_CLICKED(IDC_BUTTON_CONDITION_EDIT, OnBnClickedButtonConditionEdit)
	ON_BN_CLICKED(IDC_BUTTON_CONDITION_DELETE, OnBnClickedButtonConditionDelete)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_ANSWERS, OnNMDblclkListAnswers)
END_MESSAGE_MAP()


// CPageAnswer message handlers
BOOL CPageAnswer::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_ctrlAnswer.SetExtendedStyle ( m_ctrlAnswer.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

	RECT	rect;
	m_ctrlAnswer.GetClientRect ( &rect );
	LONG	Width = rect.right - rect.left;

	LV_COLUMN	lvcolumn;
	const int	ColumnCount = 4;	
	char*		ColumnName[ColumnCount] = { "ID", "Answer", "Action Type", "Action Data" };
	int			ColumnWidthPercent[ColumnCount] = { 5, 45, 15, 35 };

	for ( int i = 0; i < ColumnCount; i++ )
	{		
		lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM
			| LVCF_TEXT | LVCF_WIDTH;// | LVCF_IMAGE;
		lvcolumn.fmt = LVCFMT_LEFT;
		lvcolumn.pszText = ColumnName[i];
		lvcolumn.iSubItem = i;
		lvcolumn.cx = ( Width * ColumnWidthPercent[i] ) / 100;
		m_ctrlAnswer.InsertColumn (i, &lvcolumn);		
	}


	return TRUE;  
}

void CPageAnswer::SetDialogueCase ( CNpcDialogueSet *pNpcDialogueSet, CNpcDialogueCase* pDialogueCase )
{	
	assert ( pDialogueCase && "CEditorNpcActionAnswer::SetDialogueCase () Invalid pDialogueCase Pointer." );	

	m_DummyTalkControl.RemoveTalk ();
	m_DummyDialogueCase.Reset ();

	m_pNpcDialogueSet = pNpcDialogueSet;
	m_pOrgDialogueCase = pDialogueCase;
	m_DummyDialogueCase = *pDialogueCase;

	if ( m_DummyDialogueCase.GetTalkControl () )
	{
		m_DummyTalkControl = *(m_DummyDialogueCase.GetTalkControl ());
	}		

	DataShow ();
}

void CPageAnswer::OnBnClickedButtonOk()
{
	DataSave ();

	if ( m_pOrgDialogueCase )	*m_pOrgDialogueCase = m_DummyDialogueCase;

	m_pSheetTab->SetActivePage ( PAGE_ACTION );
	m_pSheetTab->m_PageAction.UpdateList();
}

void CPageAnswer::OnBnClickedButtonCancel()
{
	m_pSheetTab->SetActivePage ( PAGE_ACTION );
}

void CPageAnswer::DataShow ()
{
	std::string strCondition;
	SNpcTalkCondition* pCondition = m_DummyDialogueCase.GetCondition();
	if ( pCondition )	pCondition->GetConditionText( strCondition );

	SetWin_Text ( this, IDC_EDIT_STATEMENT, m_DummyDialogueCase.GetBasicTalk().c_str() );
	SetWin_Text ( this, IDC_EDIT_CONDITION, strCondition.c_str() );

	m_ctrlAnswer.DeleteAllItems ();
	m_ctrlAnswer.SetRedraw( FALSE );

	TCHAR szTemp[32] = {0};

	CNpcTalkControl::NPCMAP* pMap = m_DummyTalkControl.GetTalkMap ();

	CNpcTalkControl::NPCMAP_IT iter = pMap->begin();
	CNpcTalkControl::NPCMAP_IT iter_end = pMap->end();	
	for ( int nIndex = 0; iter!=iter_end; ++iter, nIndex++ )
	{
		SNpcTalk* pNpcTalk = (*iter).second;
		_snprintf( szTemp, 32, "%u", pNpcTalk->m_dwNID );

		int nItem = m_ctrlAnswer.InsertItem ( nIndex, szTemp );		
		m_ctrlAnswer.SetItemText ( nIndex, 1, pNpcTalk->GetTalk() );
		m_ctrlAnswer.SetItemText ( nIndex, 2, SNpcTalk::strACTIONTYPE[pNpcTalk->m_nACTION].c_str() );


		std::string strACT_NO;
		switch ( pNpcTalk->m_nACTION )
		{
		case SNpcTalk::EM_PAGE_MOVE:
			m_pNpcDialogueSet->GetDlgText( pNpcTalk->m_dwACTION_NO, strACT_NO );
			break;

		case SNpcTalk::EM_BASIC:
			strACT_NO =  SNpcTalk::szBASICDESC[pNpcTalk->m_dwACTION_NO].c_str();
			break;

		case SNpcTalk::EM_QUEST_START:
			m_pNpcDialogueSet->GetDlgText( pNpcTalk->m_dwACTION_NO, strACT_NO );
			break;

		case SNpcTalk::EM_QUEST_STEP:
			m_pNpcDialogueSet->GetDlgText( pNpcTalk->m_dwACTION_NO, strACT_NO );
			break;

		default:
			_snprintf( szTemp, 32, "%u", pNpcTalk->m_dwACTION_NO );
			strACT_NO = szTemp;
			break;
		};

		m_ctrlAnswer.SetItemText ( nIndex, 3, strACT_NO.c_str() );
		m_ctrlAnswer.SetItemData ( nItem, (DWORD_PTR)pNpcTalk->m_dwNID );
	}

	m_ctrlAnswer.SetRedraw( TRUE );
}

void CPageAnswer::DataSave ()
{
	CString strTalk = GetWin_Text ( this, IDC_EDIT_STATEMENT );

	m_DummyDialogueCase.SetBasicTalk ( strTalk.GetString() );
	m_DummyDialogueCase.ResetTalkControl ();

	if ( m_DummyTalkControl.GetTalkMap()->size () )
		m_DummyDialogueCase.SetTalkControl ( m_DummyTalkControl );
}

void CPageAnswer::OnBnClickedButtonAnswerAdd()
{
	DataSave ();
	m_pSheetTab->ActiveActionTalk ( PAGE_ANSWER, m_pSheetTab->GetDialogueSet(), &m_DummyTalkControl, FALSE, NULL );
}

void CPageAnswer::OnBnClickedButtonAnswerEdit()
{
	DataSave ();
	POSITION	pos = m_ctrlAnswer.GetFirstSelectedItemPosition ();
	int nItem = m_ctrlAnswer.GetNextSelectedItem ( pos );	
	if ( nItem == -1 )
		return ;

	DWORD NativeID = (DWORD) m_ctrlAnswer.GetItemData ( nItem );

	m_pSheetTab->ActiveActionTalk ( PAGE_ANSWER, m_pSheetTab->GetDialogueSet(), &m_DummyTalkControl, TRUE, m_DummyTalkControl.GetTalk(NativeID) );
}

void CPageAnswer::OnBnClickedButtonAnswerCopy()
{
	POSITION	pos = m_ctrlAnswer.GetFirstSelectedItemPosition ();
	int nItem = m_ctrlAnswer.GetNextSelectedItem ( pos );	
	if ( nItem == -1 )
		return ;

	DWORD NativeID = (DWORD) m_ctrlAnswer.GetItemData ( nItem );

	SNpcTalk* pTalk = m_DummyTalkControl.GetTalk(NativeID);
	SNpcTalk* pNewTalk = new SNpcTalk;
	*pNewTalk = *pTalk;
	pNewTalk->m_dwNID = m_DummyTalkControl.MakeTalkNID ();
	m_DummyTalkControl.AddTalk( pNewTalk );

	DataShow ();
}

void CPageAnswer::OnBnClickedButtonAnswerDelete()
{
	POSITION	pos = m_ctrlAnswer.GetFirstSelectedItemPosition ();
	int nItem = m_ctrlAnswer.GetNextSelectedItem ( pos );	
	if ( nItem == -1 )
		return;

	DWORD NativeID = (DWORD) m_ctrlAnswer.GetItemData ( nItem );
	CString BasicTalk = m_DummyTalkControl.GetTalk ( NativeID )->GetTalk();
	CString strTemp;
	strTemp.Format ( "Do you want to delete?\n[%03d] %s", NativeID, BasicTalk );

	if ( MessageBox ( strTemp, "Question", MB_YESNO | MB_ICONASTERISK ) == IDYES )
	{
		m_DummyTalkControl.DelTalk ( NativeID );
		DataShow ();
	}
}

void CPageAnswer::OnBnClickedButtonAnswerDeleteall()
{
	if ( MessageBox ( "Do you want to delete all?", "Question", MB_YESNO | MB_ICONASTERISK ) == IDYES )
	{
		m_DummyTalkControl.RemoveTalk ();
		DataShow ();
	}
}

void CPageAnswer::OnBnClickedButtonAnswerUp()
{
	POSITION	pos = m_ctrlAnswer.GetFirstSelectedItemPosition ();
	int nItem = m_ctrlAnswer.GetNextSelectedItem ( pos );	
	if ( nItem == -1 )
		return;

	DWORD dwNID = (DWORD) m_ctrlAnswer.GetItemData ( nItem );
	DWORD dwNID_NEW = m_DummyTalkControl.ToUp ( dwNID );

	if( UINT_MAX != dwNID_NEW ) 
	{
		DataShow ();
		m_ctrlAnswer.SetItemState( --nItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED ); 
	}
}

void CPageAnswer::OnBnClickedButtonAnswerDown()
{
	POSITION	pos = m_ctrlAnswer.GetFirstSelectedItemPosition ();
	int nItem = m_ctrlAnswer.GetNextSelectedItem ( pos );	
	if ( nItem == -1 )
		return;

	DWORD dwNID = (DWORD) m_ctrlAnswer.GetItemData ( nItem );
	DWORD dwNID_NEW = m_DummyTalkControl.ToDown ( dwNID );

	if( UINT_MAX != dwNID_NEW ) 
	{
		DataShow ();
		m_ctrlAnswer.SetItemState( ++nItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED ); 
	}
}

void CPageAnswer::OnBnClickedButtonConditionEdit()
{
	DataSave ();

	SNpcTalkCondition* pConditon = m_DummyDialogueCase.GetCondition();
	if ( !pConditon )
		m_DummyDialogueCase.SetCondition ( SNpcTalkCondition() );

	pConditon = m_DummyDialogueCase.GetCondition();
	m_pSheetTab->ActiveActionCondition ( PAGE_ANSWER, pConditon );
}

void CPageAnswer::OnBnClickedButtonConditionDelete()
{
	if ( MessageBox ( "Do you want to delete?", "Question", MB_YESNO | MB_ICONASTERISK ) == IDYES )
	{
		m_DummyDialogueCase.ResetCondition();
		DataShow ();
	}
}

void CPageAnswer::OnNMDblclkListAnswers(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if ( pNMItemActivate->iItem == -1 )
		return ;

	DWORD NativeID = (DWORD) m_ctrlAnswer.GetItemData ( pNMItemActivate->iItem );

	m_pSheetTab->ActiveActionTalk ( PAGE_ANSWER, m_pSheetTab->GetDialogueSet(), &m_DummyTalkControl, TRUE, m_DummyTalkControl.GetTalk(NativeID) );
}
