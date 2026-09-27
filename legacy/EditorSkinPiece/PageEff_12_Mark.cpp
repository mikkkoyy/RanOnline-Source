// PageEff_12_Mark.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkinPiece.h"
#include "EditorSkinPieceView.h"
#include "PageEff_12_Mark.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "SelColorDlg.h"
#include "Global.h"

// CPageEff_12_Mark dialog

IMPLEMENT_DYNAMIC(CPageEff_12_Mark, CPropertyPage)

CPageEff_12_Mark::CPageEff_12_Mark(LOGFONT logfont)
	: CPropertyPage(CPageEff_12_Mark::IDD)
	, m_pFont ( NULL )
	, m_pPiece( NULL )
	, m_pEff( NULL )
	, m_bADD( FALSE )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageEff_12_Mark::~CPageEff_12_Mark()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEff_12_Mark::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPageEff_12_Mark, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_CM_TEXTURE_1, OnBnClickedButtonCmTexture1)
	ON_BN_CLICKED(IDC_BUTTON_CM_COLOR_1, OnBnClickedButtonCmColor1)
	ON_BN_CLICKED(IDC_BUTTON_CM_TEXTURE_2, OnBnClickedButtonCmTexture2)
	ON_BN_CLICKED(IDC_BUTTON_CM_COLOR_2, OnBnClickedButtonCmColor2)
	ON_BN_CLICKED(IDC_CHECK_CM_USE_1, OnBnClickedCheckCmUse1)
	ON_BN_CLICKED(IDC_CHECK_CM_USE_2, OnBnClickedCheckCmUse2)
END_MESSAGE_MAP()


// CPageEff_12_Mark message handlers
BOOL CPageEff_12_Mark::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	SetWin_Combo_Init( this, IDC_COMBO_CM_BLEND, GLOBAL_EFFTOOL::strEFFMARK_BLEND, TOOL_EFFMARK_BLEND_SIZE );
	ResetTool();

	return TRUE;
}

void CPageEff_12_Mark::DataSet( DxSkinPiece* pPiece, DxEffCharMark* pEff, BOOL bADD )
{
	m_pPiece = pPiece;
	m_pEff = pEff;
	m_bADD = bADD;

	ResetTool();
	DataShow();
}

void CPageEff_12_Mark::ResetTool()
{
	SetWin_Combo_Sel( this, IDC_COMBO_CM_BLEND, 0 );

	SetWin_Check( this, IDC_CHECK_CM_USE_1, FALSE );
	SetWin_Check( this, IDC_CHECK_CM_USE_2, FALSE );

	SetWin_Text( this, IDC_EDIT_CM_TEXTURE_1, "" );
	SetWin_Text( this, IDC_EDIT_CM_TEXTURE_2, "" );

	SetWin_Num_float( this, IDC_EDIT_CM_COLOR_A_1, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CM_COLOR_R_1, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CM_COLOR_G_1, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CM_COLOR_B_1, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CM_TEX_X_1, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CM_TEX_Y_1, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CM_TEXUV_X_1, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CM_TEXUV_Y_1, 1.0f );

	SetWin_Num_float( this, IDC_EDIT_CM_COLOR_A_2, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CM_COLOR_R_2, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CM_COLOR_G_2, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CM_COLOR_B_2, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CM_TEX_X_2, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CM_TEX_Y_2, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CM_TEXUV_X_2, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CM_TEXUV_Y_2, 1.0f );
}

