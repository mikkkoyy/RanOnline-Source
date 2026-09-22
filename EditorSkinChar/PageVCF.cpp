// PageVCF.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkinChar.h"
#include "EditorSkinCharView.h"
#include "PageVCF.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "DlgAniSelect.h"

// CPageVCF dialog

extern std::string	g_strPICK_BONE;

IMPLEMENT_DYNAMIC(CPageVCF, CPropertyPage)

CPageVCF::CPageVCF(LOGFONT logfont)
	: CPropertyPage(CPageVCF::IDD)
	, m_pFont( NULL )
	, m_pSkin( NULL )
	, m_pData( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageVCF::~CPageVCF()
{
	SAFE_DELETE( m_pFont );
}

void CPageVCF::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_VCF_ANIM, m_List_Anim );
	DDX_Control(pDX, IDC_LIST_VCF_PIECE, m_List_Piece );
}


BEGIN_MESSAGE_MAP(CPageVCF, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_VCF_NEW, OnBnClickedButtonVcfNew)
	ON_BN_CLICKED(IDC_BUTTON_VCF_LOAD, OnBnClickedButtonVcfLoad)
	ON_BN_CLICKED(IDC_BUTTON_VCF_SAVE, OnBnClickedButtonVcfSave)
	ON_BN_CLICKED(IDC_BUTTON_VCF_SKELETON, OnBnClickedButtonVcfSkeleton)
	ON_BN_CLICKED(IDC_BUTTON_VCF_APPLY, OnBnClickedButtonVcfApply)
	ON_BN_CLICKED(IDC_BUTTON_VCF_PIECE_CPS, OnBnClickedButtonVcfPieceCps)
	ON_BN_CLICKED(IDC_BUTTON_VCF_PIECE_RESET, OnBnClickedButtonVcfPieceReset)
	ON_BN_CLICKED(IDC_BUTTON_VCF_ANIM_PLAY, OnBnClickedButtonVcfAnimPlay)
	ON_BN_CLICKED(IDC_BUTTON_VCF_ANIM_STOP, OnBnClickedButtonVcfAnimStop)
	ON_BN_CLICKED(IDC_BUTTON_VCF_ANIM_ADDCFG, OnBnClickedButtonVcfAnimAddcfg)
	ON_BN_CLICKED(IDC_BUTTON_VCF_ANIM_DELETE, OnBnClickedButtonVcfAnimDelete)
	ON_BN_CLICKED(IDC_BUTTON_VCF_ANIM_CLEAR, OnBnClickedButtonVcfAnimClear)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_VCF_ANIM, OnNMDblclkListVcfAnim)
	ON_BN_CLICKED(IDC_VCF_BUTTON_BONE_1, OnBnClickedVcfButtonBone1)
	ON_BN_CLICKED(IDC_VCF_BUTTON_EDITMAT_1, OnBnClickedVcfButtonEditmat1)
	ON_BN_CLICKED(IDC_VCF_BUTTON_RESET_MAT_1, OnBnClickedVcfButtonResetMat1)
	ON_BN_CLICKED(IDC_VCF_BUTTON_BONE_2, OnBnClickedVcfButtonBone2)
	ON_BN_CLICKED(IDC_VCF_BUTTON_EDITMAT_2, OnBnClickedVcfButtonEditmat2)
	ON_BN_CLICKED(IDC_VCF_BUTTON_RESET_MAT_2, OnBnClickedVcfButtonResetMat2)
	ON_BN_CLICKED(IDC_BUTTON_VCF_TEST_SKIN, OnBnClickedButtonVcfTestSkin)
	ON_BN_CLICKED(IDC_BUTTON_VCF_TEST_SKIN_P, OnBnClickedButtonVcfTestSkinP)
	ON_BN_CLICKED(IDC_BUTTON_VCF_TEST_SKIN_S, OnBnClickedButtonVcfTestSkinS)
	ON_BN_CLICKED(IDC_BUTTON_ANI_INSERT_VCF, OnBnClickedButtonAniInsertVcf)
	ON_BN_CLICKED(IDC_VCF_BUTTON_MANUAL_MAT1, OnBnClickedVcfButtonManualMat1)
	ON_BN_CLICKED(IDC_VCF_BUTTON_MANUAL_MAT2, OnBnClickedVcfButtonManualMat2)
	ON_BN_CLICKED(IDC_VCF_BUTTON_CANCEL_MAT_1, OnBnClickedVcfButtonCancelMat1)
	ON_BN_CLICKED(IDC_VCF_BUTTON_CANCEL_MAT_2, OnBnClickedVcfButtonCancelMat2)
	ON_BN_CLICKED(IDC_VCF_BUTTON_TEST_MAT_1, OnBnClickedVcfButtonTestMat1)
	ON_BN_CLICKED(IDC_VCF_BUTTON_TEST_MAT_2, OnBnClickedVcfButtonTestMat2)
	ON_BN_CLICKED(IDC_VCF_BUTTON_SAVE_MAT_1, OnBnClickedVcfButtonSaveMat1)
	ON_BN_CLICKED(IDC_VCF_BUTTON_SAVE_MAT_2, OnBnClickedVcfButtonSaveMat2)
