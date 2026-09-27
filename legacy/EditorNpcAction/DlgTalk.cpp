// DlgTalk.cpp : implementation file
//

#include "stdafx.h"
#include "EditorNpcAction.h"
#include "DlgTalk.h"
#include "NpcDialogue.h"
#include "EtcFunction.h"

// CDlgTalk dialog

IMPLEMENT_DYNAMIC(CDlgTalk, CDialog)

CDlgTalk::CDlgTalk(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgTalk::IDD, pParent)
	, m_dwNpcID( UINT_MAX )
	, m_pNpcDialogueSet ( NULL ) 
{

}

CDlgTalk::CDlgTalk( CNpcDialogueSet* pNpcDialogueSet, CWnd* pParent /*=NULL*/)
	: CDialog(CDlgTalk::IDD, pParent)
	, m_dwNpcID( UINT_MAX )
	, m_pNpcDialogueSet ( pNpcDialogueSet ) 
{
}


CDlgTalk::~CDlgTalk()
{
}

void CDlgTalk::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SELECT, m_listNpcTalk );
}


BEGIN_MESSAGE_MAP(CDlgTalk, CDialog)
	ON_LBN_SELCHANGE(IDC_LIST_SELECT, OnLbnSelchangeListSelect)
END_MESSAGE_MAP()


// CDlgTalk message handlers
BOOL CDlgTalk::OnInitDialog()
{
	CDialog::OnInitDialog();

	if ( m_pNpcDialogueSet )
	{
		CNpcDialogueSet::DIALOGMAP* pMap = m_pNpcDialogueSet->GetDialogueMap ();
		CNpcDialogueSet::DIALOGMAP_IT iter = pMap->begin();
		CNpcDialogueSet::DIALOGMAP_IT iter_end = pMap->end();		

		CString strTemp;

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

			strTemp.Format ( "[%3d] %s", pDialogue->GetDlgNID(), pDlgCase->GetBasicTalk().c_str() );
			int nItem = m_listNpcTalk.AddString( strTemp );
			m_listNpcTalk.SetItemData ( nItem, pDialogue->GetDlgNID() );
		}
	}
	return TRUE; 
}


void CDlgTalk::OnLbnSelchangeListSelect()
{
	CString strTEXT;

	int nIndex = m_listNpcTalk.GetCurSel();		
	if ( nIndex >= 0 && nIndex < m_listNpcTalk.GetCount() )
	{
		m_dwNpcID = m_listNpcTalk.GetItemData( nIndex );
		m_listNpcTalk.GetText( nIndex, strTEXT);
	}
	else
	{
		strTEXT = "";
		m_dwNpcID = UINT_MAX;
	}

	SetWin_Num_int( this, IDC_EDIT_SEL_ID, m_dwNpcID );
	SetWin_Text( this, IDC_EDIT_SEL_NAME, strTEXT.GetString() );
}