void CPageEff_12_Mark::DataShow()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	EFFCHAR_PROPERTY_MARK* pProp = (EFFCHAR_PROPERTY_MARK*)m_pEff->GetProperty();

	SetWin_Combo_Sel( this, IDC_COMBO_CM_BLEND, GLOBAL_EFFTOOL::strEFFMARK_BLEND[pProp->m_nBlend].c_str() );

	SetWin_Check( this, IDC_CHECK_CM_USE_1, ( pProp->m_nTexNum == 1 ) );
	SetWin_Check( this, IDC_CHECK_CM_USE_2, ( pProp->m_nTexNum == 2 ) );

	SetWin_Text( this, IDC_EDIT_CM_TEXTURE_1, pProp->m_szTex1 );
	SetWin_Text( this, IDC_EDIT_CM_TEXTURE_2, pProp->m_szTex2 );

	SetWin_Num_float( this, IDC_EDIT_CM_COLOR_A_1, pProp->m_cDiffuse1.a );
	SetWin_Num_float( this, IDC_EDIT_CM_COLOR_R_1, pProp->m_cDiffuse1.r );
	SetWin_Num_float( this, IDC_EDIT_CM_COLOR_G_1, pProp->m_cDiffuse1.g );
	SetWin_Num_float( this, IDC_EDIT_CM_COLOR_B_1, pProp->m_cDiffuse1.b );
	SetWin_Num_float( this, IDC_EDIT_CM_TEX_X_1, pProp->m_vTex01.x );
	SetWin_Num_float( this, IDC_EDIT_CM_TEX_Y_1, pProp->m_vTex01.y );
	SetWin_Num_float( this, IDC_EDIT_CM_TEXUV_X_1, pProp->m_vTexUV01.x );
	SetWin_Num_float( this, IDC_EDIT_CM_TEXUV_Y_1, pProp->m_vTexUV01.y );
	
	SetWin_Num_float( this, IDC_EDIT_CM_COLOR_A_2, pProp->m_cDiffuse2.a );
	SetWin_Num_float( this, IDC_EDIT_CM_COLOR_R_2, pProp->m_cDiffuse2.r );
	SetWin_Num_float( this, IDC_EDIT_CM_COLOR_G_2, pProp->m_cDiffuse2.g );
	SetWin_Num_float( this, IDC_EDIT_CM_COLOR_B_2, pProp->m_cDiffuse2.b );
	SetWin_Num_float( this, IDC_EDIT_CM_TEX_X_2, pProp->m_vTex02.x );
	SetWin_Num_float( this, IDC_EDIT_CM_TEX_Y_2, pProp->m_vTex02.y );
	SetWin_Num_float( this, IDC_EDIT_CM_TEXUV_X_2, pProp->m_vTexUV02.x );
	SetWin_Num_float( this, IDC_EDIT_CM_TEXUV_Y_2, pProp->m_vTexUV02.y );
}

BOOL CPageEff_12_Mark::DataSave()
{
	if ( !m_pPiece )	return FALSE;
	if ( !m_pEff )		return FALSE;

	BOOL bUse1 = GetWin_Check( this, IDC_CHECK_CM_USE_1 );
	BOOL bUse2 = GetWin_Check( this, IDC_CHECK_CM_USE_2 );
	std::string str1 = GetWin_Text( this, IDC_EDIT_CM_TEXTURE_1 );
	std::string str2 = GetWin_Text( this, IDC_EDIT_CM_TEXTURE_2 );

	EFFCHAR_PROPERTY_MARK sProperty;
	sProperty.m_nTexNum = (bUse1)? 1: (bUse2)? 2:0;
	StringCchCopy( sProperty.m_szTex1, MAX_PATH, str1.c_str() );
	StringCchCopy( sProperty.m_szTex2, MAX_PATH, str2.c_str() );

	sProperty.m_nBlend = GetWin_Combo_Sel( this, IDC_COMBO_CM_BLEND );

	sProperty.m_cDiffuse1.a = GetWin_Num_float( this, IDC_EDIT_CM_COLOR_A_1 );
	sProperty.m_cDiffuse1.r = GetWin_Num_float( this, IDC_EDIT_CM_COLOR_R_1 );
	sProperty.m_cDiffuse1.g = GetWin_Num_float( this, IDC_EDIT_CM_COLOR_G_1 );
	sProperty.m_cDiffuse1.b = GetWin_Num_float( this, IDC_EDIT_CM_COLOR_B_1 );
	sProperty.m_vTex01.x = GetWin_Num_float( this, IDC_EDIT_CM_TEX_X_1 );
	sProperty.m_vTex01.y = GetWin_Num_float( this, IDC_EDIT_CM_TEX_Y_1 );
	sProperty.m_vTexUV01.x = GetWin_Num_float( this, IDC_EDIT_CM_TEXUV_X_1 );
	sProperty.m_vTexUV01.y = GetWin_Num_float( this, IDC_EDIT_CM_TEXUV_Y_1 );

	sProperty.m_cDiffuse2.a = GetWin_Num_float( this, IDC_EDIT_CM_COLOR_A_2 );
	sProperty.m_cDiffuse2.r = GetWin_Num_float( this, IDC_EDIT_CM_COLOR_R_2 );
	sProperty.m_cDiffuse2.g = GetWin_Num_float( this, IDC_EDIT_CM_COLOR_G_2 );
	sProperty.m_cDiffuse2.b = GetWin_Num_float( this, IDC_EDIT_CM_COLOR_B_2 );
	sProperty.m_vTex02.x = GetWin_Num_float( this, IDC_EDIT_CM_TEX_X_2 );
	sProperty.m_vTex02.y = GetWin_Num_float( this, IDC_EDIT_CM_TEX_Y_2 );
	sProperty.m_vTexUV02.x = GetWin_Num_float( this, IDC_EDIT_CM_TEXUV_X_2 );
	sProperty.m_vTexUV02.y = GetWin_Num_float( this, IDC_EDIT_CM_TEXUV_Y_2 );

	m_pEff->SetProperty( &sProperty );
	HRESULT hr = m_pEff->Create( CEditorSkinPieceView::GetView()->Get3DDevice() );
	if ( FAILED(hr) )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "m_pEff->Create Failed! Check Effect Settings!" );
		return FALSE;
	}

	if ( m_bADD )
	{
		m_pPiece->AddEffList( m_pEff );
		m_bADD = FALSE;
	}

	return TRUE;
}