END_MESSAGE_MAP()


// CPageVCF message handlers
BOOL CPageVCF::OnInitDialog()
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

	ResetTool();

	return TRUE;  
}

void CPageVCF::ResetTool()
{
	m_pSkin = NULL;
	m_pData = NULL;

	SetWin_Text( this, IDC_EDIT_VCF_FILENAME, "" );
	SetWin_Text( this, IDC_EDIT_VCF_SKELETON, "" );
	SetWin_Num_float( this, IDC_EDIT_VCF_SCALE, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_VCF_HEIGHT, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_VCF_RADIUS, 1.0f );
	SetWin_Check( this, IDC_CHECK_VCF_WORLDOBJECT, FALSE );

	SetWin_Text( this, IDC_VCF_EDIT_BONE_1, "" );
	SetWin_Text( this, IDC_VCF_EDIT_BONE_2, "" );

	m_List_Anim.DeleteAllItems();
	m_List_Piece.DeleteAllItems();

	SetWin_Text( this, IDC_EDIT_ANI_INSERT_VCF, "" );

	SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_X_VCF_1, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Y_VCF_1, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Z_VCF_1, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_X_VCF_1, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Y_VCF_1, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Z_VCF_1, 0.0f );

	SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_X_VCF_2, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Y_VCF_2, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Z_VCF_2, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_X_VCF_2, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Y_VCF_2, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Z_VCF_2, 0.0f );

	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_X_VCF_1, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_Y_VCF_1, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_Z_VCF_1, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_X_VCF_1, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_Y_VCF_1, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_Z_VCF_1, FALSE );

	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_X_VCF_2, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_Y_VCF_2, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_Z_VCF_2, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_X_VCF_2, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_Y_VCF_2, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_Z_VCF_2, FALSE );

	SetWin_Enable( this, IDC_VCF_BUTTON_TEST_MAT_1, FALSE );
	SetWin_Enable( this, IDC_VCF_BUTTON_SAVE_MAT_1, FALSE );
	SetWin_Enable( this, IDC_VCF_BUTTON_CANCEL_MAT_1, FALSE );

	SetWin_Enable( this, IDC_VCF_BUTTON_TEST_MAT_2, FALSE );
	SetWin_Enable( this, IDC_VCF_BUTTON_SAVE_MAT_2, FALSE );
	SetWin_Enable( this, IDC_VCF_BUTTON_CANCEL_MAT_2, FALSE );
}

void CPageVCF::OnBnClickedButtonVcfNew()
{
	CEditorSkinCharView::GetView()->VCFNew();

	m_pSkin = CEditorSkinCharView::GetView()->m_pVehicle;
	m_pData = CEditorSkinCharView::GetView()->m_pVehicleData;

	if ( m_pSkin && m_pData )
		DataShow();
}

void CPageVCF::OnBnClickedButtonVcfLoad()
{
	CString szFilter = "Container (*.vcf)|*.vcf;|";
	CFileDialog dlg ( TRUE,".VCF", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = DxVehicleDataContainer::GetInstance().GetPath();
	if ( dlg.DoModal() != IDOK ) return;

	BOOL bOK = CEditorSkinCharView::GetView()->VCFLoad( dlg.GetFileName().GetString() );
	if ( bOK )
	{
		m_pSkin = CEditorSkinCharView::GetView()->m_pVehicle;
		m_pData = CEditorSkinCharView::GetView()->m_pVehicleData;

		if ( m_pSkin && m_pData )
			DataShow();
	}
}

void CPageVCF::OnBnClickedButtonVcfSave()
{
	if( !m_pData )	return;
	if( !m_pSkin )	return;

	if( !m_pData->GetSkeleton() )	
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Skeleton!" );
		return;
	}

	if( !m_pData->GetAniList().size() )	
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Animation Empty!" );
		return;
	}

	if ( m_pData->m_strCharFront.empty() )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "invalid front bonelink" );
		return;
	}

	if ( m_pData->m_strCharBack.empty() )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "invalid back bonelink" );
		return;
	}

	m_pData->m_affCharFront = *m_pSkin->m_affCharFront.m_pAffineParts;
	m_pData->m_affCharBack = *m_pSkin->m_affCharBack.m_pAffineParts;

	CString szFilter = "Container (*.vcf)|*.vcf;|";
	CFileDialog dlg(FALSE,"*.*", m_pData->GetFileName(), OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = DxVehicleDataContainer::GetInstance().GetPath();
	if ( dlg.DoModal() != IDOK ) return;

	BOOL bOK = CEditorSkinCharView::GetView()->VCFSave( dlg.GetFileName().GetString() );
	if ( bOK )
	{
		if ( m_pSkin && m_pData )
			DataShow();
	}
}

