// PageEff_23_BonePos.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkinPiece.h"
#include "EditorSkinPieceView.h"
#include "PageEff_23_BonePos.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "SelColorDlg.h"
#include "DxMethods.h"

extern std::string	g_strPICK_BONE;

// CPageEff_23_BonePos dialog

IMPLEMENT_DYNAMIC(CPageEff_23_BonePos, CPropertyPage)

CPageEff_23_BonePos::CPageEff_23_BonePos(LOGFONT logfont)
	: CPropertyPage(CPageEff_23_BonePos::IDD)
	, m_pFont ( NULL )
	, m_pPiece( NULL )
	, m_pEff( NULL )
	, m_bADD( FALSE )
{
	D3DXMatrixIdentity ( &m_matEdit );
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageEff_23_BonePos::~CPageEff_23_BonePos()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEff_23_BonePos::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPageEff_23_BonePos, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_CBP_TEXTURE, OnBnClickedButtonCbpTexture)
	ON_BN_CLICKED(IDC_BUTTON_CBP_COLOR, OnBnClickedButtonCbpColor)
	ON_BN_CLICKED(IDC_BUTTON_CBP_SELBONE, OnBnClickedButtonCbpSelbone)
	ON_BN_CLICKED(IDC_CHECK_CBP_USE_OFFSET, OnBnClickedCheckCbpUseOffset)
	ON_BN_CLICKED(IDC_BUTTON_CBP_GETOFFSET, OnBnClickedButtonCbpGetoffset)
	ON_BN_CLICKED(IDC_BUTTON_CBP_RESETOFFSET, OnBnClickedButtonCbpResetoffset)
	ON_BN_CLICKED(IDC_BUTTON_CBP_EDITOFFSET, OnBnClickedButtonCbpEditoffset)
END_MESSAGE_MAP()


// CPageEff_23_BonePos message handlers
BOOL CPageEff_23_BonePos::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	ResetTool();

	return TRUE;
}

void CPageEff_23_BonePos::DataSet( DxSkinPiece* pPiece, DxEffCharBonePosEff* pEff, BOOL bADD )
{
	m_pPiece = pPiece;
	m_pEff = pEff;
	m_bADD = bADD;

	ResetTool();
	DataShow();
	LocalPosShow();
	CEditorSkinPieceView::GetView()->m_bRenderBone = TRUE;
}

void CPageEff_23_BonePos::ResetTool()
{
	SetWin_Num_int( this, IDC_EDIT_CBP_PARTICLEPERSEC, 1 );

	SetWin_Num_float( this, IDC_EDIT_CBP_CREATELENGTH, 1.0f );

	SetWin_Num_float( this, IDC_EDIT_CBP_LIFE_MIN, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CBP_LIFE_MAX, 1.0f );

	SetWin_Num_float( this, IDC_EDIT_CBP_SCALE_X, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CBP_SCALE_Y, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CBP_SCALE_Z, 1.0f );

	SetWin_Num_float( this, IDC_EDIT_CBP_VELO_X, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CBP_VELO_Y, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CBP_VELO_Z, 1.0f );

	SetWin_Check( this, IDC_CHECK_CBP_ROTATE, FALSE );
	SetWin_Num_float( this, IDC_EDIT_CBP_ROTATE_MIN, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CBP_ROTATE_MAX, 1.0f );

	SetWin_Check( this, IDC_CHECK_CBP_SEQUENCE, FALSE );
	SetWin_Num_int( this, IDC_EDIT_CBP_COL, 1 );
	SetWin_Num_int( this, IDC_EDIT_CBP_ROW, 1 );

	SetWin_Text( this, IDC_EDIT_CBP_TEXTURE, "" );

	SetWin_Num_int( this, IDC_EDIT_CBP_COLOR_A, 1 );
	SetWin_Num_int( this, IDC_EDIT_CBP_COLOR_R, 1 );
	SetWin_Num_int( this, IDC_EDIT_CBP_COLOR_G, 1 );
	SetWin_Num_int( this, IDC_EDIT_CBP_COLOR_B, 1 );

	SetWin_Check( this, IDC_CHECK_CBP_USE_REMAIN, FALSE );

	SetWin_Text( this, IDC_EDIT_CBP_BONE, "" );

	SetWin_Check( this, IDC_CHECK_CBP_USE_OFFSET, FALSE );
	SetWin_Num_float( this, IDC_EDIT_CBP_OFFSET_X, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CBP_OFFSET_Y, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CBP_OFFSET_Z, 1.0f );

	SetWin_Enable( this, IDC_BUTTON_CBP_GETOFFSET, FALSE );
	SetWin_Enable( this, IDC_BUTTON_CBP_RESETOFFSET, FALSE );
}

