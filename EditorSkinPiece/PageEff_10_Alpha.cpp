// PageEff_10_Alpha.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkinPiece.h"
#include "EditorSkinPieceView.h"
#include "PageEff_10_Alpha.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"

// CPageEff_10_Alpha dialog

IMPLEMENT_DYNAMIC(CPageEff_10_Alpha, CPropertyPage)

CPageEff_10_Alpha::CPageEff_10_Alpha(LOGFONT logfont)
	: CPropertyPage(CPageEff_10_Alpha::IDD)
	, m_pFont ( NULL )
	, m_pPiece( NULL )
	, m_pEff( NULL )
	, m_bADD( FALSE )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageEff_10_Alpha::~CPageEff_10_Alpha()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEff_10_Alpha::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPageEff_10_Alpha, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnBnClickedButtonClose)
END_MESSAGE_MAP()


// CPageEff_10_Alpha message handlers
BOOL CPageEff_10_Alpha::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	ResetTool();

	return TRUE;
}

void CPageEff_10_Alpha::DataSet( DxSkinPiece* pPiece, DxEffCharAlpha* pEff, BOOL bADD )
{
	m_pPiece = pPiece;
	m_pEff = pEff;
	m_bADD = bADD;

	ResetTool();
	DataShow();
}

void CPageEff_10_Alpha::ResetTool()
{
	SetWin_Check( this, IDC_CHECK_CA_USE_TRANSLUCENT, FALSE );
	SetWin_Num_float( this, IDC_EDIT_CA_ALPHA, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CA_PLAYTIME, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CA_WAITTIME, 1.0f );
}

void CPageEff_10_Alpha::DataShow()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	EFFCHAR_PROPERTY_ALPHA* pProp = (EFFCHAR_PROPERTY_ALPHA*)m_pEff->GetProperty();
	SetWin_Check( this, IDC_CHECK_CA_USE_TRANSLUCENT, pProp->m_dwFlags&USETRANSLUCENT );
	SetWin_Num_float( this, IDC_EDIT_CA_ALPHA, pProp->m_fAlpha );
	SetWin_Num_float( this, IDC_EDIT_CA_PLAYTIME, pProp->m_fPlayTime );
	SetWin_Num_float( this, IDC_EDIT_CA_WAITTIME, pProp->m_fWaitTime );
}

BOOL CPageEff_10_Alpha::DataSave()
{
	if ( !m_pPiece )	return FALSE;
	if ( !m_pEff )		return FALSE;

	EFFCHAR_PROPERTY_ALPHA sProperty;
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CA_USE_TRANSLUCENT ), sProperty.m_dwFlags, USETRANSLUCENT );
	sProperty.m_fAlpha = GetWin_Num_float( this, IDC_EDIT_CA_ALPHA );
	sProperty.m_fPlayTime = GetWin_Num_float( this, IDC_EDIT_CA_PLAYTIME );
	sProperty.m_fWaitTime = GetWin_Num_float( this, IDC_EDIT_CA_WAITTIME );
	
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

void CPageEff_10_Alpha::OnBnClickedButtonApply()
{
	DataSave();
}

void CPageEff_10_Alpha::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		m_pPiece = NULL;
		m_pEff = NULL;

		if ( m_pSheetTab )
			m_pSheetTab->ShowMain();
	}
}

void CPageEff_10_Alpha::OnBnClickedButtonClose()
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