void CPageVCF::DataShow()
{
	if ( !m_pData )	return;

	SetWin_Text( this, IDC_EDIT_VCF_FILENAME, m_pData->GetFileName() );
	SetWin_Text( this, IDC_EDIT_VCF_SKELETON, m_pData->GetSkeletonName() );
	SetWin_Num_float( this, IDC_EDIT_VCF_SCALE, m_pData->m_fScale );
	SetWin_Num_float( this, IDC_EDIT_VCF_HEIGHT, m_pData->m_fHeight );
	SetWin_Num_float( this, IDC_EDIT_VCF_RADIUS, m_pData->m_fRadius );
	SetWin_Check( this, IDC_CHECK_VCF_WORLDOBJECT, m_pData->m_bWorldObj );

	SetWin_Text( this, IDC_VCF_EDIT_BONE_1, m_pData->m_strCharFront.c_str() );
	SetWin_Text( this, IDC_VCF_EDIT_BONE_2, m_pData->m_strCharBack.c_str() );

	DataShowAnim();
	DataShowPiece();

	CEditorSkinCharView::GetView()->EditSmatSet( FALSE );

	ManualMatrixReset1();
	ManualMatrixReset2();
}

void CPageVCF::DataShowAnim()
{
	m_List_Anim.DeleteAllItems();

	if ( !m_pData )	return;

	m_List_Anim.SetRedraw( FALSE );

	DxSkinAniControl::VECNODE vecAnim = m_pData->GetAniList ();
	for ( int i= 0; i<(int)vecAnim.size(); ++ i )
	{
		char szTempChar[512];
		sprintf( szTempChar, "%d", i );
		m_List_Anim.InsertItem( i, szTempChar );
		m_List_Anim.SetItemText( i, 1, vecAnim[i]->pAnimCont->m_szName );

		sprintf( szTempChar, "(%d)(%d)", vecAnim[i]->pAnimCont->m_MainType, vecAnim[i]->pAnimCont->m_SubType );
		m_List_Anim.SetItemText( i, 2, szTempChar );
	}

	m_List_Anim.SetRedraw( TRUE );
}

void CPageVCF::DataShowPiece()
{
	m_List_Piece.DeleteAllItems();

	if ( !m_pData )	return;

	m_List_Piece.SetRedraw( FALSE );

	for ( int i= 0; i<PIECE_SIZE; ++ i )
	{
		m_List_Piece.InsertItem( i, SKINTYPE_STRING::m_szPIECETYPE[i] );

		PDXSKINPIECE pPIECE = m_pData->GetPiece( i );
		if ( pPIECE )
		{
			m_List_Piece.SetItemText( i, 1, pPIECE->m_szFileName );
		}
	}

	m_List_Piece.SetRedraw( TRUE );
}

void CPageVCF::ResetAnimation()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	m_pSkin->ResetAnimationCore();
	m_pData->ResetAnimationCore();

	if ( m_pSkin->GetSkeleton() && m_pSkin->GetSkeleton()->pBoneRoot )
		m_pSkin->GetSkeleton()->pBoneRoot->ResetBone();

	if ( m_pData->GetSkeleton() && m_pData->GetSkeleton()->pBoneRoot )
		m_pData->GetSkeleton()->pBoneRoot->ResetBone();
}

