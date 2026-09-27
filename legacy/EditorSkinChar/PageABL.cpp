// PageABL.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkinChar.h"
#include "EditorSkinCharView.h"
#include "PageABL.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "DxEffectMan.h"

extern std::string	g_strPICK_BONE;

// CPageABL dialog

IMPLEMENT_DYNAMIC(CPageABL, CPropertyPage)

CPageABL::CPageABL(LOGFONT logfont)
	: CPropertyPage(CPageABL::IDD)
	, m_pFont( NULL )
	, m_pSkin( NULL )
	, m_pData( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageABL::~CPageABL()
{
	SAFE_DELETE( m_pFont );
}

void CPageABL::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPageABL, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_ABL_NEW, OnBnClickedButtonAblNew)
	ON_BN_CLICKED(IDC_BUTTON_ABL_LOAD, OnBnClickedButtonAblLoad)
	ON_BN_CLICKED(IDC_BUTTON_ABL_SAVE, OnBnClickedButtonAblSave)
	ON_BN_CLICKED(IDC_ABL_BUTTON_SKELETON, OnBnClickedAblButtonSkeleton)
	ON_BN_CLICKED(IDC_ABL_BUTTON_BONEDATA, OnBnClickedAblButtonBonedata)
	ON_BN_CLICKED(IDC_ABL_BUTTON_MASKP, OnBnClickedAblButtonMaskp)
	ON_BN_CLICKED(IDC_ABL_BUTTON_BONELINK, OnBnClickedAblButtonBonelink)
	ON_BN_CLICKED(IDC_ABL_BUTTON_EDITMAT_1, OnBnClickedAblButtonEditmat1)
	ON_BN_CLICKED(IDC_ABL_BUTTON_EDITMAT_2, OnBnClickedAblButtonEditmat2)
	ON_BN_CLICKED(IDC_ABL_BUTTON_RESET_MAT01, OnBnClickedAblButtonResetMat01)
	ON_BN_CLICKED(IDC_ABL_BUTTON_RESET_MAT02, OnBnClickedAblButtonResetMat02)
	ON_BN_CLICKED(IDC_ABL_BUTTON_MASKP_RESET, OnBnClickedAblButtonMaskpReset)
	ON_CBN_SELCHANGE(IDC_ABL_COMBO_STRIKE, OnCbnSelchangeAblComboStrike)
	ON_CBN_SELCHANGE(IDC_ABL_COMBO_PTYPE, OnCbnSelchangeAblComboPtype)
	ON_CBN_SELCHANGE(IDC_ABL_COMBO_SLOTB, OnCbnSelchangeAblComboSlotb)
	ON_BN_CLICKED(IDC_ABL_BUTTON_ANIM_PLAY, OnBnClickedAblButtonAnimPlay)
	ON_BN_CLICKED(IDC_ABL_BUTTON_ANIM_STOP, OnBnClickedAblButtonAnimStop)
	ON_BN_CLICKED(IDC_ABL_BUTTON_MANUAL_MAT1, OnBnClickedAblButtonManualMat1)
	ON_BN_CLICKED(IDC_ABL_BUTTON_MANUAL_MAT2, OnBnClickedAblButtonManualMat2)
	ON_BN_CLICKED(IDC_ABL_BUTTON_TEST_MAT_1, OnBnClickedAblButtonTestMat1)
	ON_BN_CLICKED(IDC_ABL_BUTTON_TEST_MAT_2, OnBnClickedAblButtonTestMat2)
	ON_BN_CLICKED(IDC_ABL_BUTTON_SAVE_MAT_1, OnBnClickedAblButtonSaveMat1)
	ON_BN_CLICKED(IDC_ABL_BUTTON_SAVE_MAT_2, OnBnClickedAblButtonSaveMat2)
	ON_BN_CLICKED(IDC_ABL_BUTTON_CANCEL_MAT_1, OnBnClickedAblButtonCancelMat1)
	ON_BN_CLICKED(IDC_ABL_BUTTON_CANCEL_MAT_2, OnBnClickedAblButtonCancelMat2)
END_MESSAGE_MAP()


