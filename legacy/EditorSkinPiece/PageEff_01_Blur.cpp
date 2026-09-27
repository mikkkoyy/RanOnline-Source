// PageEff_01_Blur.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkinPiece.h"
#include "EditorSkinPieceView.h"
#include "PageEff_01_Blur.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "Global.h"
#include "TextureManager.h"
#include "SelColorDlg.h"
#include "DxMethods.h"


// CPageEff_01_Blur dialog

IMPLEMENT_DYNAMIC(CPageEff_01_Blur, CPropertyPage)

CPageEff_01_Blur::CPageEff_01_Blur(LOGFONT logfont)
	: CPropertyPage(CPageEff_01_Blur::IDD)
	, m_pFont ( NULL )
	, m_pPiece( NULL )
	, m_pEff( NULL )
	, m_bADD( FALSE )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageEff_01_Blur::~CPageEff_01_Blur()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEff_01_Blur::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_LIST_TRACE_UP, m_List_TraceUp );
	DDX_Control( pDX, IDC_LIST_TRACE_DOWN, m_List_TraceDown );
}


BEGIN_MESSAGE_MAP(CPageEff_01_Blur, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_APPLY, OnBnClickedButtonEditApply)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_SAVE, OnBnClickedButtonEditSave)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_CLOSE, OnBnClickedButtonEditClose)
	ON_BN_CLICKED(IDC_BUTTON_TEXTURE, OnBnClickedButtonTexture)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_UP, OnBnClickedButtonColorUp)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_DOWN, OnBnClickedButtonColorDown)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_TRACE_UP, OnNMDblclkListTraceUp)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_TRACE_DOWN, OnNMDblclkListTraceDown)
END_MESSAGE_MAP()