void CPageEff_12_Mark::OnBnClickedButtonApply()
{
	DataSave();
}

void CPageEff_12_Mark::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		m_pPiece = NULL;
		m_pEff = NULL;

		if ( m_pSheetTab )
			m_pSheetTab->ShowMain();
	}
}

void CPageEff_12_Mark::OnBnClickedButtonClose()
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

void CPageEff_12_Mark::OnBnClickedButtonCmTexture1()
{
	CString szFilter = "Texture Image (*.bmp,*.dds,*.tga,*.jpg)|*.bmp;*.dds;*.tga;*.jpg|";
	CFileDialog dlg(TRUE,".",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = TextureManager::GetTexturePath(); 
	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, IDC_EDIT_CM_TEXTURE_1, dlg.GetFileName().GetString() );
}

void CPageEff_12_Mark::OnBnClickedButtonCmColor1()
{
	float fColor_r = GetWin_Num_float( this, IDC_EDIT_CM_COLOR_R_1 );
	float fColor_g = GetWin_Num_float( this, IDC_EDIT_CM_COLOR_G_1 );
	float fColor_b = GetWin_Num_float( this, IDC_EDIT_CM_COLOR_B_1 );

	CSelColorDlg dlg( BYTE( fColor_r * 255.0f ), BYTE( fColor_g * 255.0f ), BYTE( fColor_b * 255.0f ) );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_float( this, IDC_EDIT_CM_COLOR_R_1, float( GetRValue( crNewColor ) / 255.0f ) );
		SetWin_Num_float( this, IDC_EDIT_CM_COLOR_G_1, float( GetGValue( crNewColor ) / 255.0f ) );
		SetWin_Num_float( this, IDC_EDIT_CM_COLOR_B_1, float( GetBValue( crNewColor ) / 255.0f ) );
	}
}

void CPageEff_12_Mark::OnBnClickedButtonCmTexture2()
{
	CString szFilter = "Texture Image (*.bmp,*.dds,*.tga,*.jpg)|*.bmp;*.dds;*.tga;*.jpg|";
	CFileDialog dlg(TRUE,".",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = TextureManager::GetTexturePath(); 
	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, IDC_EDIT_CM_TEXTURE_2, dlg.GetFileName().GetString() );
}

void CPageEff_12_Mark::OnBnClickedButtonCmColor2()
{
	float fColor_r = GetWin_Num_float( this, IDC_EDIT_CM_COLOR_R_2 );
	float fColor_g = GetWin_Num_float( this, IDC_EDIT_CM_COLOR_G_2 );
	float fColor_b = GetWin_Num_float( this, IDC_EDIT_CM_COLOR_B_2 );

	CSelColorDlg dlg( BYTE( fColor_r * 255.0f ), BYTE( fColor_g * 255.0f ), BYTE( fColor_b * 255.0f ) );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_float( this, IDC_EDIT_CM_COLOR_R_2, float( GetRValue( crNewColor ) / 255.0f ) );
		SetWin_Num_float( this, IDC_EDIT_CM_COLOR_G_2, float( GetGValue( crNewColor ) / 255.0f ) );
		SetWin_Num_float( this, IDC_EDIT_CM_COLOR_B_2, float( GetBValue( crNewColor ) / 255.0f ) );
	}
}

void CPageEff_12_Mark::OnBnClickedCheckCmUse1()
{
	SetWin_Check( this, IDC_CHECK_CM_USE_1, TRUE );
	SetWin_Check( this, IDC_CHECK_CM_USE_2, FALSE );
}

void CPageEff_12_Mark::OnBnClickedCheckCmUse2()
{
	SetWin_Check( this, IDC_CHECK_CM_USE_1, FALSE );
	SetWin_Check( this, IDC_CHECK_CM_USE_2, TRUE );
}