// CPageABL message handlers
BOOL CPageABL::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	std::string strPIECE[PIECE_SIZE];
	for( int i=0; i<PIECE_SIZE; ++i )
	{
		strPIECE[i] = SKINTYPE_STRING::m_szPIECETYPE[i];
	}

	std::string strSLOT[EMPEACE_WEAPON_SIZE];
	for( int i=0; i<EMPEACE_WEAPON_SIZE; ++i )
	{
		strSLOT[i] = SKINTYPE_STRING::m_szWHEREBACKTYPE[i];
	}

	SetWin_Combo_Init ( this, IDC_ABL_COMBO_STRIKE, strPIECE, PIECE_SIZE );
	SetWin_Combo_Init ( this, IDC_ABL_COMBO_PTYPE, strPIECE, PIECE_SIZE );
	SetWin_Combo_Init ( this, IDC_ABL_COMBO_SLOTB, strSLOT, EMPEACE_WEAPON_SIZE );
	

	ResetTool();

	return TRUE;  
}

void CPageABL::ResetTool()
{
	m_pSkin = NULL;
	m_pData = NULL;

	SetWin_Text( this, IDC_EDIT_ABL_FILENAME, "" );

	SetWin_Combo_Sel( this, IDC_ABL_COMBO_STRIKE, 0 );
	SetWin_Combo_Sel( this, IDC_ABL_COMBO_PTYPE, 0 );
	SetWin_Combo_Sel( this, IDC_ABL_COMBO_SLOTB, 0 );

	SetWin_Text( this, IDC_ABL_EDIT_SKELETON, "" );
	SetWin_Text( this, IDC_ABL_EDIT_BONEDATA, "" );
	SetWin_Text( this, IDC_ABL_EDIT_MASKP, "" );
	SetWin_Text( this, IDC_ABL_EDIT_BONELINK, "" );

	SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_X_1, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Y_1, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Z_1, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_X_1, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Y_1, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Z_1, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_SCALE_X_1, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_SCALE_Y_1, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_SCALE_Z_1, 1.0f );

	SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_X_2, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Y_2, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Z_2, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_X_2, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Y_2, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Z_2, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_SCALE_X_2, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_SCALE_Y_2, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_SCALE_Z_2, 1.0f );

	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_X_1, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_Y_1, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_Z_1, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_X_1, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_Y_1, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_Z_1, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_SCALE_X_1, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_SCALE_Y_1, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_SCALE_Z_1, FALSE );

	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_X_2, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_Y_2, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_Z_2, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_X_2, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_Y_2, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_Z_2, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_SCALE_X_2, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_SCALE_Y_2, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_SCALE_Z_2, FALSE );

	SetWin_Enable( this, IDC_ABL_BUTTON_TEST_MAT_1, FALSE );
	SetWin_Enable( this, IDC_ABL_BUTTON_SAVE_MAT_1, FALSE );
	SetWin_Enable( this, IDC_ABL_BUTTON_CANCEL_MAT_1, FALSE );

	SetWin_Enable( this, IDC_ABL_BUTTON_TEST_MAT_2, FALSE );
	SetWin_Enable( this, IDC_ABL_BUTTON_SAVE_MAT_2, FALSE );
	SetWin_Enable( this, IDC_ABL_BUTTON_CANCEL_MAT_2, FALSE );
}

void CPageABL::OnBnClickedButtonAblNew()
{
	CEditorSkinCharView::GetView()->ABLNew();

	m_pSkin = CEditorSkinCharView::GetView()->m_pAttBone;
	m_pData = CEditorSkinCharView::GetView()->m_pAttBoneLink;

	if ( m_pSkin && m_pData )
		DataShow();
}

