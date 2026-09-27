// PageABF.cpp : implementation file
//

#include "stdafx.h"
#include "GameViewer.h"
#include "GameViewerView.h"
#include "PageABF.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "DxEffCharData.h"

/*Item Self Effect, Juver, 2017/06/09 */
#include "DxEffSelfEffect.h"

// CPageABF dialog

IMPLEMENT_DYNAMIC(CPageABF, CPropertyPage)

CPageABF::CPageABF(LOGFONT logfont)
	: CPropertyPage(CPageABF::IDD)
	, m_pFont ( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageABF::~CPageABF()
{
	SAFE_DELETE ( m_pFont );
}

void CPageABF::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ANIM, m_List_Anim );
	DDX_Control(pDX, IDC_LIST_PIECE, m_List_Piece );
}


BEGIN_MESSAGE_MAP(CPageABF, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_ABF_LOAD, OnBnClickedButtonAbfLoad)
	ON_BN_CLICKED(IDC_BUTTON_PIECE_RESET, OnBnClickedButtonPieceReset)
	ON_BN_CLICKED(IDC_BUTTON_PIECE_CPS, OnBnClickedButtonPieceCps)
	ON_BN_CLICKED(IDC_BUTTON_PIECE_EFFSKIN, OnBnClickedButtonPieceEffskin)
	ON_BN_CLICKED(IDC_BUTTON_ANIM_PLAY, OnBnClickedButtonAnimPlay)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_ANIM, OnNMDblclkListAnim)
	ON_BN_CLICKED(IDC_BUTTON_PIECE_EFFSKIN_ABF, OnBnClickedButtonPieceEffskinAbf)
END_MESSAGE_MAP()


