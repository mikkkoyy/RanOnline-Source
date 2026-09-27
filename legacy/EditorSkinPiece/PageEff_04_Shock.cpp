// PageEff_04_Shock.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkinPiece.h"
#include "EditorSkinPieceView.h"
#include "PageEff_04_Shock.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "SelColorDlg.h"
#include "DxMethods.h"

// CPageEff_04_Shock dialog

IMPLEMENT_DYNAMIC(CPageEff_04_Shock, CPropertyPage)

CPageEff_04_Shock::CPageEff_04_Shock(LOGFONT logfont)
	: CPropertyPage(CPageEff_04_Shock::IDD)
	, m_pFont ( NULL )
	, m_pPiece( NULL )
	, m_pEff( NULL )
	, m_bADD( FALSE )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageEff_04_Shock::~CPageEff_04_Shock()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEff_04_Shock::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPageEff_04_Shock, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_SH_COLOR, OnBnClickedButtonShColor)
END_MESSAGE_MAP()


// CPageEff_04_Shock message handlers
BOOL CPageEff_04_Shock::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	ResetTool();

	return TRUE;
}

void CPageEff_04_Shock::DataSet( DxSkinPiece* pPiece, DxEffCharShock* pEff, BOOL bADD )
{
	m_pPiece = pPiece;
	m_pEff = pEff;
	m_bADD = bADD;

	ResetTool();
	DataShow();
}

void CPageEff_04_Shock::ResetTool()
{
	SetWin_Num_float( this, IDC_EDIT_SH_FULLTIME, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_SH_COLOR_A, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_SH_COLOR_R, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_SH_COLOR_G, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_SH_COLOR_B, 1.0f );
}

void CPageEff_04_Shock::DataShow()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	EFFCHAR_PROPERTY_SHOCK* pProp = (EFFCHAR_PROPERTY_SHOCK*)m_pEff->GetProperty();
	SetWin_Num_float( this, IDC_EDIT_SH_FULLTIME, pProp->m_fFullTime );
	SetWin_Num_float( this, IDC_EDIT_SH_COLOR_A, pProp->m_cColor.a );
	SetWin_Num_float( this, IDC_EDIT_SH_COLOR_R, pProp->m_cColor.r );
	SetWin_Num_float( this, IDC_EDIT_SH_COLOR_G, pProp->m_cColor.g );
	SetWin_Num_float( this, IDC_EDIT_SH_COLOR_B, pProp->m_cColor.b );
}

BOOL CPageEff_04_Shock::DataSave()
{
	if ( !m_pPiece )	return FALSE;
	if ( !m_pEff )		return FALSE;

	EFFCHAR_PROPERTY_SHOCK sProperty;
	sProperty.m_fFullTime = GetWin_Num_float( this, IDC_EDIT_SH_FULLTIME );
	sProperty.m_cColor.a = GetWin_Num_float( this, IDC_EDIT_SH_COLOR_A );
	sProperty.m_cColor.r = GetWin_Num_float( this, IDC_EDIT_SH_COLOR_R );
	sProperty.m_cColor.g = GetWin_Num_float( this, IDC_EDIT_SH_COLOR_G );
	sProperty.m_cColor.b = GetWin_Num_float( this, IDC_EDIT_SH_COLOR_B );
	
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

void CPageEff_04_Shock::OnBnClickedButtonApply()
{
	DataSave();
}

void CPageEff_04_Shock::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		m_pPiece = NULL;
		m_pEff = NULL;

		if ( m_pSheetTab )
			m_pSheetTab->ShowMain();
	}
}

void CPageEff_04_Shock::OnBnClickedButtonClose()
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

void CPageEff_04_Shock::OnBnClickedButtonShColor()
{
	float fColor_r = GetWin_Num_float( this, IDC_EDIT_SH_COLOR_R );
	float fColor_g = GetWin_Num_float( this, IDC_EDIT_SH_COLOR_G );
	float fColor_b = GetWin_Num_float( this, IDC_EDIT_SH_COLOR_B );

	CSelColorDlg dlg( BYTE( fColor_r * 255.0f ), BYTE( fColor_g * 255.0f ), BYTE( fColor_b * 255.0f ) );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_float( this, IDC_EDIT_SH_COLOR_R, float( GetRValue( crNewColor ) / 255.0f ) );
		SetWin_Num_float( this, IDC_EDIT_SH_COLOR_G, float( GetGValue( crNewColor ) / 255.0f ) );
		SetWin_Num_float( this, IDC_EDIT_SH_COLOR_B, float( GetBValue( crNewColor ) / 255.0f ) );
	}
}