void CPageABL::OnBnClickedButtonAblLoad()
{
	CString szFilter = "Container (*.abl)|*.abl;|";
	CFileDialog dlg ( TRUE,".ABL", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = DxAttBoneLinkContainer::GetInstance().GetPath();
	if ( dlg.DoModal() != IDOK ) return;

	BOOL bOK = CEditorSkinCharView::GetView()->ABLLoad( dlg.GetFileName().GetString() );
	if ( bOK )
	{
		m_pSkin = CEditorSkinCharView::GetView()->m_pAttBone;
		m_pData = CEditorSkinCharView::GetView()->m_pAttBoneLink;
		
		if ( m_pSkin && m_pData )
			DataShow();
	}
}

void CPageABL::OnBnClickedButtonAblSave()
{
	if( !m_pData )	return;
	if( !m_pSkin )	return;

	if( !m_pData->GetData()->GetSkeleton() )	
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Skeleton!" );
		return;
	}

	if( !m_pSkin->m_pSkeletonLink )	
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid SkeletonLink!" );
		return;
	}

	if ( m_pData->m_strAttBoneData.size() == 0 )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid BoneDataFile!" );
		return;
	}

	if ( m_pData->m_strBoneTrans.size() == 0 )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid BoneLink!" );
		return;
	}


	m_pData->m_affBoneTrans = *m_pSkin->m_affBoneTrans.m_pAffineParts;
	m_pData->m_affPeaceZone = *m_pSkin->m_affPeaceZone.m_pAffineParts;

	CString szFilter = "Container (*.abl)|*.abl;|";
	CFileDialog dlg(FALSE,"*.*", m_pData->GetFileName(), OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = DxAttBoneLinkContainer::GetInstance().GetPath();
	if ( dlg.DoModal() != IDOK ) return;

	BOOL bOK = CEditorSkinCharView::GetView()->ABLSave( dlg.GetFileName().GetString() );
	if ( bOK )
	{
		if ( m_pSkin && m_pData )
			DataShow();
	}
}

void CPageABL::DataShow()
{
	if ( !m_pData )	return;

	SetWin_Text( this, IDC_EDIT_ABL_FILENAME, m_pData->GetFileName() );

	SetWin_Combo_Sel( this, IDC_ABL_COMBO_STRIKE, m_pData->m_emPieceStrike );
	SetWin_Combo_Sel( this, IDC_ABL_COMBO_PTYPE, m_pData->m_emPieceType );
	SetWin_Combo_Sel( this, IDC_ABL_COMBO_SLOTB, m_pData->m_emWeaponWhereBack );

	SetWin_Text( this, IDC_ABL_EDIT_SKELETON, m_pData->m_strSkeleton.c_str() );
	SetWin_Text( this, IDC_ABL_EDIT_BONEDATA, m_pData->m_strAttBoneData.c_str() );

	SetWin_Text( this, IDC_ABL_EDIT_MASKP, m_pData->m_strMaskPiece.c_str() );
	SetWin_Text( this, IDC_ABL_EDIT_BONELINK, m_pData->m_strBoneTrans.c_str() );


	CEditorSkinCharView::GetView()->EditSmatSet( FALSE );

	ManualMatrixReset1();
	ManualMatrixReset2();
}

void CPageABL::OnBnClickedAblButtonSkeleton()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	int nRETURN = CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Change Skeleton? BoneLink will be cleared!" );
	if ( nRETURN != IDYES )	return;

	CString szFilterInput = "Skeleton (*.x)|*.x;|";
	CFileDialog dlgInput(true,"*.X*",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilterInput, this );
	dlgInput.m_ofn.lpstrInitialDir = DxBoneCollector::GetInstance().GetPath();
	if ( dlgInput.DoModal() != IDOK ) return;

	std::string strSkeleton = dlgInput.GetFileName();
	DxSkeleton* pSkeleton = DxBoneCollector::GetInstance().Load( strSkeleton.c_str(), CEditorSkinCharView::GetView()->Get3DDevice() );
	if ( pSkeleton )
	{
		m_pSkin->m_pSkeletonLink = pSkeleton;
		m_pSkin->m_strSkeletonLink = pSkeleton->szXFileName;
		m_pData->m_strSkeleton = pSkeleton->szXFileName;

		m_pData->m_strBoneTrans = "";
		m_pSkin->m_strBoneTrans = "";

		m_pSkin->m_strMaskPiece = "";
		m_pData->m_strMaskPiece = "";
		CEditorSkinCharView::GetView()->m_pMaskPiece = NULL;

		DataShow();
	}
}

