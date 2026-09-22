// DlgActivityList.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgActivityList.h"
#include "GLActivity.h"
#include "GLActivityData.h"
#include "EtcFunction.h"

// CDlgActivityList dialog

IMPLEMENT_DYNAMIC(CDlgActivityList, CDialog)

CDlgActivityList::CDlgActivityList(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgActivityList::IDD, pParent)
	, m_dwSelected(UINT_MAX)
{

}

CDlgActivityList::~CDlgActivityList()
{
}

void CDlgActivityList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SELECT, m_listSELECT );
}


BEGIN_MESSAGE_MAP(CDlgActivityList, CDialog)
	ON_LBN_SELCHANGE(IDC_LIST_SELECT, OnLbnSelchangeListSelect)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgActivityList message handlers
BOOL CDlgActivityList::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_listSELECT.ResetContent();

	SACTIVITY_FILE_DATA_MAP &mapdata = GLActivity::GetInstance().m_mapActivity;
	for( SACTIVITY_FILE_DATA_MAP_ITER iter = mapdata.begin(); 
		iter != mapdata.end(); ++ iter )
	{
		const SACTIVITY_FILE_DATA &sdata = (*iter).second;
		CString strNAME;
		if ( sdata.bRewardBadge )
			strNAME.Format ( "[%03d] %s~%s", sdata.dwActivityID, sdata.strActivityTitle.c_str(), sdata.strBadgeString.c_str() );
		else
			strNAME.Format ( "[%03d] %s", sdata.dwActivityID, sdata.strActivityTitle.c_str() );

		int nIndex = m_listSELECT.AddString ( strNAME );
		m_listSELECT.SetItemData ( nIndex, sdata.dwActivityID );
	}


	m_dwSelected = UINT_MAX;
	SetWin_Text( this, IDC_EDIT_SEL_NAME, "" );

	return TRUE;
}

void CDlgActivityList::OnLbnSelchangeListSelect()
{
	int nSel = m_listSELECT.GetCurSel();
	if ( nSel < 0 ){
		return;
	}

	DWORD dwData = (DWORD) m_listSELECT.GetItemData ( nSel );

	SACTIVITY_FILE_DATA* pActivity = GLActivity::GetInstance().GetActivity( dwData );
	if ( pActivity )
	{
		m_dwSelected = pActivity->dwActivityID;

		CString strNAME;
		strNAME.Format ( "[%03d] %s", pActivity->dwActivityID, pActivity->strActivityTitle.c_str() );
		SetWin_Text( this, IDC_EDIT_SEL_NAME, strNAME.GetString() );
	}
	else
	{
		m_dwSelected = UINT_MAX;
		SetWin_Text( this, IDC_EDIT_SEL_NAME, "" );
	}
}

void CDlgActivityList::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}