void CPageEff_23_BonePos::DataShow()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	EFFCHAR_PROPERTY_BONEPOSEFF* pProp = (EFFCHAR_PROPERTY_BONEPOSEFF*)m_pEff->GetProperty();

	SetWin_Num_int( this, IDC_EDIT_CBP_PARTICLEPERSEC, pProp->m_dwParticlePec );
	SetWin_Num_float( this, IDC_EDIT_CBP_CREATELENGTH, pProp->m_fCreateLength );

	SetWin_Num_float( this, IDC_EDIT_CBP_LIFE_MIN, pProp->m_vLife.x );
	SetWin_Num_float( this, IDC_EDIT_CBP_LIFE_MAX, pProp->m_vLife.y );

	SetWin_Num_float( this, IDC_EDIT_CBP_SCALE_X, pProp->m_vSize.x );
	SetWin_Num_float( this, IDC_EDIT_CBP_SCALE_Y, pProp->m_vSize.y );
	SetWin_Num_float( this, IDC_EDIT_CBP_SCALE_Z, pProp->m_vSize.z );

	SetWin_Num_float( this, IDC_EDIT_CBP_VELO_X, pProp->m_vVelocity.x );
	SetWin_Num_float( this, IDC_EDIT_CBP_VELO_Y, pProp->m_vVelocity.y );
	SetWin_Num_float( this, IDC_EDIT_CBP_VELO_Z, pProp->m_vVelocity.z );

	SetWin_Check( this, IDC_CHECK_CBP_ROTATE, pProp->m_dwFlag&USE_ROTATE );
	SetWin_Num_float( this, IDC_EDIT_CBP_ROTATE_MIN, pProp->m_vRotate.x );
	SetWin_Num_float( this, IDC_EDIT_CBP_ROTATE_MAX, pProp->m_vRotate.y );

	SetWin_Check( this, IDC_CHECK_CBP_SEQUENCE, pProp->m_dwFlag&USE_SEQUENCE );
	SetWin_Num_int( this, IDC_EDIT_CBP_COL, pProp->m_dwSequenceCol );
	SetWin_Num_int( this, IDC_EDIT_CBP_ROW, pProp->m_dwSequenceRow );

	SetWin_Text( this, IDC_EDIT_CBP_TEXTURE, pProp->m_szTexture );

	SetWin_Num_int( this, IDC_EDIT_CBP_COLOR_R, RGBA_GETRED( pProp->m_dwColor ) );
	SetWin_Num_int( this, IDC_EDIT_CBP_COLOR_G, RGBA_GETGREEN( pProp->m_dwColor ) );
	SetWin_Num_int( this, IDC_EDIT_CBP_COLOR_B, RGBA_GETBLUE( pProp->m_dwColor ) );
	SetWin_Num_int( this, IDC_EDIT_CBP_COLOR_A, RGBA_GETALPHA( pProp->m_dwColor ) );

	SetWin_Check( this, IDC_CHECK_CBP_USE_REMAIN, pProp->m_dwFlag&USE_REMAIN );

	SetWin_Text( this, IDC_EDIT_CBP_BONE, pProp->m_szBoneName );

	SetWin_Check( this, IDC_CHECK_CBP_USE_OFFSET, pProp->m_dwFlag&USE_BONEOFFSETPOS );
	SetWin_Num_float( this, IDC_EDIT_CBP_OFFSET_X, pProp->m_vLocalPos.x );
	SetWin_Num_float( this, IDC_EDIT_CBP_OFFSET_Y, pProp->m_vLocalPos.y );
	SetWin_Num_float( this, IDC_EDIT_CBP_OFFSET_Z, pProp->m_vLocalPos.z );
}

