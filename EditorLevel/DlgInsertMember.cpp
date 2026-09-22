// DlgInsertMember.cpp : implementation file
//

#include "stdafx.h"
#include "EditorLevel.h"
#include "DlgInsertMember.h"
#include ".\dlginsertmember.h"


// CDlgInsertMember dialog

IMPLEMENT_DYNAMIC(CDlgInsertMember, CDialog)
CDlgInsertMember::CDlgInsertMember(CWnd* pParent /*=NULL*/, GLLandMan* pLand /*= NULL*/, GLMobSchedule* pSCHED /*= NULL*/)
	: CDialog(CDlgInsertMember::IDD, pParent)
	, m_pGLLand(NULL)
	, m_pSched(NULL)
{
	m_pGLLand = pLand;
	m_pSched = pSCHED;
}

CDlgInsertMember::~CDlgInsertMember()
{
}

void CDlgInsertMember::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_List);
}


BEGIN_MESSAGE_MAP(CDlgInsertMember, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgInsertMember message handlers
BOOL CDlgInsertMember::OnInitDialog()
{
	CDialog::OnInitDialog();

	RECT rectCtrl;
	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List.GetClientRect ( &rectCtrl );
	m_List.InsertColumn( 0, "Group", LVCFMT_LEFT, ( 100* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	ShowData();

	return TRUE;  
}

void CDlgInsertMember::ShowData()
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
			m_List.InsertItem( i, pCur->Data->m_strGroupName.c_str() );
			m_List.SetItemData( i, (DWORD_PTR)pCur->Data );

			i++;
		}

		pCur = pCur->pNext;
	}

	m_List.SetRedraw( TRUE );
}

void CDlgInsertMember::OnBnClickedOk()
{
	if ( m_pGLLand && m_pSched )
	{
		int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
		if( nSelect != -1 ) 
		{
			std::string strCrow = m_List.GetItemText( nSelect, 0 );

			if ( strCrow.size() )
			{
				if ( m_pSched->m_strGroupName.size() )
				{
					m_pGLLand->DEL_GROUPMEMBER( m_pSched->m_strGroupName.c_str(), m_pSched->m_dwGlobID );
					if( m_pSched->m_bLeaderMonster )
					{
						m_pGLLand->DEL_GROUPLEADER( m_pSched->m_strGroupName.c_str() );
					}
				}

				m_pSched->m_strGroupName = strCrow.c_str();
				m_pGLLand->ADD_GROUPMEMBER( strCrow.c_str(), m_pSched->m_dwGlobID );

				m_pGLLand->DropOutCrow( m_pSched->m_dwGlobID );
			}
			
		}	
	}

	OnOK();
}

void CDlgInsertMember::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}