void CPageVCF::OnBnClickedButtonVcfSkeleton()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	int nRETURN = CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Change Skeleton? Piece and animations will be cleared!" );
	if ( nRETURN != IDYES )	return;

	CString szFilterInput = "Skeleton (*.x)|*.x;|";
	CFileDialog dlgInput(true,"*.X*",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilterInput, this );
	dlgInput.m_ofn.lpstrInitialDir = DxBoneCollector::GetInstance().GetPath();
	if ( dlgInput.DoModal() != IDOK ) return;

	std::string strSkeleton = dlgInput.GetFileName();
	DxSkeleton* pSkeleton = DxBoneCollector::GetInstance().Load( strSkeleton.c_str(), CEditorSkinCharView::GetView()->Get3DDevice() );
	if ( pSkeleton )
	{
		ResetAnimation();

		m_pData->ClearAll();
		m_pSkin->ClearAll();

		BOOL bOK = m_pData->LoadSkeleton( strSkeleton.c_str(), CEditorSkinCharView::GetView()->Get3DDevice() );
		if ( !bOK )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Error Load Skeleton :%s", strSkeleton.c_str() );
			return;
		}

		bOK = m_pSkin->LoadSkeleton( strSkeleton.c_str(), CEditorSkinCharView::GetView()->Get3DDevice() );
		if ( !bOK )
		{
			CDebugSet::MsgBox( GetSafeHwnd(), "Error Load Skeleton :%s", strSkeleton.c_str() );
			return;
		}

		DataShow();
	}
}

void CPageVCF::OnBnClickedButtonVcfApply()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	m_pData->m_fScale = GetWin_Num_float( this, IDC_EDIT_VCF_SCALE );
	m_pData->m_fHeight = GetWin_Num_float( this, IDC_EDIT_VCF_HEIGHT );
	m_pData->m_fRadius = GetWin_Num_float( this, IDC_EDIT_VCF_RADIUS );
	m_pData->m_bWorldObj = GetWin_Check( this, IDC_CHECK_VCF_WORLDOBJECT );
	m_pData->SetRadiusHeight();

	m_pSkin->SetScale( m_pData->m_fScale );
	m_pSkin->SetRadiusHeight( m_pData );
	m_pSkin->SetWorldObj( m_pData->m_bWorldObj );
}

void CPageVCF::OnBnClickedButtonVcfPieceCps()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	CString szFilterInput = "Piece (*.cps,*.aps,*.vps)|*.cps;*.aps;*.vps|";
	CFileDialog dlgInput(true,"*.*",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilterInput,this);
	dlgInput.m_ofn.lpstrInitialDir = DxSkinPieceContainer::GetInstance().GetPath();
	if ( dlgInput.DoModal() != IDOK ) return;

	std::string strFILE = dlgInput.GetFileName();
	PDXSKINPIECE pPiece = new DxSkinPiece;
	HRESULT hr = pPiece->LoadPiece( strFILE.c_str(), CEditorSkinCharView::GetView()->Get3DDevice(), TRUE );
	if ( FAILED(hr) )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "%s Error Loading Piece", strFILE.c_str() );
		SAFE_DELETE( pPiece );
		return;
	}

	//skeleton check
	CString strFile1( m_pData->GetSkeletonName() );
	strFile1.MakeLower();

	CString strFile2( pPiece->m_szSkeleton );
	strFile2.MakeLower();

	if( _tcscmp(strFile1.GetString(),strFile2.GetString()) != 0 )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "%s Invalid Piece Skeleton Mismatch", strFILE.c_str() );
		SAFE_DELETE( pPiece );
		return;
	}

	m_pSkin->ResetPiece( pPiece->m_emType );
	m_pData->ResetPiece( CEditorSkinCharView::GetView()->Get3DDevice(), pPiece->m_emType );

	SAFE_DELETE( pPiece );
	DxSkinPieceContainer::GetInstance().DeletePiece ( strFILE.c_str() );

	hr = m_pData->SetPiece( strFILE.c_str(), CEditorSkinCharView::GetView()->Get3DDevice(), TRUE );
	if ( FAILED(hr) )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "%s Invalid Piece", strFILE.c_str() );
		return;
	}

	hr = m_pSkin->SetPiece( strFILE.c_str(), CEditorSkinCharView::GetView()->Get3DDevice(), 0, 0, TRUE );
	if ( FAILED(hr) )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "%s Invalid Piece", strFILE.c_str() );
		return;
	}

	DataShowPiece();

	for( int nItem = 0; nItem < m_List_Piece.GetItemCount(); nItem++)
	{
		std::string str = m_List_Piece.GetItemText( nItem, 1 );
		if( _tcscmp( strFILE.c_str(), str.c_str() ) == 0 )
		{
			m_List_Piece.EnsureVisible( nItem, TRUE );
			m_List_Piece.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
		}
	}
}