BOOL CPageEff_23_BonePos::DataSave()
{
	if ( !m_pPiece )	return FALSE;
	if ( !m_pEff )		return FALSE;

	EFFCHAR_PROPERTY_BONEPOSEFF sProperty;
	sProperty.m_dwParticlePec = GetWin_Num_int( this, IDC_EDIT_CBP_PARTICLEPERSEC );
	sProperty.m_fCreateLength = GetWin_Num_float( this, IDC_EDIT_CBP_CREATELENGTH );

	sProperty.m_vLife.x = GetWin_Num_float( this, IDC_EDIT_CBP_LIFE_MIN );
	sProperty.m_vLife.y = GetWin_Num_float( this, IDC_EDIT_CBP_LIFE_MAX );

	sProperty.m_vSize.x = GetWin_Num_float( this, IDC_EDIT_CBP_SCALE_X );
	sProperty.m_vSize.y = GetWin_Num_float( this, IDC_EDIT_CBP_SCALE_Y );
	sProperty.m_vSize.z = GetWin_Num_float( this, IDC_EDIT_CBP_SCALE_Z );

	sProperty.m_vVelocity.x = GetWin_Num_float( this, IDC_EDIT_CBP_VELO_X );
	sProperty.m_vVelocity.y = GetWin_Num_float( this, IDC_EDIT_CBP_VELO_Y );
	sProperty.m_vVelocity.z = GetWin_Num_float( this, IDC_EDIT_CBP_VELO_Z );

	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CBP_ROTATE ), sProperty.m_dwFlag, USE_ROTATE );
	sProperty.m_vRotate.x = GetWin_Num_float( this, IDC_EDIT_CBP_ROTATE_MIN );
	sProperty.m_vRotate.y = GetWin_Num_float( this, IDC_EDIT_CBP_ROTATE_MAX );

	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CBP_SEQUENCE ), sProperty.m_dwFlag, USE_SEQUENCE );
	sProperty.m_dwSequenceCol = GetWin_Num_int( this, IDC_EDIT_CBP_COL );
	sProperty.m_dwSequenceRow = GetWin_Num_int( this, IDC_EDIT_CBP_ROW );

	std::string strTex = GetWin_Text( this, IDC_EDIT_CBP_TEXTURE );
	StringCchCopy( sProperty.m_szTexture , BONEPOSEFF_TEXNAME_SIZE, strTex.c_str() );

	sProperty.m_dwColor = RGBA_MAKE(
		GetWin_Num_int( this, IDC_EDIT_CBP_COLOR_R ), 
		GetWin_Num_int( this, IDC_EDIT_CBP_COLOR_G ), 
		GetWin_Num_int( this, IDC_EDIT_CBP_COLOR_B ),
		GetWin_Num_int( this, IDC_EDIT_CBP_COLOR_A ));

	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CBP_USE_REMAIN ), sProperty.m_dwFlag, USE_REMAIN );

	std::string strBone = GetWin_Text( this, IDC_EDIT_CBP_BONE );
	StringCchCopy( sProperty.m_szBoneName , BONEPOSEFF_TEXNAME_SIZE, strBone.c_str() );

	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CBP_USE_OFFSET ), sProperty.m_dwFlag, USE_BONEOFFSETPOS );
	sProperty.m_vLocalPos.x = GetWin_Num_float( this, IDC_EDIT_CBP_OFFSET_X );
	sProperty.m_vLocalPos.y = GetWin_Num_float( this, IDC_EDIT_CBP_OFFSET_Y );
	sProperty.m_vLocalPos.z = GetWin_Num_float( this, IDC_EDIT_CBP_OFFSET_Z );

	m_pEff->SetProperty( &sProperty );

	HRESULT hr = m_pEff->Create( CEditorSkinPieceView::GetView()->Get3DDevice() );
	if ( FAILED(hr) )
	{
		return FALSE;
	}

	if ( m_bADD )
	{
		m_pPiece->AddEffList( m_pEff );
		m_bADD = FALSE;
	}

	return TRUE;
}

