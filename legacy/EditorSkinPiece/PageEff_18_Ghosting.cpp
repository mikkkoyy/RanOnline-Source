// PageEff_18_Ghosting.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkinPiece.h"
#include "EditorSkinPieceView.h"
#include "PageEff_18_Ghosting.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "SelColorDlg.h"
#include "DxMethods.h"

// CPageEff_18_Ghosting dialog

IMPLEMENT_DYNAMIC(CPageEff_18_Ghosting, CPropertyPage)

CPageEff_18_Ghosting::CPageEff_18_Ghosting(LOGFONT logfont)
	: CPropertyPage(CPageEff_18_Ghosting::IDD)
	, m_pFont ( NULL )
	, m_pPiece( NULL )
	, m_pEff( NULL )
	, m_bADD( FALSE )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageEff_18_Ghosting::~CPageEff_18_Ghosting()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEff_18_Ghosting::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPageEff_18_Ghosting, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_CGH_COLOR, OnBnClickedButtonCghColor)
END_MESSAGE_MAP()


// CPageEff_18_Ghosting message handlers
BOOL CPageEff_18_Ghosting::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	SetWin_Combo_Init( this, IDC_COMBO_CGH_ANI_M, COMMENT::ANI_MAINTYPE_CHAR, AN_TYPE_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_CGH_ANI_S, COMMENT::ANI_SUBTYPE_SKILL, AN_SUB_00_SIZE );

	ResetTool();

	return TRUE;
}

void CPageEff_18_Ghosting::DataSet( DxSkinPiece* pPiece, DxEffCharGhosting* pEff, BOOL bADD )
{
	m_pPiece = pPiece;
	m_pEff = pEff;
	m_bADD = bADD;

	ResetTool();
	DataShow();
}

void CPageEff_18_Ghosting::ResetTool()
{
	SetWin_Combo_Sel( this, IDC_COMBO_CGH_ANI_M, 0 );
	SetWin_Combo_Sel( this, IDC_COMBO_CGH_ANI_S, 0 );

	SetWin_Num_int( this, IDC_EDIT_CGH_NUM, 1 );
	SetWin_Num_int( this, IDC_EDIT_CGH_UNIT, 1 );
	SetWin_Num_int( this, IDC_EDIT_CGH_ALPHA, 1 );

	SetWin_Num_int( this, IDC_EDIT_CGH_COLOR_A, 1 );
	SetWin_Num_int( this, IDC_EDIT_CGH_COLOR_R, 1 );
	SetWin_Num_int( this, IDC_EDIT_CGH_COLOR_G, 1 );
	SetWin_Num_int( this, IDC_EDIT_CGH_COLOR_B, 1 );
}

void CPageEff_18_Ghosting::DataShow()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	EFFCHAR_PROPERTY_GHOSTING* pProp = (EFFCHAR_PROPERTY_GHOSTING*)m_pEff->GetProperty();
	SetWin_Combo_Sel( this, IDC_COMBO_CGH_ANI_M, COMMENT::ANI_MAINTYPE_CHAR[pProp->m_AniMType].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_CGH_ANI_S, COMMENT::ANI_SUBTYPE_SKILL[pProp->m_AniSType].c_str() );

	SetWin_Num_int( this, IDC_EDIT_CGH_NUM, pProp->m_dwGhostingNum );
	SetWin_Num_int( this, IDC_EDIT_CGH_UNIT, pProp->m_dwUnit );
	SetWin_Num_int( this, IDC_EDIT_CGH_ALPHA, pProp->m_dwAlphaMax );

	SetWin_Num_int( this, IDC_EDIT_CGH_COLOR_R, RGBA_GETRED( pProp->m_dwColor ) );
	SetWin_Num_int( this, IDC_EDIT_CGH_COLOR_G, RGBA_GETGREEN( pProp->m_dwColor ) );
	SetWin_Num_int( this, IDC_EDIT_CGH_COLOR_B, RGBA_GETBLUE( pProp->m_dwColor ) );
	SetWin_Num_int( this, IDC_EDIT_CGH_COLOR_A, RGBA_GETALPHA( pProp->m_dwColor ) );
}

BOOL CPageEff_18_Ghosting::DataSave()
{
	if ( !m_pPiece )	return FALSE;
	if ( !m_pEff )		return FALSE;


	EFFCHAR_PROPERTY_GHOSTING sProperty;
	sProperty.m_AniMType = static_cast<EMANI_MAINTYPE>( GetWin_Combo_Sel( this, IDC_COMBO_CGH_ANI_M ) );
	sProperty.m_AniSType = static_cast<EMANI_SUBTYPE>( GetWin_Combo_Sel( this, IDC_COMBO_CGH_ANI_S ) );

	sProperty.m_dwGhostingNum = GetWin_Num_int( this, IDC_EDIT_CGH_NUM );
	sProperty.m_dwUnit = GetWin_Num_int( this, IDC_EDIT_CGH_UNIT );
	sProperty.m_dwAlphaMax = GetWin_Num_int( this, IDC_EDIT_CGH_ALPHA );

	sProperty.m_dwColor = RGBA_MAKE(
		GetWin_Num_int( this, IDC_EDIT_CGH_COLOR_R ), 
		GetWin_Num_int( this, IDC_EDIT_CGH_COLOR_G ), 
		GetWin_Num_int( this, IDC_EDIT_CGH_COLOR_B ),
		GetWin_Num_int( this, IDC_EDIT_CGH_COLOR_A ));


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

void CPageEff_18_Ghosting::OnBnClickedButtonApply()
{
	DataSave();
}

void CPageEff_18_Ghosting::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		m_pPiece = NULL;
		m_pEff = NULL;

		if ( m_pSheetTab )
			m_pSheetTab->ShowMain();
	}
}

void CPageEff_18_Ghosting::OnBnClickedButtonClose()
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

void CPageEff_18_Ghosting::OnBnClickedButtonCghColor()
{
	BYTE nCUR_R = GetWin_Num_int( this, IDC_EDIT_CGH_COLOR_R );
	BYTE nCUR_G = GetWin_Num_int( this, IDC_EDIT_CGH_COLOR_G );
	BYTE nCUR_B = GetWin_Num_int( this, IDC_EDIT_CGH_COLOR_B );

	CSelColorDlg dlg( nCUR_R, nCUR_G, nCUR_B );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_int( this, IDC_EDIT_CGH_COLOR_R, GetRValue( crNewColor ) );
		SetWin_Num_int( this, IDC_EDIT_CGH_COLOR_G, GetGValue( crNewColor ) );
		SetWin_Num_int( this, IDC_EDIT_CGH_COLOR_B, GetBValue( crNewColor ) );
	}
}