void CPageVCF::OnBnClickedButtonVcfPieceReset()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	int nSelect = m_List_Piece.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	m_pSkin->ResetPiece( nSelect );
	m_pData->ResetPiece( CEditorSkinCharView::GetView()->Get3DDevice(), nSelect );

	DataShowPiece();
}

void CPageVCF::OnBnClickedButtonVcfAnimPlay()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	int nSelect = m_List_Anim.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	std::string str = m_List_Anim.GetItemText( nSelect, 1 );
	m_pSkin->SELECTANI( str.c_str() );
	m_pData->SELECTANI( str.c_str() );
}

void CPageVCF::OnBnClickedButtonVcfAnimStop()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	ResetAnimation();
}

void CPageVCF::OnBnClickedButtonVcfAnimAddcfg()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	if ( !m_pData->GetSkeleton() )	
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Skeleton!" );
		return;
	}

	CString szFilterInput = "Animation Cfg Files (*.cfg)|*.cfg|";
	CFileDialog dlg( TRUE,".bin",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilterInput, this );
	dlg.m_ofn.lpstrInitialDir = DxSkinAniMan::GetInstance().GetPath();
	if ( dlg.DoModal() != IDOK ) return;

	CString strFile = dlg.GetFileName().GetString();
	int nIndex = strFile.ReverseFind ( '.' );
	std::string strFileName = strFile.Left(nIndex) + ".x";

	PSANIMCONTAINER pAnim = DxSkinAniMan::GetInstance().LoadAnimContainer ( strFileName.c_str(), m_pData->GetSkeletonName(), CEditorSkinCharView::GetView()->Get3DDevice(), FALSE );
	if ( !pAnim )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Error Load CFG :%s", strFileName.c_str() );
		return;
	}

	ResetAnimation();

	BOOL bOK = m_pSkin->LoadAnimation( strFileName.c_str(), CEditorSkinCharView::GetView()->Get3DDevice(), FALSE );
	if ( !bOK )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Error Load Animation :%s", strFileName.c_str() );
		return;
	}

	bOK = m_pData->LoadAnimation( strFileName.c_str(), CEditorSkinCharView::GetView()->Get3DDevice(), FALSE );
	if ( !bOK )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Error Load Animation :%s", strFileName.c_str() );
		return;
	}

	DataShowAnim();

	for( int nItem = 0; nItem < m_List_Anim.GetItemCount(); nItem++)
	{
		std::string str = m_List_Anim.GetItemText( nItem, 1 );
		if( _tcscmp( strFileName.c_str(), str.c_str() ) == 0 )
		{
			m_List_Anim.EnsureVisible( nItem, TRUE );
			m_List_Anim.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
		}
	}
}

void CPageVCF::OnBnClickedButtonVcfAnimDelete()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	int nSelect = m_List_Anim.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	std::string str = m_List_Anim.GetItemText( nSelect, 1 );

	ResetAnimation();

	m_pSkin->ReleaseAnimation( str.c_str() );
	m_pData->ReleaseAnimation( str.c_str() );

	DataShowAnim();

	int nNewSelect = nSelect > 1? nSelect-1:nSelect;
	m_List_Anim.EnsureVisible( nNewSelect, TRUE );
	m_List_Anim.SetItemState( nNewSelect, LVIS_SELECTED, LVIS_SELECTED );
}

void CPageVCF::OnBnClickedButtonVcfAnimClear()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	int nRETURN = CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Clear all animations?" );
	if ( nRETURN != IDYES )	return;

	ResetAnimation();

	DxSkinAniControl::VECNODE vecNode = m_pSkin->GetAniList();
	for( size_t i=0; i<vecNode.size(); ++i )
	{
		std::string strAniName = vecNode[i]->pAnimCont->m_szName;
		m_pSkin->ReleaseAnimation( strAniName.c_str() );
	}

	vecNode = m_pData->GetAniList();
	for( size_t i=0; i<vecNode.size(); ++i )
	{
		std::string strAniName = vecNode[i]->pAnimCont->m_szName;
		m_pData->ReleaseAnimation( strAniName.c_str() );
	}

	DataShowAnim();
}

void CPageVCF::OnNMDblclkListVcfAnim(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	int nSelect = m_List_Anim.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	std::string str = m_List_Anim.GetItemText( nSelect, 1 );
	m_pSkin->SELECTANI( str.c_str() );
	m_pData->SELECTANI( str.c_str() );
}

void CPageVCF::OnBnClickedVcfButtonBone1()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	if ( !g_strPICK_BONE.size() )	return;

	int nRETURN = CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Change Front BoneLink to :%s ?", g_strPICK_BONE.c_str() );
	if ( nRETURN != IDYES )	return;

	m_pSkin->m_strCharFront = g_strPICK_BONE.c_str();
	m_pData->m_strCharFront = g_strPICK_BONE.c_str();

	DataShow();
}

