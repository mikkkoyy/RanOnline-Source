// PageEff_26_Normal.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkinPiece.h"
#include "EditorSkinPieceView.h"
#include "PageEff_26_Normal.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"

// CPageEff_26_Normal dialog

IMPLEMENT_DYNAMIC(CPageEff_26_Normal, CPropertyPage)

CPageEff_26_Normal::CPageEff_26_Normal(LOGFONT logfont)
	: CPropertyPage(CPageEff_26_Normal::IDD)
	, m_pFont ( NULL )
	, m_pPiece( NULL )
	, m_pEff( NULL )
	, m_bADD( FALSE )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageEff_26_Normal::~CPageEff_26_Normal()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEff_26_Normal::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPageEff_26_Normal, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnBnClickedButtonClose)
END_MESSAGE_MAP()


// CPageEff_26_Normal message handlers
BOOL CPageEff_26_Normal::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	ResetTool();

	return TRUE;
}

void CPageEff_26_Normal::DataSet( DxSkinPiece* pPiece, DxEffCharNormal* pEff, BOOL bADD )
{
	m_pPiece = pPiece;
	m_pEff = pEff;
	m_bADD = bADD;

	ResetTool();
	DataShow();
}

void CPageEff_26_Normal::ResetTool()
{
}

void CPageEff_26_Normal::DataShow()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	EFFCHAR_PROPERTY_NORMAL* pProp = (EFFCHAR_PROPERTY_NORMAL*)m_pEff->GetProperty();
}

BOOL CPageEff_26_Normal::DataSave()
{
	if ( !m_pPiece )	return FALSE;
	if ( !m_pEff )		return FALSE;


	EFFCHAR_PROPERTY_NORMAL sProperty;
	

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

void CPageEff_26_Normal::OnBnClickedButtonApply()
{
	DataSave();
}

void CPageEff_26_Normal::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		m_pPiece = NULL;
		m_pEff = NULL;

		if ( m_pSheetTab )
			m_pSheetTab->ShowMain();
	}
}

void CPageEff_26_Normal::OnBnClickedButtonClose()
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