// CPageABF message handlers
BOOL CPageABF::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_List_Anim.SetExtendedStyle ( m_List_Anim.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List_Anim.GetClientRect ( &rectCtrl );
	m_List_Anim.InsertColumn( 0, "Num", LVCFMT_LEFT, ( 15* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List_Anim.InsertColumn( 1, "Animation", LVCFMT_LEFT, ( 60* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List_Anim.InsertColumn( 2, "Main/Sub", LVCFMT_LEFT, ( 25* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	m_List_Piece.SetExtendedStyle ( m_List_Piece.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List_Piece.GetClientRect ( &rectCtrl );
	m_List_Piece.InsertColumn( 0, "Piece", LVCFMT_LEFT, ( 40* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List_Piece.InsertColumn( 1, "File", LVCFMT_LEFT, ( 60* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	return TRUE;
}

void CPageABF::DataReset()
{
	SetWin_Text( this, IDC_EDIT_FILENAME, "" );

	m_List_Anim.DeleteAllItems();
	m_List_Piece.DeleteAllItems();
}

void CPageABF::DataShow()
{
	DxAttBoneRender* pData = CGameViewerView::GetView()->m_pAttBoneRender;
	if ( !pData )	return;

	SetWin_Text( this, IDC_EDIT_FILENAME, pData->GetFileName() );

	DataShowAnim();
	DataShowPiece();
}

void CPageABF::DataShowAnim()
{
	m_List_Anim.DeleteAllItems();

	DxAttBoneRender* pData = CGameViewerView::GetView()->m_pAttBoneRender;
	if ( !pData )	return;

	m_List_Anim.SetRedraw( FALSE );

	DxSkinAniControl::VECNODE vecAnim = pData->GetAniList ();
	for ( int i= 0; i<(int)vecAnim.size(); ++ i )
	{
		char szTempChar[512];
		sprintf( szTempChar, "%d", i );
		m_List_Anim.InsertItem( i, szTempChar );

		sprintf( szTempChar, "%s", vecAnim[i]->pAnimCont->m_szName );
		m_List_Anim.SetItemText( i, 1, szTempChar );

		EMANI_MAINTYPE emMain = vecAnim[i]->pAnimCont->m_MainType;
		EMANI_SUBTYPE emSub = vecAnim[i]->pAnimCont->m_SubType;

		sprintf( szTempChar, "(%d)(%d)", emMain, emSub );
		m_List_Anim.SetItemText( i, 2, szTempChar );
	}

	m_List_Anim.SetRedraw( TRUE );
}

void CPageABF::DataShowPiece()
{
	m_List_Piece.DeleteAllItems();

	DxAttBoneRender* pData = CGameViewerView::GetView()->m_pAttBoneRender;
	if ( !pData )	return;

	m_List_Piece.SetRedraw( FALSE );

	for ( int i= 0; i<PIECE_SIZE; ++ i )
	{
		m_List_Piece.InsertItem( i, SKINTYPE_STRING::m_szPIECETYPE[i] );

		PDXCHARPART pPIECE = pData->GetPiece( i );
		if ( pPIECE )
		{
			m_List_Piece.SetItemText( i, 1, pPIECE->GetFileName() );
		}
	}

	m_List_Piece.SetRedraw( TRUE );
}

void CPageABF::OnBnClickedButtonAbfLoad()
{
	CString szFilter = "Container (*.abf)|*.abf;|";
	CFileDialog dlg ( TRUE,".ABF", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );

	dlg.m_ofn.lpstrInitialDir = DxAttBoneDataContainer::GetInstance().GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		std::string strFile = dlg.GetFileName().GetString();
		BOOL bLOAD = CGameViewerView::GetView()->DataLoadABF( strFile.c_str() );
		if ( bLOAD )
		{
			DataShow();
		}else{
			CDebugSet::MsgBox( "Error Load File: %s", strFile.c_str() );
			DataReset();
		}
	}
}

void CPageABF::OnBnClickedButtonPieceReset()
{
	DxAttBoneRender* pData = CGameViewerView::GetView()->m_pAttBoneRender;
	if ( !pData )	return;

	int nSelect = m_List_Piece.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	pData->ResetPiece( nSelect );

	DataShowPiece();
}

void CPageABF::OnBnClickedButtonPieceCps()
{
	DxAttBoneRender* pData = CGameViewerView::GetView()->m_pAttBoneRender;
	if ( !pData )	return;

	CString szFilterInput = "Piece (*.cps,*.aps,*.vps)|*.cps;*.aps;*.vps|";
	CFileDialog dlgInput(true,"*.*",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilterInput,this);
	dlgInput.m_ofn.lpstrInitialDir = DxSkinPieceContainer::GetInstance().GetPath();
	if ( dlgInput.DoModal() != IDOK ) return;

	std::string strFILE = dlgInput.GetFileName();
	pData->SetPiece ( strFILE.c_str(), CGameViewerView::GetView()->Get3DDevice(), FG_MUSTNEWLOAD, 0, TRUE );

	DataShowPiece();
}

void CPageABF::OnBnClickedButtonPieceEffskin()
{
	DxAttBoneRender* pData = CGameViewerView::GetView()->m_pAttBoneRender;
	if ( !pData )	return;

	CString szFilter = "effskin_a|*.effskin_a|";
	CFileDialog dlg ( TRUE,".effskin_a",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = DxEffcharDataMan::GetInstance().GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		D3DXVECTOR3 vDIR( 0.0f, 0.0f, 0.0f);
		DxEffcharDataMan::GetInstance().PutPassiveEffect( pData, dlg.GetFileName(), &vDIR );
	}
}

void CPageABF::OnBnClickedButtonAnimPlay()
{
	PlayAnim();
}

void CPageABF::OnNMDblclkListAnim(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	PlayAnim();
}

void CPageABF::PlayAnim()
{
	DxAttBoneRender* pData = CGameViewerView::GetView()->m_pAttBoneRender;
	if ( !pData )	return;

	int nSelect = m_List_Anim.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	std::string str = m_List_Anim.GetItemText( nSelect, 1 );
	pData->SELECTANI( str.c_str() );
}

void CPageABF::OnBnClickedButtonPieceEffskinAbf()
{
	/*Item Self Effect, Juver, 2017/06/09 */
	DxAttBoneRender* pData = CGameViewerView::GetView()->m_pAttBoneRender;
	if ( !pData )	return;

	CString szFilter = "effskin|*.effskin|";
	CFileDialog dlg ( TRUE,".effskin",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = DxEffSelfEffectMan::GetInstance().GetPath ();
	if ( dlg.DoModal() == IDOK )
	{
		DxEffSelfEffectMan::GetInstance().PutEffect( pData, dlg.GetFileName() );
	}
}