void CPageABL::OnBnClickedAblButtonBonedata()
{
	CString szFilter = "Container (*.abf)|*.abf;|";
	CFileDialog dlg ( TRUE,".ABF", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = DxAttBoneDataContainer::GetInstance().GetPath();
	if ( dlg.DoModal() != IDOK ) return;

	DxAttBoneData* pData = DxAttBoneDataContainer::GetInstance().LoadData( dlg.GetFileName(), CEditorSkinCharView::GetView()->Get3DDevice(), TRUE );
	if ( pData )
	{
		m_pData->SetData( pData );
		m_pSkin->SetCharData( m_pData, CEditorSkinCharView::GetView()->Get3DDevice(), 0, TRUE );

		m_pSkin->InitDeviceObjects( CEditorSkinCharView::GetView()->Get3DDevice() );
		m_pSkin->RestoreDeviceObjects( CEditorSkinCharView::GetView()->Get3DDevice() );
		m_pSkin->DEFAULTANI();
		m_pSkin->ResetAnimationCore();

		if ( m_pSkin->GetSkeleton() && m_pSkin->GetSkeleton()->pBoneRoot )
			m_pSkin->GetSkeleton()->pBoneRoot->ResetBone();

		if ( m_pSkin && m_pData )
			DataShow();
	}
}

void CPageABL::OnBnClickedAblButtonMaskp()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	CString szFilterInput = "Piece (*.cps,*.aps,*.vps)|*.cps;*.aps;*.vps|";
	CFileDialog dlg(true,"*.*",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilterInput,this);
	dlg.m_ofn.lpstrInitialDir = DxSkinPieceContainer::GetInstance().GetPath();
	if ( dlg.DoModal() != IDOK ) return;

	PDXSKINPIECE pPiece = DxSkinPieceContainer::GetInstance().LoadPiece( dlg.GetFileName(), CEditorSkinCharView::GetView()->Get3DDevice(), TRUE );
	if ( pPiece )
	{
		m_pSkin->m_strMaskPiece = pPiece->m_szFileName;
		m_pData->m_strMaskPiece = pPiece->m_szFileName;

		CEditorSkinCharView::GetView()->m_pMaskPiece = pPiece;

		if ( m_pSkin && m_pData )
			DataShow();
	}
}

void CPageABL::OnBnClickedAblButtonBonelink()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	if ( !g_strPICK_BONE.size() )	return;

	int nRETURN = CDebugSet::MsgBoxYesNo( GetSafeHwnd(), "Change BoneLink to :%s ?", g_strPICK_BONE.c_str() );
	if ( nRETURN != IDYES )	return;

	m_pSkin->m_strBoneTrans = g_strPICK_BONE.c_str();
	m_pData->m_strBoneTrans = g_strPICK_BONE.c_str();

	DataShow();
}

void CPageABL::OnBnClickedAblButtonEditmat1()
{
	if( !m_pSkin )	return;

	CEditorSkinCharView::GetView()->EditSmatSet( TRUE );
	CEditorSkinCharView::GetView()->m_DxEditMat.SetAffineMatrix( &m_pSkin->m_affBoneTrans );
	CEditorSkinCharView::GetView()->m_bAniAttack = TRUE;

	if ( m_pSkin->m_pSkeletonLink )
	{
		DxBoneTrans *pBoneCur = m_pSkin->m_pSkeletonLink->FindBone ( m_pSkin->m_strBoneTrans.c_str() );
		if ( pBoneCur )
			CEditorSkinCharView::GetView()->m_DxEditMat.SetFrameMatrix( pBoneCur->matCombined );
	}

	ManualMatrixReset1();
}

void CPageABL::OnBnClickedAblButtonEditmat2()
{
	if( !m_pSkin )	return;

	CEditorSkinCharView::GetView()->EditSmatSet( TRUE );
	CEditorSkinCharView::GetView()->m_DxEditMat.SetAffineMatrix( &m_pSkin->m_affPeaceZone );
	CEditorSkinCharView::GetView()->m_bAniAttack = FALSE;

	if ( m_pSkin->m_pSkeletonLink )
	{
		DxBoneTrans *pBoneCur = m_pSkin->m_pSkeletonLink->FindBone ( SKINTYPE_STRING::m_szWHEREBACKTYPE[ m_pSkin->m_emWeaponWhereBack ] );
		if ( pBoneCur )
			CEditorSkinCharView::GetView()->m_DxEditMat.SetFrameMatrix( pBoneCur->matCombined );
	}

	ManualMatrixReset2();
}

void CPageABL::OnBnClickedAblButtonResetMat01()
{
	if( !m_pSkin )	return;
	m_pSkin->m_affBoneTrans.UseAffineMatrix();

	DataShow();
}

void CPageABL::OnBnClickedAblButtonResetMat02()
{
	if( !m_pSkin )	return;
	m_pSkin->m_affPeaceZone.UseAffineMatrix();

	DataShow();
}