void CPageVCF::OnBnClickedVcfButtonEditmat1()
{
	if( !m_pSkin )	return;

	CEditorSkinCharView::GetView()->EditSmatSet( TRUE );
	CEditorSkinCharView::GetView()->m_DxEditMat.SetAffineMatrix( &m_pSkin->m_affCharFront );
	CEditorSkinCharView::GetView()->m_bEditBack = FALSE;

	if ( m_pSkin->GetSkeleton() )
	{
		DxBoneTrans *pBoneCur = m_pSkin->GetSkeleton()->FindBone ( m_pSkin->m_strCharFront.c_str() );
		if ( pBoneCur )
			CEditorSkinCharView::GetView()->m_DxEditMat.SetFrameMatrix( pBoneCur->matCombined );
	}

	ManualMatrixReset1();
}

void CPageVCF::OnBnClickedVcfButtonResetMat1()
{
	if( !m_pSkin )	return;
	m_pSkin->m_affCharFront.UseAffineMatrix();

	DataShow();
}

void CPageVCF::OnBnClickedVcfButtonBone2()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	if ( !g_strPICK_BONE.size() )	return;

	int nRETURN = CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Change Back BoneLink to :%s ?", g_strPICK_BONE.c_str() );
	if ( nRETURN != IDYES )	return;

	m_pSkin->m_strCharBack = g_strPICK_BONE.c_str();
	m_pData->m_strCharBack = g_strPICK_BONE.c_str();

	DataShow();
}

void CPageVCF::OnBnClickedVcfButtonEditmat2()
{
	if( !m_pSkin )	return;

	CEditorSkinCharView::GetView()->EditSmatSet( TRUE );
	CEditorSkinCharView::GetView()->m_DxEditMat.SetAffineMatrix( &m_pSkin->m_affCharBack );
	CEditorSkinCharView::GetView()->m_bEditBack = TRUE;

	if ( m_pSkin->GetSkeleton() )
	{
		DxBoneTrans *pBoneCur = m_pSkin->GetSkeleton()->FindBone ( m_pSkin->m_strCharBack.c_str() );
		if ( pBoneCur )
			CEditorSkinCharView::GetView()->m_DxEditMat.SetFrameMatrix( pBoneCur->matCombined );
	}

	ManualMatrixReset2();
}

void CPageVCF::OnBnClickedVcfButtonResetMat2()
{
	if( !m_pSkin )	return;
	m_pSkin->m_affCharBack.UseAffineMatrix();

	DataShow();
}

void CPageVCF::OnBnClickedButtonVcfTestSkin()
{
	CString szFilter = "Container (*.chf)|*.chf;|";
	CFileDialog dlg ( TRUE,".CHF", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = DxSkinCharDataContainer::GetInstance().GetPath();
	if ( dlg.DoModal() != IDOK ) return;

	CEditorSkinCharView::GetView()->VCFLoadTest( dlg.GetFileName().GetString() );
}

void CPageVCF::OnBnClickedButtonVcfTestSkinP()
{
	DxSkinChar*	pSkin = CEditorSkinCharView::GetView()->m_pVehicleSkin;
	if ( !pSkin )	return;

	CDlgAniSelect dlg( this, pSkin );
	if( dlg.DoModal() == IDOK )
	{

	}
}

void CPageVCF::OnBnClickedButtonVcfTestSkinS()
{
	DxSkinChar*	pSkin = CEditorSkinCharView::GetView()->m_pVehicleSkin;
	if ( !pSkin )	return;

	pSkin->ResetAnimationCore();

	if ( pSkin->GetSkeleton() && pSkin->GetSkeleton()->pBoneRoot )
		pSkin->GetSkeleton()->pBoneRoot->ResetBone();
}

void CPageVCF::OnBnClickedButtonAniInsertVcf()
{
	if ( !m_pData )	return;
	if ( !m_pSkin )	return;

	CString strAni = GetWin_Text( this, IDC_EDIT_ANI_INSERT_VCF );
	if ( strAni.GetLength() <= 0  )	return;

	int nIndex = strAni.ReverseFind ( '.' );
	std::string strFileName = strAni.Left(nIndex) + ".x";

	PSANIMCONTAINER pAnim = DxSkinAniMan::GetInstance().LoadAnimContainer ( strFileName.c_str(), m_pData->GetSkeletonName(), CEditorSkinCharView::GetView()->Get3DDevice(), FALSE );
	if ( !pAnim )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Error Load CFG :%s", strFileName.c_str() );
		return;
	}

	ResetAnimation();

	BOOL bOK = m_pSkin->LoadAnimation( strFileName.c_str(), CEditorSkinCharView::GetView()->Get3DDevice(), FALSE );
	if ( !bOK )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Error Load Animation :%s", strFileName.c_str() );
		return;
	}

	bOK = m_pData->LoadAnimation( strFileName.c_str(), CEditorSkinCharView::GetView()->Get3DDevice(), FALSE );
	if ( !bOK )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Error Load Animation :%s", strFileName.c_str() );
		return;
	}

	DataShowAnim();

	for( int nItem = 0; nItem < m_List_Anim.GetItemCount(); nItem++)
	{
		std::string str = m_List_Anim.GetItemText( nItem, 1 );
		if( _tcscmp( strFileName.c_str(), str.c_str() ) == 0 )
		{
			m_List_Anim.EnsureVisible( nItem, TRUE );
			m_List_Anim.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
		}
	}
}

