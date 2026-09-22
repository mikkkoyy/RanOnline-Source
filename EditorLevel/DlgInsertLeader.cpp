// DlgInsertLeader.cpp : implementation file
//

#include "stdafx.h"
#include "EditorLevel.h"
#include "DlgInsertLeader.h"
#include ".\dlginsertleader.h"
#include "EtcFunction.h"


// CDlgInsertLeader dialog

IMPLEMENT_DYNAMIC(CDlgInsertLeader, CDialog)
CDlgInsertLeader::CDlgInsertLeader(CWnd* pParent /*=NULL*/, GLLandMan* pLand /*= NULL*/, GLMobSchedule* pSCHED /*= NULL*/)
	: CDialog(CDlgInsertLeader::IDD, pParent)
	, m_pGLLand(NULL)
	, m_pSched(NULL)
{
	m_pGLLand = pLand;
	m_pSched = pSCHED;
}

CDlgInsertLeader::~CDlgInsertLeader()
{
}

void CDlgInsertLeader::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_List);
}


BEGIN_MESSAGE_MAP(CDlgInsertLeader, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgInsertLeader message handlers
BOOL CDlgInsertLeader::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	RECT rectCtrl;
	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List.GetClientRect ( &rectCtrl );
	m_List.InsertColumn( 0, "Name", LVCFMT_LEFT, ( 40* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List.InsertColumn( 1, "Group", LVCFMT_LEFT, ( 60* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	ShowData();

	return TRUE;  
}

void CDlgInsertLeader::ShowData()
{
	m_List.DeleteAllItems();

	if ( !m_pGLLand )	return;

	MOBSCHEDULELIST * GLMobSchList = m_pGLLand->GetMobSchMan()->GetMobSchList();
	SGLNODE<GLMobSchedule*>* pCur = GLMobSchList->m_pHead;

	m_List.SetRedraw( FALSE );

	int i = 0;
	while ( pCur )
	{
		if ( pCur->Data->m_bLeaderMonster )
		{
			m_List.InsertItem( i, pCur->Data->m_szName  );
			m_List.SetItemText( i, 1, pCur->Data->m_strGroupName.c_str() );
			m_List.SetItemData( i, (DWORD_PTR)pCur->Data );

			i++;
		}

		pCur = pCur->pNext;
	}

	m_List.SetRedraw( TRUE );
}

void CDlgInsertLeader::OnBnClickedOk()
{
	if ( !InsertLeader() )	return;

	OnOK();
}

void CDlgInsertLeader::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

BOOL CDlgInsertLeader::InsertLeader()
{
	if ( !m_pSched )	return FALSE;
	if ( !m_pGLLand )	return FALSE;

	std::string strName = GetWin_Text( this, IDC_EDIT_LEADER_INSERT );
	if ( strName.empty() )	return FALSE;

	PGLCROW pcrow_check = m_pGLLand->GET_GROUPLEADER( strName.c_str() );
	if ( pcrow_check )
	{
		CDebugSet::MsgBox( "%s already leader for group %s", pcrow_check->GetMobSchedule()->m_szName, strName.c_str() );
		return FALSE;
	}

	m_pSched->m_bLeaderMonster = TRUE;
	m_pSched->m_strGroupName = strName.c_str();

	m_pGLLand->ADD_GROUPMEMBER( strName.c_str(), m_pSched->m_dwGlobID );
	m_pGLLand->ADD_GROUPLEADER( strName.c_str(), m_pSched->m_dwGlobID );

	m_pGLLand->DropOutCrow( m_pSched->m_dwGlobID );

	ShowData();

	return TRUE;
}