void CPageABL::OnBnClickedAblButtonMaskpReset()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	m_pSkin->m_strMaskPiece = "";
	m_pData->m_strMaskPiece = "";

	CEditorSkinCharView::GetView()->m_pMaskPiece = NULL;

	DataShow();
}

void CPageABL::OnCbnSelchangeAblComboStrike()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	EMPIECECHAR emPIECE = static_cast<EMPIECECHAR>( GetWin_Combo_Sel( this, IDC_ABL_COMBO_STRIKE ) );
	m_pSkin->m_emPieceStrike = emPIECE;
	m_pData->m_emPieceStrike = emPIECE;

	DataShow();
}

void CPageABL::OnCbnSelchangeAblComboPtype()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	EMPIECECHAR emPIECE = static_cast<EMPIECECHAR>( GetWin_Combo_Sel( this, IDC_ABL_COMBO_PTYPE ) );
	m_pSkin->m_emPieceType = emPIECE;
	m_pData->m_emPieceType = emPIECE;

	DataShow();
}

void CPageABL::OnCbnSelchangeAblComboSlotb()
{
	if ( !m_pSkin )	return;
	if ( !m_pData )	return;

	EMPEACEZONEWEAPON emZone = static_cast<EMPEACEZONEWEAPON>( GetWin_Combo_Sel( this, IDC_ABL_COMBO_SLOTB ) );
	m_pSkin->m_emWeaponWhereBack = emZone;
	m_pData->m_emWeaponWhereBack = emZone;

	DataShow();
}

void CPageABL::OnBnClickedAblButtonAnimPlay()
{
	if ( !m_pSkin )	return;
	m_pSkin->DEFAULTANI();
}

void CPageABL::OnBnClickedAblButtonAnimStop()
{
	if ( !m_pSkin )	return;

	m_pSkin->ResetAnimationCore();

	if ( m_pSkin->GetSkeleton() && m_pSkin->GetSkeleton()->pBoneRoot )
		m_pSkin->GetSkeleton()->pBoneRoot->ResetBone();

}

void CPageABL::OnBnClickedAblButtonManualMat1()
{
	CEditorSkinCharView::GetView()->EditSmatSet( FALSE );
	CEditorSkinCharView::GetView()->m_bAniAttack = TRUE;
	ManualMatrixShow1();
}

void CPageABL::OnBnClickedAblButtonManualMat2()
{
	CEditorSkinCharView::GetView()->EditSmatSet( FALSE );
	CEditorSkinCharView::GetView()->m_bAniAttack = FALSE;
	ManualMatrixShow2();
}

void CPageABL::OnBnClickedAblButtonTestMat1()
{
	ManualMatrixSave1();
}

void CPageABL::OnBnClickedAblButtonTestMat2()
{
	ManualMatrixSave2();
}

void CPageABL::OnBnClickedAblButtonSaveMat1()
{
	ManualMatrixSave1();
	ManualMatrixReset1();
}

void CPageABL::OnBnClickedAblButtonSaveMat2()
{
	ManualMatrixSave2();
	ManualMatrixReset2();
}

void CPageABL::OnBnClickedAblButtonCancelMat1()
{
	ManualMatrixReset1();
}

void CPageABL::OnBnClickedAblButtonCancelMat2()
{
	ManualMatrixReset2();
}

void CPageABL::ManualMatrixReset1()
{
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_X_1, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_Y_1, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_Z_1, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_X_1, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_Y_1, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_Z_1, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_SCALE_X_1, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_SCALE_Y_1, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_SCALE_Z_1, FALSE );

	SetWin_Enable( this, IDC_ABL_BUTTON_TEST_MAT_1, FALSE );
	SetWin_Enable( this, IDC_ABL_BUTTON_SAVE_MAT_1, FALSE );
	SetWin_Enable( this, IDC_ABL_BUTTON_CANCEL_MAT_1, FALSE );

	if ( m_pSkin )
	{
		SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_X_1, m_pSkin->m_affBoneTrans.m_pAffineParts->vTrans.x );
		SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Y_1, m_pSkin->m_affBoneTrans.m_pAffineParts->vTrans.y );
		SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Z_1, m_pSkin->m_affBoneTrans.m_pAffineParts->vTrans.z );
		SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_X_1, m_pSkin->m_affBoneTrans.m_pAffineParts->vRotate.x );
		SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Y_1, m_pSkin->m_affBoneTrans.m_pAffineParts->vRotate.y );
		SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Z_1, m_pSkin->m_affBoneTrans.m_pAffineParts->vRotate.z );
		SetWin_Num_float( this, IDC_EDIT_MATRIX_SCALE_X_1, m_pSkin->m_affBoneTrans.m_pAffineParts->vScale.x );
		SetWin_Num_float( this, IDC_EDIT_MATRIX_SCALE_Y_1, m_pSkin->m_affBoneTrans.m_pAffineParts->vScale.y );
		SetWin_Num_float( this, IDC_EDIT_MATRIX_SCALE_Z_1, m_pSkin->m_affBoneTrans.m_pAffineParts->vScale.z );
	}
}

