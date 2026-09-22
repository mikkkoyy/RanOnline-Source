// PageEff_20_Toon.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkinPiece.h"
#include "EditorSkinPieceView.h"
#include "PageEff_20_Toon.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"

// CPageEff_20_Toon dialog

IMPLEMENT_DYNAMIC(CPageEff_20_Toon, CPropertyPage)

CPageEff_20_Toon::CPageEff_20_Toon(LOGFONT logfont)
	: CPropertyPage(CPageEff_20_Toon::IDD)
	, m_pFont ( NULL )
	, m_pPiece( NULL )
	, m_pEff( NULL )
	, m_bADD( FALSE )
{	
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageEff_20_Toon::~CPageEff_20_Toon()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEff_20_Toon::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPageEff_20_Toon, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_CTN_TEX, OnBnClickedButtonCtnTex)
	ON_BN_CLICKED(IDC_BUTTON_CTN_OBLIQUE, OnBnClickedButtonCtnOblique)
END_MESSAGE_MAP()


// CPageEff_20_Toon message handlers
BOOL CPageEff_20_Toon::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	ResetTool();

	return TRUE;
}

void CPageEff_20_Toon::DataSet( DxSkinPiece* pPiece, DxEffCharToon* pEff, BOOL bADD )
{
	m_pPiece = pPiece;
	m_pEff = pEff;
	m_bADD = bADD;

	ResetTool();
	DataShow();
}

void CPageEff_20_Toon::ResetTool()
{
	SetWin_Num_float( this, IDC_EDIT_CTN_SIZE, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CTN_ADDSIZE, 1.0f );
	SetWin_Text( this, IDC_EDIT_CTN_TEX, "" );
	SetWin_Check( this, IDC_CHECK_CTN_USE_OBLIQUE, FALSE );
	SetWin_Text( this, IDC_EDIT_CTN_OBLIQUE, "" );
}

void CPageEff_20_Toon::DataShow()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	EFFCHAR_PROPERTY_TOON* pProp = (EFFCHAR_PROPERTY_TOON*)m_pEff->GetProperty();
	SetWin_Num_float( this, IDC_EDIT_CTN_SIZE, pProp->m_fSize );
	SetWin_Num_float( this, IDC_EDIT_CTN_ADDSIZE, pProp->m_fSizeADD );
	SetWin_Text( this, IDC_EDIT_CTN_TEX, pProp->m_szToonTex );

	SetWin_Check( this, IDC_CHECK_CTN_USE_OBLIQUE, pProp->m_dwFlag&USE_OBLIQUE);
	SetWin_Text( this, IDC_EDIT_CTN_OBLIQUE, pProp->m_szObliqueTex );
}

BOOL CPageEff_20_Toon::DataSave()
{
	if ( !m_pPiece )	return FALSE;
	if ( !m_pEff )		return FALSE;

	std::string strTex = GetWin_Text( this, IDC_EDIT_CTN_TEX );
	std::string strObli = GetWin_Text( this, IDC_EDIT_CTN_OBLIQUE );

	EFFCHAR_PROPERTY_TOON sProperty;
	sProperty.m_fSize = GetWin_Num_float( this, IDC_EDIT_CTN_SIZE );
	sProperty.m_fSizeADD = GetWin_Num_float( this, IDC_EDIT_CTN_ADDSIZE );
	StringCchCopy( sProperty.m_szToonTex, 32, strTex.c_str() );
	StringCchCopy( sProperty.m_szObliqueTex, 32, strObli.c_str() );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CTN_USE_OBLIQUE ), sProperty.m_dwFlag, USE_OBLIQUE );

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

	m_pPiece->CreateCartoonMesh( CEditorSkinPieceView::GetView()->Get3DDevice(), sProperty.m_fSize, sProperty.m_fSizeADD );
	//m_pPiece->CreateNormalMapMESH( CEditorSkinPieceView::GetView()->Get3DDevice() );

	return TRUE;
}

void CPageEff_20_Toon::OnBnClickedButtonApply()
{
	DataSave();
}

void CPageEff_20_Toon::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		m_pPiece = NULL;
		m_pEff = NULL;

		if ( m_pSheetTab )
			m_pSheetTab->ShowMain();
	}
}

void CPageEff_20_Toon::OnBnClickedButtonClose()
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

void CPageEff_20_Toon::OnBnClickedButtonCtnTex()
{
	CString szFilter = "Texture Image (*.bmp,*.dds,*.tga,*.jpg)|*.bmp;*.dds;*.tga;*.jpg|";
	CFileDialog dlg(TRUE,".",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = TextureManager::GetTexturePath(); 
	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, IDC_EDIT_CTN_TEX, dlg.GetFileName().GetString() );
}

void CPageEff_20_Toon::OnBnClickedButtonCtnOblique()
{
	CString szFilter = "Texture Image (*.bmp,*.dds,*.tga,*.jpg)|*.bmp;*.dds;*.tga;*.jpg|";
	CFileDialog dlg(TRUE,".",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = TextureManager::GetTexturePath(); 
	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, IDC_EDIT_CTN_OBLIQUE, dlg.GetFileName().GetString() );
}