void CPageVCF::OnBnClickedVcfButtonManualMat1()
{
	CEditorSkinCharView::GetView()->EditSmatSet( FALSE );
	CEditorSkinCharView::GetView()->m_bEditBack = FALSE;
	ManualMatrixShow1();
}

void CPageVCF::OnBnClickedVcfButtonManualMat2()
{
	CEditorSkinCharView::GetView()->EditSmatSet( FALSE );
	CEditorSkinCharView::GetView()->m_bEditBack = TRUE;
	ManualMatrixShow2();
}

void CPageVCF::OnBnClickedVcfButtonCancelMat1()
{
	ManualMatrixReset1();
}

void CPageVCF::OnBnClickedVcfButtonCancelMat2()
{
	ManualMatrixReset2();
}

void CPageVCF::OnBnClickedVcfButtonTestMat1()
{
	ManualMatrixSave1();
}

void CPageVCF::OnBnClickedVcfButtonTestMat2()
{
	ManualMatrixSave2();
}

void CPageVCF::OnBnClickedVcfButtonSaveMat1()
{
	ManualMatrixSave1();
	ManualMatrixReset1();
}

void CPageVCF::OnBnClickedVcfButtonSaveMat2()
{
	ManualMatrixSave2();
	ManualMatrixReset2();
}

void CPageVCF::ManualMatrixReset1()
{
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_X_VCF_1, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_Y_VCF_1, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_Z_VCF_1, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_X_VCF_1, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_Y_VCF_1, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_Z_VCF_1, FALSE );

	SetWin_Enable( this, IDC_VCF_BUTTON_TEST_MAT_1, FALSE );
	SetWin_Enable( this, IDC_VCF_BUTTON_SAVE_MAT_1, FALSE );
	SetWin_Enable( this, IDC_VCF_BUTTON_CANCEL_MAT_1, FALSE );

	if ( m_pSkin )
	{
		SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_X_VCF_1, m_pSkin->m_affCharFront.m_pAffineParts->vTrans.x );
		SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Y_VCF_1, m_pSkin->m_affCharFront.m_pAffineParts->vTrans.y );
		SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Z_VCF_1, m_pSkin->m_affCharFront.m_pAffineParts->vTrans.z );
		SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_X_VCF_1, m_pSkin->m_affCharFront.m_pAffineParts->vRotate.x );
		SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Y_VCF_1, m_pSkin->m_affCharFront.m_pAffineParts->vRotate.y );
		SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Z_VCF_1, m_pSkin->m_affCharFront.m_pAffineParts->vRotate.z );
	}
}

void CPageVCF::ManualMatrixReset2()
{
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_X_VCF_2, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_Y_VCF_2, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_Z_VCF_2, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_X_VCF_2, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_Y_VCF_2, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_Z_VCF_2, FALSE );

	SetWin_Enable( this, IDC_VCF_BUTTON_TEST_MAT_2, FALSE );
	SetWin_Enable( this, IDC_VCF_BUTTON_SAVE_MAT_2, FALSE );
	SetWin_Enable( this, IDC_VCF_BUTTON_CANCEL_MAT_2, FALSE );

	if ( m_pSkin )
	{
		SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_X_VCF_2, m_pSkin->m_affCharBack.m_pAffineParts->vTrans.x );
		SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Y_VCF_2, m_pSkin->m_affCharBack.m_pAffineParts->vTrans.y );
		SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Z_VCF_2, m_pSkin->m_affCharBack.m_pAffineParts->vTrans.z );
		SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_X_VCF_2, m_pSkin->m_affCharBack.m_pAffineParts->vRotate.x );
		SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Y_VCF_2, m_pSkin->m_affCharBack.m_pAffineParts->vRotate.y );
		SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Z_VCF_2, m_pSkin->m_affCharBack.m_pAffineParts->vRotate.z );
	}
}