void CPageABL::ManualMatrixShow1()
{
	if ( !m_pSkin )	return;

	SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_X_1, m_pSkin->m_affBoneTrans.m_pAffineParts->vTrans.x );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Y_1, m_pSkin->m_affBoneTrans.m_pAffineParts->vTrans.y );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Z_1, m_pSkin->m_affBoneTrans.m_pAffineParts->vTrans.z );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_X_1, m_pSkin->m_affBoneTrans.m_pAffineParts->vRotate.x );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Y_1, m_pSkin->m_affBoneTrans.m_pAffineParts->vRotate.y );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Z_1, m_pSkin->m_affBoneTrans.m_pAffineParts->vRotate.z );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_SCALE_X_1, m_pSkin->m_affBoneTrans.m_pAffineParts->vScale.x );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_SCALE_Y_1, m_pSkin->m_affBoneTrans.m_pAffineParts->vScale.y );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_SCALE_Z_1, m_pSkin->m_affBoneTrans.m_pAffineParts->vScale.z );

	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_X_1, TRUE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_Y_1, TRUE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_Z_1, TRUE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_X_1, TRUE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_Y_1, TRUE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_Z_1, TRUE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_SCALE_X_1, TRUE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_SCALE_Y_1, TRUE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_SCALE_Z_1, TRUE );

	SetWin_Enable( this, IDC_ABL_BUTTON_TEST_MAT_1, TRUE );
	SetWin_Enable( this, IDC_ABL_BUTTON_SAVE_MAT_1, TRUE );
	SetWin_Enable( this, IDC_ABL_BUTTON_CANCEL_MAT_1, TRUE );
}

void CPageABL::ManualMatrixSave1()
{
	if ( !m_pSkin )	return;

	m_pSkin->m_affBoneTrans.m_pAffineParts->vTrans.x = GetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_X_1 );
	m_pSkin->m_affBoneTrans.m_pAffineParts->vTrans.y = GetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Y_1 );
	m_pSkin->m_affBoneTrans.m_pAffineParts->vTrans.z = GetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Z_1 );
	m_pSkin->m_affBoneTrans.m_pAffineParts->vRotate.x = GetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_X_1 );
	m_pSkin->m_affBoneTrans.m_pAffineParts->vRotate.y = GetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Y_1 );
	m_pSkin->m_affBoneTrans.m_pAffineParts->vRotate.z = GetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Z_1 );
	m_pSkin->m_affBoneTrans.m_pAffineParts->vScale.x = GetWin_Num_float( this, IDC_EDIT_MATRIX_SCALE_X_1 );
	m_pSkin->m_affBoneTrans.m_pAffineParts->vScale.y = GetWin_Num_float( this, IDC_EDIT_MATRIX_SCALE_Y_1 );
	m_pSkin->m_affBoneTrans.m_pAffineParts->vScale.z = GetWin_Num_float( this, IDC_EDIT_MATRIX_SCALE_Z_1 );
}

