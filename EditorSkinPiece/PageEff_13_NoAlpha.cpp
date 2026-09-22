// PageEff_13_NoAlpha.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkinPiece.h"
#include "EditorSkinPieceView.h"
#include "PageEff_13_NoAlpha.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"

// CPageEff_13_NoAlpha dialog

IMPLEMENT_DYNAMIC(CPageEff_13_NoAlpha, CPropertyPage)

CPageEff_13_NoAlpha::CPageEff_13_NoAlpha(LOGFONT logfont)
	: CPropertyPage(CPageEff_13_NoAlpha::IDD)
	, m_pFont ( NULL )
	, m_pPiece( NULL )
	, m_pEff( NULL )
	, m_bADD( FALSE )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageEff_13_NoAlpha::~CPageEff_13_NoAlpha()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEff_13_NoAlpha::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPageEff_13_NoAlpha, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnBnClickedButtonClose)
END_MESSAGE_MAP()


// CPageEff_13_NoAlpha message handlers
BOOL CPageEff_13_NoAlpha::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	ResetTool();

	return TRUE;
}

void CPageEff_13_NoAlpha::DataSet( DxSkinPiece* pPiece, DxEffCharNoAlpha* pEff, BOOL bADD )
{
	m_pPiece = pPiece;
	m_pEff = pEff;
	m_bADD = bADD;

	ResetTool();
	DataShow();
}

void CPageEff_13_NoAlpha::ResetTool()
{
}

void CPageEff_13_NoAlpha::DataShow()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	EFFCHAR_PROPERTY_NOALPHA* pProp = (EFFCHAR_PROPERTY_NOALPHA*)m_pEff->GetProperty();
	//theres nothing!
}

BOOL CPageEff_13_NoAlpha::DataSave()
{
	if ( !m_pPiece )	return FALSE;
	if ( !m_pEff )		return FALSE;

	EFFCHAR_PROPERTY_NOALPHA sProperty;

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

void CPageEff_13_NoAlpha::OnBnClickedButtonApply()
{
	DataSave();
}

void CPageEff_13_NoAlpha::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		m_pPiece = NULL;
		m_pEff = NULL;

		if ( m_pSheetTab )
			m_pSheetTab->ShowMain();
	}
}

void CPageEff_13_NoAlpha::OnBnClickedButtonClose()
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
