// DlgCodexList.cpp : implementation file
//

#include "stdafx.h"
#include "GMTool.h"
#include "DlgCodexList.h"
#include "GLCodex.h"
#include "GLCodexData.h"
#include "EtcFunction.h"

// CDlgCodexList dialog

IMPLEMENT_DYNAMIC(CDlgCodexList, CDialog)

CDlgCodexList::CDlgCodexList(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgCodexList::IDD, pParent)
	, m_dwSelected(UINT_MAX)
{

}

CDlgCodexList::~CDlgCodexList()
{
}

void CDlgCodexList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SELECT, m_listSELECT );
}


BEGIN_MESSAGE_MAP(CDlgCodexList, CDialog)
	ON_LBN_SELCHANGE(IDC_LIST_SELECT, OnLbnSelchangeListSelect)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgCodexList message handlers
BOOL CDlgCodexList::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_listSELECT.ResetContent();

	SCODEX_FILE_DATA_MAP &mapdata = GLCodex::GetInstance().m_mapCodex;
	for( SCODEX_FILE_DATA_MAP_ITER iter = mapdata.begin(); 
		iter != mapdata.end(); ++ iter )
	{
		const SCODEX_FILE_DATA &sdata = (*iter).second;
		CString strNAME;
		if ( sdata.bRewardBadge )
			strNAME.Format ( "[%03d] %s~%s", sdata.dwCodexID, sdata.strCodexTitle.c_str(), sdata.strBadgeString.c_str() );
		else
			strNAME.Format ( "[%03d] %s", sdata.dwCodexID, sdata.strCodexTitle.c_str() );

		int nIndex = m_listSELECT.AddString ( strNAME );
		m_listSELECT.SetItemData ( nIndex, sdata.dwCodexID );
	}


	m_dwSelected = UINT_MAX;
	SetWin_Text( this, IDC_EDIT_SEL_NAME, "" );

	return TRUE;
}

void CDlgCodexList::OnLbnSelchangeListSelect()
{
	int nSel = m_listSELECT.GetCurSel();
	if ( nSel < 0 ){
		return;
	}

	DWORD dwData = (DWORD) m_listSELECT.GetItemData ( nSel );

	SCODEX_FILE_DATA* pCodex = GLCodex::GetInstance().GetCodex( dwData );
	if ( pCodex )
	{
		m_dwSelected = pCodex->dwCodexID;

		CString strNAME;
		strNAME.Format ( "[%03d] %s", pCodex->dwCodexID, pCodex->strCodexTitle.c_str() );
		SetWin_Text( this, IDC_EDIT_SEL_NAME, strNAME.GetString() );
	}
	else
	{
		m_dwSelected = UINT_MAX;
		SetWin_Text( this, IDC_EDIT_SEL_NAME, "" );
	}
}

void CDlgCodexList::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}
