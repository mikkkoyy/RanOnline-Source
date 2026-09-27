// PageEff_15_Ambient.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkinPiece.h"
#include "EditorSkinPieceView.h"
#include "PageEff_15_Ambient.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "SelColorDlg.h"

// CPageEff_15_Ambient dialog

IMPLEMENT_DYNAMIC(CPageEff_15_Ambient, CPropertyPage)

CPageEff_15_Ambient::CPageEff_15_Ambient(LOGFONT logfont)
	: CPropertyPage(CPageEff_15_Ambient::IDD)
	, m_pFont ( NULL )
	, m_pPiece( NULL )
	, m_pEff( NULL )
	, m_bADD( FALSE )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageEff_15_Ambient::~CPageEff_15_Ambient()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEff_15_Ambient::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPageEff_15_Ambient, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_CA_COLOR, OnBnClickedButtonCaColor)
END_MESSAGE_MAP()


// CPageEff_15_Ambient message handlers
BOOL CPageEff_15_Ambient::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	ResetTool();

	return TRUE;
}

void CPageEff_15_Ambient::DataSet( DxSkinPiece* pPiece, DxEffCharAmbient* pEff, BOOL bADD )
{
	m_pPiece = pPiece;
	m_pEff = pEff;
	m_bADD = bADD;

	ResetTool();
	DataShow();
}

void CPageEff_15_Ambient::ResetTool()
{
	SetWin_Check( this, IDC_CHECK_CA_USE_ALLTIME, FALSE );
	SetWin_Check( this, IDC_CHECK_CA_USE_TEXCOLOR, FALSE );

	SetWin_Num_float( this, IDC_EDIT_CA_FULLTIME, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CA_COLOR_A, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CA_COLOR_R, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CA_COLOR_G, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CA_COLOR_B, 1.0f );
}

void CPageEff_15_Ambient::DataShow()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	EFFCHAR_PROPERTY_AMBIENT* pProp = (EFFCHAR_PROPERTY_AMBIENT*)m_pEff->GetProperty();
	SetWin_Check( this, IDC_CHECK_CA_USE_ALLTIME, pProp->m_dwFlage&USEALL );
	SetWin_Check( this, IDC_CHECK_CA_USE_TEXCOLOR, pProp->m_dwFlage&USETEXCOLOR );
	SetWin_Num_float( this, IDC_EDIT_CA_FULLTIME, pProp->m_fFullTime );
	SetWin_Num_float( this, IDC_EDIT_CA_COLOR_A, pProp->m_cColor.a );
	SetWin_Num_float( this, IDC_EDIT_CA_COLOR_R, pProp->m_cColor.r );
	SetWin_Num_float( this, IDC_EDIT_CA_COLOR_G, pProp->m_cColor.g );
	SetWin_Num_float( this, IDC_EDIT_CA_COLOR_B, pProp->m_cColor.b );
}

BOOL CPageEff_15_Ambient::DataSave()
{
	if ( !m_pPiece )	return FALSE;
	if ( !m_pEff )		return FALSE;

	EFFCHAR_PROPERTY_AMBIENT sProperty;
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CA_USE_ALLTIME ), sProperty.m_dwFlage, USEALL );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CA_USE_TEXCOLOR ), sProperty.m_dwFlage, USETEXCOLOR );
	sProperty.m_fFullTime = GetWin_Num_float( this, IDC_EDIT_CA_FULLTIME );
	sProperty.m_cColor.a = GetWin_Num_float( this, IDC_EDIT_CA_COLOR_A );
	sProperty.m_cColor.r = GetWin_Num_float( this, IDC_EDIT_CA_COLOR_R );
	sProperty.m_cColor.g = GetWin_Num_float( this, IDC_EDIT_CA_COLOR_G );
	sProperty.m_cColor.b = GetWin_Num_float( this, IDC_EDIT_CA_COLOR_B );
	
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

void CPageEff_15_Ambient::OnBnClickedButtonApply()
{
	DataSave();
}

void CPageEff_15_Ambient::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		m_pPiece = NULL;
		m_pEff = NULL;

		if ( m_pSheetTab )
			m_pSheetTab->ShowMain();
	}
}

void CPageEff_15_Ambient::OnBnClickedButtonClose()
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

void CPageEff_15_Ambient::OnBnClickedButtonCaColor()
{
	float fColor_r = GetWin_Num_float( this, IDC_EDIT_CA_COLOR_R );
	float fColor_g = GetWin_Num_float( this, IDC_EDIT_CA_COLOR_G );
	float fColor_b = GetWin_Num_float( this, IDC_EDIT_CA_COLOR_B );

	CSelColorDlg dlg( BYTE( fColor_r * 255.0f ), BYTE( fColor_g * 255.0f ), BYTE( fColor_b * 255.0f ) );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_float( this, IDC_EDIT_CA_COLOR_R, float( GetRValue( crNewColor ) / 255.0f ) );
		SetWin_Num_float( this, IDC_EDIT_CA_COLOR_G, float( GetGValue( crNewColor ) / 255.0f ) );
		SetWin_Num_float( this, IDC_EDIT_CA_COLOR_B, float( GetBValue( crNewColor ) / 255.0f ) );
	}
}