void CPageVCF::ManualMatrixShow1()
{
	if ( !m_pSkin )	return;

	SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_X_VCF_1, m_pSkin->m_affCharFront.m_pAffineParts->vTrans.x );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Y_VCF_1, m_pSkin->m_affCharFront.m_pAffineParts->vTrans.y );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Z_VCF_1, m_pSkin->m_affCharFront.m_pAffineParts->vTrans.z );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_X_VCF_1, m_pSkin->m_affCharFront.m_pAffineParts->vRotate.x );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Y_VCF_1, m_pSkin->m_affCharFront.m_pAffineParts->vRotate.y );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Z_VCF_1, m_pSkin->m_affCharFront.m_pAffineParts->vRotate.z );

	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_X_VCF_1, TRUE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_Y_VCF_1, TRUE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_Z_VCF_1, TRUE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_X_VCF_1, TRUE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_Y_VCF_1, TRUE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_Z_VCF_1, TRUE );

	SetWin_Enable( this, IDC_VCF_BUTTON_TEST_MAT_1, TRUE );
	SetWin_Enable( this, IDC_VCF_BUTTON_SAVE_MAT_1, TRUE );
	SetWin_Enable( this, IDC_VCF_BUTTON_CANCEL_MAT_1, TRUE );
}

void CPageVCF::ManualMatrixShow2()
{
	if ( !m_pSkin )	return;

	SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_X_VCF_2, m_pSkin->m_affCharBack.m_pAffineParts->vTrans.x );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Y_VCF_2, m_pSkin->m_affCharBack.m_pAffineParts->vTrans.y );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Z_VCF_2, m_pSkin->m_affCharBack.m_pAffineParts->vTrans.z );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_X_VCF_2, m_pSkin->m_affCharBack.m_pAffineParts->vRotate.x );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Y_VCF_2, m_pSkin->m_affCharBack.m_pAffineParts->vRotate.y );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Z_VCF_2, m_pSkin->m_affCharBack.m_pAffineParts->vRotate.z );

	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_X_VCF_2, TRUE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_Y_VCF_2, TRUE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_Z_VCF_2, TRUE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_X_VCF_2, TRUE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_Y_VCF_2, TRUE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_Z_VCF_2, TRUE );

	SetWin_Enable( this, IDC_VCF_BUTTON_TEST_MAT_2, TRUE );
	SetWin_Enable( this, IDC_VCF_BUTTON_SAVE_MAT_2, TRUE );
	SetWin_Enable( this, IDC_VCF_BUTTON_CANCEL_MAT_2, TRUE );
}

void CPageVCF::ManualMatrixSave1()
{
	if ( !m_pSkin )	return;

	m_pSkin->m_affCharFront.m_pAffineParts->vTrans.x = GetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_X_VCF_1 );
	m_pSkin->m_affCharFront.m_pAffineParts->vTrans.y = GetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Y_VCF_1 );
	m_pSkin->m_affCharFront.m_pAffineParts->vTrans.z = GetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Z_VCF_1 );
	m_pSkin->m_affCharFront.m_pAffineParts->vRotate.x = GetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_X_VCF_1 );
	m_pSkin->m_affCharFront.m_pAffineParts->vRotate.y = GetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Y_VCF_1 );
	m_pSkin->m_affCharFront.m_pAffineParts->vRotate.z = GetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Z_VCF_1 );
}

void CPageVCF::ManualMatrixSave2()
{
	if ( !m_pSkin )	return;

	m_pSkin->m_affCharBack.m_pAffineParts->vTrans.x = GetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_X_VCF_2 );
	m_pSkin->m_affCharBack.m_pAffineParts->vTrans.y = GetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Y_VCF_2 );
	m_pSkin->m_affCharBack.m_pAffineParts->vTrans.z = GetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Z_VCF_2 );
	m_pSkin->m_affCharBack.m_pAffineParts->vRotate.x = GetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_X_VCF_2 );
	m_pSkin->m_affCharBack.m_pAffineParts->vRotate.y = GetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Y_VCF_2 );
	m_pSkin->m_affCharBack.m_pAffineParts->vRotate.z = GetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Z_VCF_2 );
}