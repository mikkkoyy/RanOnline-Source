// DlgMobGen.cpp : implementation file
//

#include "stdafx.h"
#include "EditorQuest.h"
#include "DlgMobGen.h"

#include "EtcFunction.h"
#include "GLItemMan.h"
#include "GLCrowData.h"

#include "DlgItem.h"
#include "DlgCrow.h"
// CDlgMobGen dialog

IMPLEMENT_DYNAMIC(CDlgMobGen, CDialog)

CDlgMobGen::CDlgMobGen(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgMobGen::IDD, pParent)
{

}

CDlgMobGen::~CDlgMobGen()
{
}

void CDlgMobGen::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_GEN_MOB, m_listboxGENMOB);
}


BEGIN_MESSAGE_MAP(CDlgMobGen, CDialog)
	ON_EN_CHANGE(IDC_EDIT_GENITEM_MID, OnEnChangeEditGenitemMid)
	ON_EN_CHANGE(IDC_EDIT_GENITEM_SID, OnEnChangeEditGenitemSid)
	ON_BN_CLICKED(IDC_BUTTON_GENITEM, OnBnClickedButtonGenitem)
	ON_BN_CLICKED(IDC_BUTTON_GEN_MOB_ADD, OnBnClickedButtonGenMobAdd)
	ON_BN_CLICKED(IDC_BUTTON_GEN_MOB_DEL, OnBnClickedButtonGenMobDel)
	ON_BN_CLICKED(IDC_BUTTON_GEN_MOB_INSERT, OnBnClickedButtonGenMobInsert)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgMobGen message handlers
BOOL CDlgMobGen::OnInitDialog()
{
	CDialog::OnInitDialog();

	UpdateDlgItem();

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CDlgMobGen::UpdateDlgItem ()
{
	CString strGENITEM_NAME;

	SetWin_Num_int( this, IDC_EDIT_GENITEM_MID, m_sGENQITEM.sNID.wMainID );
	SetWin_Num_int( this, IDC_EDIT_GENITEM_SID, m_sGENQITEM.sNID.wSubID );
	SetWin_Num_int ( this, IDC_EDIT_GEN_NUM, m_sGENQITEM.wNUM );
	SetWin_Num_float ( this, IDC_EDIT_GEN_RATE, m_sGENQITEM.fGEN_RATE );

	ShowItem ();
	ShowCrowList();
}

void CDlgMobGen::ShowItem ()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_GENITEM_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_GENITEM_SID );

	std::string strNAME = "Unknown Item";
	SITEM* pITEM = GLItemMan::GetInstance().GetItem ( wMID, wSID );	
	if ( pITEM )	strNAME = pITEM->GetName();
	SetWin_Text( this, IDC_EDIT_GENITEM, strNAME.c_str() );
}

void CDlgMobGen::ShowCrowList ()
{
	m_listboxGENMOB.ResetContent();
	m_listboxGENMOB.SetRedraw( FALSE );

	DWQARRAY_ITER iter = m_sGENQITEM.vecGEN_MOB.begin();
	DWQARRAY_ITER iter_end = m_sGENQITEM.vecGEN_MOB.end();
	for ( ; iter!=iter_end; ++iter )
	{
		const SNATIVEID sMOBID((*iter));
		PCROWDATA pCROW = GLCrowDataMan::GetInstance().GetCrowData ( sMOBID );

		CString strNAME;
		strNAME.Format ( "[%d/%d] %s", sMOBID.wMainID, sMOBID.wSubID, pCROW->GetName() );
		int nIndex = m_listboxGENMOB.AddString ( strNAME );

		DWORD dwData = DWORD(iter-m_sGENQITEM.vecGEN_MOB.begin());
		m_listboxGENMOB.SetItemData ( nIndex, dwData );
	}

	m_listboxGENMOB.SetRedraw( TRUE );

	SetWin_Num_int( this, IDC_EDIT_INSERT_MID, SNATIVEID( false ).wMainID );
	SetWin_Num_int( this, IDC_EDIT_INSERT_SID, SNATIVEID( false ).wSubID );
}

void CDlgMobGen::OnEnChangeEditGenitemMid()
{
	ShowItem ();
}

void CDlgMobGen::OnEnChangeEditGenitemSid()
{
	ShowItem ();
}

void CDlgMobGen::OnBnClickedButtonGenitem()
{
	CDlgItem dlg;
	if ( dlg.DoModal() == IDOK  )
	{
		SITEM* pITEM = GLItemMan::GetInstance().GetItem ( dlg.m_SELECTED );	
		if ( pITEM )
		{
			SetWin_Num_int( this, IDC_EDIT_GENITEM_MID, pITEM->sBasicOp.sNativeID.wMainID );
			SetWin_Num_int( this, IDC_EDIT_GENITEM_SID, pITEM->sBasicOp.sNativeID.wSubID );
			ShowItem();
		}
	}
}

void CDlgMobGen::OnBnClickedButtonGenMobAdd()
{
	CDlgCrow dlg;
	if ( dlg.DoModal() == IDOK  )
	{
		SCROWDATA* pCROW = GLCrowDataMan::GetInstance().GetCrowData( dlg.m_SELECTED );	
		if ( pCROW )
		{
			m_sGENQITEM.vecGEN_MOB.push_back( pCROW->m_sBasic.sNativeID.dwID );
			ShowCrowList();
		}
	}
}

void CDlgMobGen::OnBnClickedButtonGenMobDel()
{
	int nIndex = m_listboxGENMOB.GetCurSel ();
	if ( LB_ERR == nIndex )		return;

	DWORD dwDATA = (DWORD) m_listboxGENMOB.GetItemData ( nIndex );
	m_sGENQITEM.GENMOB_ERASE ( dwDATA );
	ShowCrowList();
}

void CDlgMobGen::OnBnClickedButtonGenMobInsert()
{
	WORD wMID = GetWin_Num_int( this, IDC_EDIT_INSERT_MID );
	WORD wSID = GetWin_Num_int( this, IDC_EDIT_INSERT_SID );

	SCROWDATA* pCROW = GLCrowDataMan::GetInstance().GetCrowData( wMID, wSID );	
	if ( pCROW )
	{
		m_sGENQITEM.vecGEN_MOB.push_back( pCROW->m_sBasic.sNativeID.dwID );
		ShowCrowList();
	}else{
		MessageBox( "Invalid Crow" );
	}
}

void CDlgMobGen::OnBnClickedOk()
{
	m_sGENQITEM.sNID.wMainID = GetWin_Num_int ( this, IDC_EDIT_GENITEM_MID );
	m_sGENQITEM.sNID.wSubID = GetWin_Num_int ( this, IDC_EDIT_GENITEM_SID );
	m_sGENQITEM.wNUM = GetWin_Num_int ( this, IDC_EDIT_GEN_NUM );
	m_sGENQITEM.fGEN_RATE = GetWin_Num_float ( this, IDC_EDIT_GEN_RATE );

	OnOK();
}

void CDlgMobGen::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}