// CPageEff_01_Blur message handlers
BOOL CPageEff_01_Blur::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_List_TraceUp.SetExtendedStyle ( m_List_TraceUp.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List_TraceUp.GetClientRect ( &rectCtrl );
	m_List_TraceUp.InsertColumn( 0, "Trace", LVCFMT_LEFT, ( 35* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List_TraceUp.InsertColumn( 1, "TracePos", LVCFMT_LEFT, ( 65* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	m_List_TraceDown.SetExtendedStyle ( m_List_TraceDown.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List_TraceDown.GetClientRect ( &rectCtrl );
	m_List_TraceDown.InsertColumn( 0, "Trace", LVCFMT_LEFT, ( 35* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List_TraceDown.InsertColumn( 1, "TracePos", LVCFMT_LEFT, ( 65* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	SetWin_Combo_Init( this, IDC_COMBO_BLEND, GLOBAL_EFFTOOL::strEFFBLUR_BLEND, TOOL_EFFBLUR_BLEND_SIZE );

	ResetTool();

	return TRUE;
}

void CPageEff_01_Blur::DataSet( DxSkinPiece* pPiece, DxEffCharBlur* pEff, BOOL bADD )
{
	m_pPiece = pPiece;
	m_pEff = pEff;
	m_bADD = bADD;

	ResetTool();
	DataShow();
	TraceShow();
}

void CPageEff_01_Blur::ResetTool()
{
	m_List_TraceUp.DeleteAllItems();
	m_List_TraceDown.DeleteAllItems();

	SetWin_Text( this, IDC_EDIT_TEXTURE, "" );
	SetWin_Text( this, IDC_EDIT_TRACE_UP, "" );
	SetWin_Text( this, IDC_EDIT_TRACE_DOWN, "" );
	SetWin_Num_int( this, IDC_EDIT_COLOR_A_UP, 0 );
	SetWin_Num_int( this, IDC_EDIT_COLOR_R_UP, 0 );
	SetWin_Num_int( this, IDC_EDIT_COLOR_G_UP, 0 );
	SetWin_Num_int( this, IDC_EDIT_COLOR_B_UP, 0 );
	SetWin_Num_int( this, IDC_EDIT_COLOR_A_DOWN, 0 );
	SetWin_Num_int( this, IDC_EDIT_COLOR_R_DOWN, 0 );
	SetWin_Num_int( this, IDC_EDIT_COLOR_G_DOWN, 0 );
	SetWin_Num_int( this, IDC_EDIT_COLOR_B_DOWN, 0 );
	SetWin_Num_float( this, IDC_EDIT_GENTIME, 1.0f );
	SetWin_Combo_Sel( this, IDC_COMBO_BLEND, 0 );
	SetWin_Check( this, IDC_CHECK_USE_REFRACTION, FALSE );
	SetWin_Check( this, IDC_CHECK_USE_ALLACTION, FALSE );
}

void CPageEff_01_Blur::DataShow()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	EFFCHAR_PROPERTY_BLUR* pProp = (EFFCHAR_PROPERTY_BLUR*)m_pEff->GetProperty();

	SetWin_Text( this, IDC_EDIT_TEXTURE, pProp->m_szTexture );
	SetWin_Text( this, IDC_EDIT_TRACE_UP, pProp->m_szTraceUP );
	SetWin_Text( this, IDC_EDIT_TRACE_DOWN, pProp->m_szTraceDOWN );
	SetWin_Num_int( this, IDC_EDIT_COLOR_R_UP, RGBA_GETRED( pProp->m_vColorUPTemp ) );
	SetWin_Num_int( this, IDC_EDIT_COLOR_G_UP, RGBA_GETGREEN( pProp->m_vColorUPTemp ) );
	SetWin_Num_int( this, IDC_EDIT_COLOR_B_UP, RGBA_GETBLUE( pProp->m_vColorUPTemp ) );
	SetWin_Num_int( this, IDC_EDIT_COLOR_A_UP, RGBA_GETALPHA( pProp->m_vColorUPTemp ) );
	SetWin_Num_int( this, IDC_EDIT_COLOR_R_DOWN, RGBA_GETRED( pProp->m_vColorDOWNTemp ) );
	SetWin_Num_int( this, IDC_EDIT_COLOR_G_DOWN, RGBA_GETGREEN( pProp->m_vColorDOWNTemp ) );
	SetWin_Num_int( this, IDC_EDIT_COLOR_B_DOWN, RGBA_GETBLUE( pProp->m_vColorDOWNTemp ) );
	SetWin_Num_int( this, IDC_EDIT_COLOR_A_DOWN, RGBA_GETALPHA( pProp->m_vColorDOWNTemp ) );
	SetWin_Num_float( this, IDC_EDIT_GENTIME, pProp->m_fFullLifeTemp );
	SetWin_Check( this, IDC_CHECK_USE_REFRACTION, pProp->m_dwFlag&USEREFRACT );
	SetWin_Check( this, IDC_CHECK_USE_ALLACTION, pProp->m_dwFlag&USEALLACTION );
	SetWin_Combo_Sel( this, IDC_COMBO_BLEND, pProp->m_nBlend-1 );
}

BOOL CPageEff_01_Blur::DataSave()
{
	if ( !m_pPiece )	return FALSE;
	if ( !m_pEff )		return FALSE;

	std::string strFile = GetWin_Text( this, IDC_EDIT_TEXTURE );
	std::string strTraceUp = GetWin_Text( this, IDC_EDIT_TRACE_UP );
	std::string strTraceDown = GetWin_Text( this, IDC_EDIT_TRACE_DOWN );

	EFFCHAR_PROPERTY_BLUR sProperty;
	StringCchCopy( sProperty.m_szTexture, MAX_PATH, strFile.c_str() );
	StringCchCopy( sProperty.m_szTraceUP, STRACE_NSIZE, strTraceUp.c_str() );
	StringCchCopy( sProperty.m_szTraceDOWN, STRACE_NSIZE, strTraceDown.c_str() );

	sProperty.m_vColorUPTemp = RGBA_MAKE(
		GetWin_Num_int( this, IDC_EDIT_COLOR_R_UP ), 
		GetWin_Num_int( this, IDC_EDIT_COLOR_G_UP ), 
		GetWin_Num_int( this, IDC_EDIT_COLOR_B_UP ),
		GetWin_Num_int( this, IDC_EDIT_COLOR_A_UP ));
	sProperty.m_vColorDOWNTemp = RGBA_MAKE( 
		GetWin_Num_int( this, IDC_EDIT_COLOR_R_DOWN ), 
		GetWin_Num_int( this, IDC_EDIT_COLOR_G_DOWN ), 
		GetWin_Num_int( this, IDC_EDIT_COLOR_B_DOWN ), 
		GetWin_Num_int( this, IDC_EDIT_COLOR_A_DOWN ));

	sProperty.m_fFullLifeTemp = GetWin_Num_float( this, IDC_EDIT_GENTIME );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_USE_REFRACTION ), sProperty.m_dwFlag, USEREFRACT );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_USE_ALLACTION ), sProperty.m_dwFlag, USEALLACTION );
	sProperty.m_nBlend = GetWin_Combo_Sel( this, IDC_COMBO_BLEND ) +1;

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

void CPageEff_01_Blur::TraceShow()
{
	m_List_TraceUp.DeleteAllItems();
	m_List_TraceDown.DeleteAllItems();

	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	m_List_TraceUp.SetRedraw( FALSE );
	m_List_TraceDown.SetRedraw( FALSE );

	for ( int i =0; i < TOOL_EFFBLUR_TRACE_SIZE; ++i )
	{
		m_List_TraceUp.InsertItem( i, GLOBAL_EFFTOOL::strEFFBLUR_TRACE[i].c_str() );
		m_List_TraceDown.InsertItem( i, GLOBAL_EFFTOOL::strEFFBLUR_TRACE[i].c_str() );
	}

	for( int i = 0; i < m_List_TraceUp.GetItemCount(); i++ )
	{
		std::string str = m_List_TraceUp.GetItemText( i, 0 );
		SVERTEXINFLU* pTrace = m_pPiece->GetTracePos ( str.c_str() );
		if ( pTrace )
		{
			char szTempChar[512];
			sprintf( szTempChar, "(%d) %4.2f~%4.2f~%4.2f", pTrace->m_dwIndex, pTrace->m_vVector.x, pTrace->m_vVector.y, pTrace->m_vVector.z );
			m_List_TraceUp.SetItemText( i, 1, szTempChar );
		}
	}

	for( int i = 0; i < m_List_TraceDown.GetItemCount(); i++ )
	{
		std::string str = m_List_TraceDown.GetItemText( i, 0 );
		SVERTEXINFLU* pTrace = m_pPiece->GetTracePos ( str.c_str() );
		if ( pTrace )
		{
			char szTempChar[512];
			sprintf( szTempChar, "(%d) %4.2f~%4.2f~%4.2f", pTrace->m_dwIndex, pTrace->m_vVector.x, pTrace->m_vVector.y, pTrace->m_vVector.z );
			m_List_TraceDown.SetItemText( i, 1, szTempChar );
		}
	}

	m_List_TraceUp.SetRedraw( TRUE );
	m_List_TraceDown.SetRedraw( TRUE );
}

void CPageEff_01_Blur::OnBnClickedButtonEditApply()
{
	DataSave();
}

void CPageEff_01_Blur::OnBnClickedButtonEditSave()
{
	if ( DataSave() )
	{
		m_pPiece = NULL;
		m_pEff = NULL;

		if ( m_pSheetTab )
			m_pSheetTab->ShowMain();
	}
}

void CPageEff_01_Blur::OnBnClickedButtonEditClose()
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

void CPageEff_01_Blur::OnBnClickedButtonTexture()
{
	CString szFilter = "Texture Image (*.bmp,*.dds,*.tga,*.jpg)|*.bmp;*.dds;*.tga;*.jpg|";
	CFileDialog dlg(TRUE,".",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = TextureManager::GetTexturePath(); 
	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, IDC_EDIT_TEXTURE, dlg.GetFileName().GetString() );
}

void CPageEff_01_Blur::OnBnClickedButtonColorUp()
{
	BYTE nCUR_R = GetWin_Num_int( this, IDC_EDIT_COLOR_R_UP );
	BYTE nCUR_G = GetWin_Num_int( this, IDC_EDIT_COLOR_G_UP );
	BYTE nCUR_B = GetWin_Num_int( this, IDC_EDIT_COLOR_B_UP );

	CSelColorDlg dlg( nCUR_R, nCUR_G, nCUR_B );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_int( this, IDC_EDIT_COLOR_R_UP, GetRValue( crNewColor ) );
		SetWin_Num_int( this, IDC_EDIT_COLOR_G_UP, GetGValue( crNewColor ) );
		SetWin_Num_int( this, IDC_EDIT_COLOR_B_UP, GetBValue( crNewColor ) );
	}
}

void CPageEff_01_Blur::OnBnClickedButtonColorDown()
{
	BYTE nCUR_R = GetWin_Num_int( this, IDC_EDIT_COLOR_R_DOWN );
	BYTE nCUR_G = GetWin_Num_int( this, IDC_EDIT_COLOR_G_DOWN );
	BYTE nCUR_B = GetWin_Num_int( this, IDC_EDIT_COLOR_B_DOWN );

	CSelColorDlg dlg( nCUR_R, nCUR_G, nCUR_B );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_int( this, IDC_EDIT_COLOR_R_DOWN, GetRValue( crNewColor ) );
		SetWin_Num_int( this, IDC_EDIT_COLOR_G_DOWN, GetGValue( crNewColor ) );
		SetWin_Num_int( this, IDC_EDIT_COLOR_B_DOWN, GetBValue( crNewColor ) );
	}
}

void CPageEff_01_Blur::OnNMDblclkListTraceUp(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	int nSelect = m_List_TraceUp.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	std::string str = m_List_TraceUp.GetItemText( nSelect, 0 );
	SetWin_Text( this, IDC_EDIT_TRACE_UP, str.c_str());
}

void CPageEff_01_Blur::OnNMDblclkListTraceDown(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	int nSelect = m_List_TraceDown.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	std::string str = m_List_TraceDown.GetItemText( nSelect, 0 );
	SetWin_Text( this, IDC_EDIT_TRACE_DOWN, str.c_str());
}
