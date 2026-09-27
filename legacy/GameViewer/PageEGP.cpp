// PageEGP.cpp : implementation file
//

#include "stdafx.h"
#include "GameViewer.h"
#include "PageEGP.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "GameViewerView.h"

// CPageEGP dialog

IMPLEMENT_DYNAMIC(CPageEGP, CPropertyPage)

CPageEGP::CPageEGP(LOGFONT logfont)
	: CPropertyPage(CPageEGP::IDD)
	, m_pFont ( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageEGP::~CPageEGP()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEGP::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_EFFECTS, m_List_Effect );
}


BEGIN_MESSAGE_MAP(CPageEGP, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_EGP_LOAD, OnBnClickedButtonEgpLoad)
	ON_BN_CLICKED(IDC_BUTTON_EGP_RELOAD, OnBnClickedButtonEgpReload)
	ON_BN_CLICKED(IDC_BUTTON_EGP_PLAY, OnBnClickedButtonEgpPlay)
	ON_BN_CLICKED(IDC_BUTTON_EGP_SAVE, OnBnClickedButtonEgpSave)
END_MESSAGE_MAP()


// CPageEGP message handlers
BOOL CPageEGP::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_List_Effect.SetExtendedStyle ( m_List_Effect.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List_Effect.GetClientRect ( &rectCtrl );
	m_List_Effect.InsertColumn( 0, "Effects", LVCFMT_LEFT, ( 100* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	return TRUE;
}

void CPageEGP::LoadFile( std::string strFile )
{
	if ( strFile.size() <= 0 )	return;

	BOOL bLOAD = CGameViewerView::GetView()->DataLoadEGP( strFile.c_str() );
	if ( bLOAD )
	{
		DataShow();
	}else{
		CDebugSet::MsgBox( "Error Load File:%s", strFile.c_str() );
		DataReset();
	}
}

void CPageEGP::DataReset()
{
	SetWin_Text( this, IDC_EDIT_FILENAME, "" );
	m_List_Effect.DeleteAllItems();

	SetWin_Check( this, IDC_CHECK_FLAG_PLAY, FALSE );
	SetWin_Check( this, IDC_CHECK_FLAG_DONE, FALSE );
	SetWin_Check( this, IDC_CHECK_FLAG_END, FALSE );
	SetWin_Check( this, IDC_CHECK_FLAG_RENDPASS, FALSE );
	SetWin_Check( this, IDC_CHECK_FLAG_NEVERDIE, FALSE );
	SetWin_Check( this, IDC_CHECK_FLAG_EVENT, FALSE );
	SetWin_Check( this, IDC_CHECK_FLAG_AUTOAABBBOX, FALSE );
	SetWin_Check( this, IDC_CHECK_FLAG_NONE, FALSE );
	SetWin_Check( this, IDC_CHECK_FLAG_TARGETPOS, FALSE );
	SetWin_Check( this, IDC_CHECK_FLAG_TARGETID, FALSE );
	SetWin_Check( this, IDC_CHECK_FLAG_MOVE, FALSE );
	SetWin_Check( this, IDC_CHECK_FLAG_TARGETLINE, FALSE );
	SetWin_Check( this, IDC_CHECK_FLAG_AVERAGE, FALSE );
	SetWin_Check( this, IDC_CHECK_FLAG_SLOWFAST, FALSE );
	SetWin_Check( this, IDC_CHECK_FLAG_FASTSLOW, FALSE );
	SetWin_Check( this, IDC_CHECK_FLAG_ACCELERATE, FALSE );
}

void CPageEGP::DataShow()
{
	DxEffSingleGroup* pData = CGameViewerView::GetView()->m_pEffSingle;
	if ( !pData )	return;

	SetWin_Text( this, IDC_EDIT_FILENAME, pData->m_strFileName.c_str() );
	m_List_Effect.DeleteAllItems();

	SetWin_Check( this, IDC_CHECK_FLAG_PLAY, pData->m_dwFlag&EFF_PLY_PLAY );
	SetWin_Check( this, IDC_CHECK_FLAG_DONE, pData->m_dwFlag&EFF_PLY_DONE );
	SetWin_Check( this, IDC_CHECK_FLAG_END, pData->m_dwFlag&EFF_PLY_END );
	SetWin_Check( this, IDC_CHECK_FLAG_RENDPASS, pData->m_dwFlag&EFF_PLY_RENDPASS );
	SetWin_Check( this, IDC_CHECK_FLAG_NEVERDIE, pData->m_dwFlag&EFF_CFG_NEVERDIE );
	SetWin_Check( this, IDC_CHECK_FLAG_EVENT, pData->m_dwFlag&EFF_CFG_EVENT );
	SetWin_Check( this, IDC_CHECK_FLAG_AUTOAABBBOX, pData->m_dwFlag&EFF_CFG_AUTOAABBBOX );
	SetWin_Check( this, IDC_CHECK_FLAG_NONE, pData->m_dwFlag&EFF_CFG_NONE );
	SetWin_Check( this, IDC_CHECK_FLAG_TARGETPOS, pData->m_dwFlag&EFF_CFG_TARGETPOS );
	SetWin_Check( this, IDC_CHECK_FLAG_TARGETID, pData->m_dwFlag&EFF_CFG_TARGETID );
	SetWin_Check( this, IDC_CHECK_FLAG_MOVE, pData->m_dwFlag&EFF_CFG_MOVE );
	SetWin_Check( this, IDC_CHECK_FLAG_TARGETLINE, pData->m_dwFlag&EFF_CFG_TARGETLINE );
	SetWin_Check( this, IDC_CHECK_FLAG_AVERAGE, pData->m_dwFlag&EFF_CFG_AVERAGE );
	SetWin_Check( this, IDC_CHECK_FLAG_SLOWFAST, pData->m_dwFlag&EFF_CFG_SLOWFAST );
	SetWin_Check( this, IDC_CHECK_FLAG_FASTSLOW, pData->m_dwFlag&EFF_CFG_FASTSLOW );
	SetWin_Check( this, IDC_CHECK_FLAG_ACCELERATE, pData->m_dwFlag&EFF_CFG_ACCEL );

	m_List_Effect.SetRedraw( FALSE );

	if ( pData->m_pEffSingleRoot )
	{
		ShowEffSingle( 0, pData->m_pEffSingleRoot );
	}

	m_List_Effect.SetRedraw( TRUE );
}

int CPageEGP::ShowEffSingle( int nIndex, DxEffSingle* pSingle )
{
	if ( !pSingle )	return 0;

	nIndex = m_List_Effect.InsertItem( nIndex, pSingle->GetName() );

	if ( pSingle->m_pChild )
		nIndex = ShowEffSingle( nIndex, pSingle->m_pChild );

	if ( pSingle->m_pSibling )
		nIndex = ShowEffSingle( nIndex, pSingle->m_pSibling );

	return nIndex;
}

void CPageEGP::OnBnClickedButtonEgpLoad()
{
	CString szFilter = "Effect (*.egp)|*.egp;|";
	CFileDialog dlg ( TRUE,".EGP", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );

	dlg.m_ofn.lpstrInitialDir = DxEffSinglePropGMan::GetInstance().GetPath();
	if ( dlg.DoModal() == IDOK )
	{
		LoadFile( dlg.GetFileName().GetString() );
	}
}

void CPageEGP::OnBnClickedButtonEgpReload()
{
	DxEffSingleGroup* pData = CGameViewerView::GetView()->m_pEffSingle;
	if ( !pData )	return;

	LoadFile( pData->m_strFileName.c_str() );
}

void CPageEGP::OnBnClickedButtonEgpPlay()
{
	DxEffSingleGroup* pData = CGameViewerView::GetView()->m_pEffSingle;
	if ( !pData )	return;

	pData->ReStartEff();
}

void CPageEGP::OnBnClickedButtonEgpSave()
{
	CString szFilter = "Effect (*.egp)|*.egp;|";
	CFileDialog dlg ( FALSE,".EGP", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );

	dlg.m_ofn.lpstrInitialDir = DxEffSinglePropGMan::GetInstance().GetPath();
	if ( dlg.DoModal() == IDOK )
	{
		BOOL bSAVE = CGameViewerView::GetView()->DataSaveEGP( dlg.GetPathName().GetString() );
		if ( bSAVE )
		{
			LoadFile( dlg.GetFileName().GetString() );
		}else{
			CDebugSet::MsgBox( "Error Save File: %s", dlg.GetPathName().GetString() );
		}
	}
}