void CPageABL::ManualMatrixReset2()
{
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_X_2, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_Y_2, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_Z_2, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_X_2, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_Y_2, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_Z_2, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_SCALE_X_2, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_SCALE_Y_2, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_SCALE_Z_2, FALSE );

	SetWin_Enable( this, IDC_ABL_BUTTON_TEST_MAT_2, FALSE );
	SetWin_Enable( this, IDC_ABL_BUTTON_SAVE_MAT_2, FALSE );
	SetWin_Enable( this, IDC_ABL_BUTTON_CANCEL_MAT_2, FALSE );

	if ( m_pSkin )
	{
		SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_X_2, m_pSkin->m_affPeaceZone.m_pAffineParts->vTrans.x );
		SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Y_2, m_pSkin->m_affPeaceZone.m_pAffineParts->vTrans.y );
		SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Z_2, m_pSkin->m_affPeaceZone.m_pAffineParts->vTrans.z );
		SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_X_2, m_pSkin->m_affPeaceZone.m_pAffineParts->vRotate.x );
		SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Y_2, m_pSkin->m_affPeaceZone.m_pAffineParts->vRotate.y );
		SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Z_2, m_pSkin->m_affPeaceZone.m_pAffineParts->vRotate.z );
		SetWin_Num_float( this, IDC_EDIT_MATRIX_SCALE_X_2, m_pSkin->m_affPeaceZone.m_pAffineParts->vScale.x );
		SetWin_Num_float( this, IDC_EDIT_MATRIX_SCALE_Y_2, m_pSkin->m_affPeaceZone.m_pAffineParts->vScale.y );
		SetWin_Num_float( this, IDC_EDIT_MATRIX_SCALE_Z_2, m_pSkin->m_affPeaceZone.m_pAffineParts->vScale.z );
	}
}

void CPageABL::ManualMatrixShow2()
{
	if ( !m_pSkin )	return;

	SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_X_2, m_pSkin->m_affPeaceZone.m_pAffineParts->vTrans.x );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Y_2, m_pSkin->m_affPeaceZone.m_pAffineParts->vTrans.y );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Z_2, m_pSkin->m_affPeaceZone.m_pAffineParts->vTrans.z );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_X_2, m_pSkin->m_affPeaceZone.m_pAffineParts->vRotate.x );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Y_2, m_pSkin->m_affPeaceZone.m_pAffineParts->vRotate.y );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Z_2, m_pSkin->m_affPeaceZone.m_pAffineParts->vRotate.z );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_SCALE_X_2, m_pSkin->m_affPeaceZone.m_pAffineParts->vScale.x );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_SCALE_Y_2, m_pSkin->m_affPeaceZone.m_pAffineParts->vScale.y );
	SetWin_Num_float( this, IDC_EDIT_MATRIX_SCALE_Z_2, m_pSkin->m_affPeaceZone.m_pAffineParts->vScale.z );

	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_X_2, TRUE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_Y_2, TRUE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_TRANS_Z_2, TRUE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_X_2, TRUE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_Y_2, TRUE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_ROTATE_Z_2, TRUE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_SCALE_X_2, TRUE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_SCALE_Y_2, TRUE );
	SetWin_Enable( this, IDC_EDIT_MATRIX_SCALE_Z_2, TRUE );

	SetWin_Enable( this, IDC_ABL_BUTTON_TEST_MAT_2, TRUE );
	SetWin_Enable( this, IDC_ABL_BUTTON_SAVE_MAT_2, TRUE );
	SetWin_Enable( this, IDC_ABL_BUTTON_CANCEL_MAT_2, TRUE );
}

void CPageABL::ManualMatrixSave2()
{
	if ( !m_pSkin )	return;

	m_pSkin->m_affPeaceZone.m_pAffineParts->vTrans.x = GetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_X_2 );
	m_pSkin->m_affPeaceZone.m_pAffineParts->vTrans.y = GetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Y_2 );
	m_pSkin->m_affPeaceZone.m_pAffineParts->vTrans.z = GetWin_Num_float( this, IDC_EDIT_MATRIX_TRANS_Z_2 );
	m_pSkin->m_affPeaceZone.m_pAffineParts->vRotate.x = GetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_X_2 );
	m_pSkin->m_affPeaceZone.m_pAffineParts->vRotate.y = GetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Y_2 );
	m_pSkin->m_affPeaceZone.m_pAffineParts->vRotate.z = GetWin_Num_float( this, IDC_EDIT_MATRIX_ROTATE_Z_2 );
	m_pSkin->m_affPeaceZone.m_pAffineParts->vScale.x = GetWin_Num_float( this, IDC_EDIT_MATRIX_SCALE_X_2 );
	m_pSkin->m_affPeaceZone.m_pAffineParts->vScale.y = GetWin_Num_float( this, IDC_EDIT_MATRIX_SCALE_Y_2 );
	m_pSkin->m_affPeaceZone.m_pAffineParts->vScale.z = GetWin_Num_float( this, IDC_EDIT_MATRIX_SCALE_Z_2 );
}