void CPageEff_23_BonePos::LocalPosShow()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	EFFCHAR_PROPERTY_BONEPOSEFF* pProp = (EFFCHAR_PROPERTY_BONEPOSEFF*)m_pEff->GetProperty();
	SetWin_Num_float( this, IDC_EDIT_CBP_OFFSET_X, pProp->m_vLocalPos.x );
	SetWin_Num_float( this, IDC_EDIT_CBP_OFFSET_Y, pProp->m_vLocalPos.y );
	SetWin_Num_float( this, IDC_EDIT_CBP_OFFSET_Z, pProp->m_vLocalPos.z );

	SetWin_Enable( this, IDC_BUTTON_CBP_GETOFFSET, FALSE );
	SetWin_Enable( this, IDC_BUTTON_CBP_RESETOFFSET, FALSE );

	CEditorSkinPieceView::GetView()->m_bEditMRS = FALSE;
	CEditorSkinPieceView::GetView()->m_DxEditMRS.SetMatrix( NULL );
	m_matEdit = D3DXMATRIX();
}


void CPageEff_23_BonePos::OnBnClickedButtonApply()
{
	DataSave();
}

void CPageEff_23_BonePos::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		m_pPiece = NULL;
		m_pEff = NULL;

		if ( m_pSheetTab )
			m_pSheetTab->ShowMain();
	}
}

void CPageEff_23_BonePos::OnBnClickedButtonClose()
{
	if ( m_pSheetTab )
	{
		m_pPiece = NULL;
		m_pEff = NULL;

		if ( m_bADD )	
			SAFE_DELETE( m_pEff );

		m_pSheetTab->ShowMain();
	}
}

void CPageEff_23_BonePos::OnBnClickedButtonCbpTexture()
{
	CString szFilter = "Texture Image (*.bmp,*.dds,*.tga,*.jpg)|*.bmp;*.dds;*.tga;*.jpg|";
	CFileDialog dlg(TRUE,".",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = TextureManager::GetTexturePath(); 
	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, IDC_EDIT_CBP_TEXTURE, dlg.GetFileName().GetString() );
}

void CPageEff_23_BonePos::OnBnClickedButtonCbpColor()
{
	BYTE nCUR_R = GetWin_Num_int( this, IDC_EDIT_CBP_COLOR_R );
	BYTE nCUR_G = GetWin_Num_int( this, IDC_EDIT_CBP_COLOR_G );
	BYTE nCUR_B = GetWin_Num_int( this, IDC_EDIT_CBP_COLOR_B );

	CSelColorDlg dlg( nCUR_R, nCUR_G, nCUR_B );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_int( this, IDC_EDIT_CBP_COLOR_R, GetRValue( crNewColor ) );
		SetWin_Num_int( this, IDC_EDIT_CBP_COLOR_G, GetGValue( crNewColor ) );
		SetWin_Num_int( this, IDC_EDIT_CBP_COLOR_B, GetBValue( crNewColor ) );
	}
}

void CPageEff_23_BonePos::OnBnClickedButtonCbpSelbone()
{
	if ( g_strPICK_BONE.size() <= 0 )
	{
		CDebugSet::MsgBox( "No Bone Selected!" );
		return;
	}else{
		SetWin_Text( this, IDC_EDIT_CBP_BONE, g_strPICK_BONE.c_str() );
	}
}

void CPageEff_23_BonePos::OnBnClickedCheckCbpUseOffset()
{
}

void CPageEff_23_BonePos::OnBnClickedButtonCbpGetoffset()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	m_pEff->SetChangeMatrix( m_matEdit );
	LocalPosShow();
}

void CPageEff_23_BonePos::OnBnClickedButtonCbpResetoffset()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	m_pEff->ResetLocalPos();
	LocalPosShow();
}

void CPageEff_23_BonePos::OnBnClickedButtonCbpEditoffset()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	SetWin_Enable( this, IDC_BUTTON_CBP_GETOFFSET, TRUE );
	SetWin_Enable( this, IDC_BUTTON_CBP_RESETOFFSET, TRUE );

	m_matEdit = m_pEff->GetInfluenceMatrix();
	CEditorSkinPieceView::GetView()->m_bEditMRS = TRUE;
	CEditorSkinPieceView::GetView()->m_DxEditMRS.SetMatrix( &m_matEdit );
}
