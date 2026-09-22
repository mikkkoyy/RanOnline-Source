// PageEff_06_Arrow.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkinPiece.h"
#include "EditorSkinPieceView.h"
#include "PageEff_06_Arrow.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "SelColorDlg.h"
#include "DxMethods.h"
#include "Global.h"

// CPageEff_06_Arrow dialog

IMPLEMENT_DYNAMIC(CPageEff_06_Arrow, CPropertyPage)

CPageEff_06_Arrow::CPageEff_06_Arrow(LOGFONT logfont)
	: CPropertyPage(CPageEff_06_Arrow::IDD)
	, m_pFont ( NULL )
	, m_pPiece( NULL )
	, m_pEff( NULL )
	, m_bADD( FALSE )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageEff_06_Arrow::~CPageEff_06_Arrow()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEff_06_Arrow::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPageEff_06_Arrow, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_CA_EFFBASE, OnBnClickedButtonCaEffbase)
	ON_BN_CLICKED(IDC_BUTTON_CA_EFFUP, OnBnClickedButtonCaEffup)
	ON_BN_CLICKED(IDC_BUTTON_CA_COLOR, OnBnClickedButtonCaColor)
	ON_BN_CLICKED(IDC_BUTTON_CA_MESH, OnBnClickedButtonCaMesh)
END_MESSAGE_MAP()


// CPageEff_06_Arrow message handlers
BOOL CPageEff_06_Arrow::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	SetWin_Combo_Init( this, IDC_COMBO_CA_BLEND, GLOBAL_EFFTOOL::strEFFBLUR_BLEND, TOOL_EFFBLUR_BLEND_SIZE );

	ResetTool();

	return TRUE;
}

void CPageEff_06_Arrow::DataSet( DxSkinPiece* pPiece, DxEffCharArrow* pEff, BOOL bADD )
{
	m_pPiece = pPiece;
	m_pEff = pEff;
	m_bADD = bADD;

	ResetTool();
	DataShow();
}

void CPageEff_06_Arrow::ResetTool()
{
	SetWin_Check( this, IDC_CHECK_CA_USEEFFECT, FALSE );
	SetWin_Text( this, IDC_EDIT_CA_EFFBASE, "" );
	SetWin_Text( this, IDC_EDIT_CA_EFFUP, "" );
	SetWin_Num_float( this, IDC_EDIT_CA_EFFRANGE, 1.0f );

	SetWin_Combo_Sel( this, IDC_COMBO_CA_BLEND, 0 );
	SetWin_Check( this, IDC_CHECK_CA_USEROTATION, FALSE );
	SetWin_Check( this, IDC_CHECK_CA_USESEC, FALSE );
	SetWin_Num_int( this, IDC_EDIT_CA_SEC, 0 );
	SetWin_Check( this, IDC_CHECK_CA_USEARROW, FALSE );

	SetWin_Check( this, IDC_CHECK_CA_USESIZE, FALSE );
	SetWin_Num_float( this, IDC_EDIT_CA_SIZE, 1.0f );

	SetWin_Num_float( this, IDC_EDIT_CA_FULLTIME, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CA_ALPHATIME, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CA_SETRATE, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CA_DEPTH, 1.0f );

	SetWin_Num_int( this, IDC_EDIT_CA_COLOR_R, 0 );
	SetWin_Num_int( this, IDC_EDIT_CA_COLOR_G, 0 );
	SetWin_Num_int( this, IDC_EDIT_CA_COLOR_B, 0 );
	SetWin_Num_int( this, IDC_EDIT_CA_COLOR_A, 0 );

	SetWin_Text( this, IDC_EDIT_CA_MESH, "" );
}

void CPageEff_06_Arrow::DataShow()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	EFFCHAR_PROPERTY_ARROW* pProp = (EFFCHAR_PROPERTY_ARROW*)m_pEff->GetProperty();

	SetWin_Check( this, IDC_CHECK_CA_USEEFFECT, pProp->m_dwFlag&USEEFFECT );
	SetWin_Text( this, IDC_EDIT_CA_EFFBASE, pProp->m_szFileName_R );
	SetWin_Text( this, IDC_EDIT_CA_EFFUP, pProp->m_szFileName_C );
	SetWin_Num_float( this, IDC_EDIT_CA_EFFRANGE, pProp->m_vEffEnd.y );

	SetWin_Combo_Sel( this, IDC_COMBO_CA_BLEND, pProp->m_nBlend-1 );
	SetWin_Check( this, IDC_CHECK_CA_USEROTATION, pProp->m_dwFlag&USERANDROTATE );
	SetWin_Check( this, IDC_CHECK_CA_USESEC, pProp->m_dwFlag&USESEC );
	SetWin_Num_int( this, IDC_EDIT_CA_SEC, pProp->m_nSec );
	SetWin_Check( this, IDC_CHECK_CA_USEARROW, pProp->m_dwFlag&USEARROW );

	SetWin_Check( this, IDC_CHECK_CA_USESIZE, pProp->m_dwFlag&USESIZE );
	SetWin_Num_float( this, IDC_EDIT_CA_SIZE, pProp->m_fSize );

	SetWin_Num_float( this, IDC_EDIT_CA_FULLTIME, pProp->m_fFullTime );
	SetWin_Num_float( this, IDC_EDIT_CA_ALPHATIME, pProp->m_fAlphaTime );
	SetWin_Num_float( this, IDC_EDIT_CA_SETRATE, pProp->m_fSetRate );
	SetWin_Num_float( this, IDC_EDIT_CA_DEPTH, pProp->m_fDepth );

	SetWin_Num_int( this, IDC_EDIT_CA_COLOR_R, RGBA_GETRED( pProp->m_vColor ) );
	SetWin_Num_int( this, IDC_EDIT_CA_COLOR_G, RGBA_GETGREEN( pProp->m_vColor ) );
	SetWin_Num_int( this, IDC_EDIT_CA_COLOR_B, RGBA_GETBLUE( pProp->m_vColor ) );
	SetWin_Num_int( this, IDC_EDIT_CA_COLOR_A, RGBA_GETALPHA( pProp->m_vColor ) );

	SetWin_Text( this, IDC_EDIT_CA_MESH, pProp->m_szMeshFile );
}

BOOL CPageEff_06_Arrow::DataSave()
{
	if ( !m_pPiece )	return FALSE;
	if ( !m_pEff )		return FALSE;

	std::string str_R = GetWin_Text( this, IDC_EDIT_CA_EFFBASE );
	std::string str_C = GetWin_Text( this, IDC_EDIT_CA_EFFUP );
	std::string strMESH = GetWin_Text( this, IDC_EDIT_CA_MESH );

	EFFCHAR_PROPERTY_ARROW sProperty;
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CA_USEEFFECT ), sProperty.m_dwFlag, USEEFFECT );
	StringCchCopy( sProperty.m_szFileName_R , MAX_PATH, str_R.c_str() );
	StringCchCopy( sProperty.m_szFileName_C , MAX_PATH, str_C.c_str() );
	sProperty.m_vEffEnd.y = GetWin_Num_float( this, IDC_EDIT_CA_EFFRANGE );

	sProperty.m_nBlend = GetWin_Combo_Sel( this, IDC_COMBO_CA_BLEND ) +1;
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CA_USEROTATION ), sProperty.m_dwFlag, USERANDROTATE );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CA_USESEC ), sProperty.m_dwFlag, USESEC );
	sProperty.m_nSec = GetWin_Num_int( this, IDC_EDIT_CA_SEC );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CA_USEARROW ), sProperty.m_dwFlag, USEARROW );

	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CA_USESIZE ), sProperty.m_dwFlag, USESIZE );
	sProperty.m_fSize = GetWin_Num_float( this, IDC_EDIT_CA_SIZE );

	sProperty.m_fFullTime = GetWin_Num_float( this, IDC_EDIT_CA_FULLTIME );
	sProperty.m_fAlphaTime = GetWin_Num_float( this, IDC_EDIT_CA_ALPHATIME );
	sProperty.m_fSetRate = GetWin_Num_float( this, IDC_EDIT_CA_SETRATE );
	sProperty.m_fDepth = GetWin_Num_float( this, IDC_EDIT_CA_DEPTH );

	sProperty.m_vColor = RGBA_MAKE(
		GetWin_Num_int( this, IDC_EDIT_CA_COLOR_R ), 
		GetWin_Num_int( this, IDC_EDIT_CA_COLOR_G ), 
		GetWin_Num_int( this, IDC_EDIT_CA_COLOR_B ),
		GetWin_Num_int( this, IDC_EDIT_CA_COLOR_A ));

	StringCchCopy( sProperty.m_szMeshFile, MAX_PATH, strMESH.c_str() );

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

void CPageEff_06_Arrow::OnBnClickedButtonApply()
{
	DataSave();
}

void CPageEff_06_Arrow::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		m_pPiece = NULL;
		m_pEff = NULL;

		if ( m_pSheetTab )
			m_pSheetTab->ShowMain();
	}
}

void CPageEff_06_Arrow::OnBnClickedButtonClose()
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

void CPageEff_06_Arrow::OnBnClickedButtonCaEffbase()
{
	CString szFilter = "Effect File (*.egp)|*.egp|";
	CFileDialog dlg(TRUE,".egp",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = DxEffSinglePropGMan::GetInstance().GetPath ();
	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, IDC_EDIT_CA_EFFBASE, dlg.GetFileName().GetString() );
}

void CPageEff_06_Arrow::OnBnClickedButtonCaEffup()
{
	CString szFilter = "Effect File (*.egp)|*.egp|";
	CFileDialog dlg(TRUE,".egp",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = DxEffSinglePropGMan::GetInstance().GetPath ();
	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, IDC_EDIT_CA_EFFUP, dlg.GetFileName().GetString() );
}

void CPageEff_06_Arrow::OnBnClickedButtonCaColor()
{
	BYTE nCUR_R = GetWin_Num_int( this, IDC_EDIT_CA_COLOR_R );
	BYTE nCUR_G = GetWin_Num_int( this, IDC_EDIT_CA_COLOR_G );
	BYTE nCUR_B = GetWin_Num_int( this, IDC_EDIT_CA_COLOR_B );

	CSelColorDlg dlg( nCUR_R, nCUR_G, nCUR_B );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_int( this, IDC_EDIT_CA_COLOR_R, GetRValue( crNewColor ) );
		SetWin_Num_int( this, IDC_EDIT_CA_COLOR_G, GetGValue( crNewColor ) );
		SetWin_Num_int( this, IDC_EDIT_CA_COLOR_B, GetBValue( crNewColor ) );
	}
}

void CPageEff_06_Arrow::OnBnClickedButtonCaMesh()
{
	CString szFilter = "X Mesh File (*.x)|*.x|";
	CFileDialog dlg(TRUE,".x",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = DxSimpleMeshMan::GetInstance().GetPath ();
	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, IDC_EDIT_CA_MESH, dlg.GetFileName().GetString() );
}
