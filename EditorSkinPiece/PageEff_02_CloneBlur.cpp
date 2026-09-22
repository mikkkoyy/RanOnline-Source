// PageEff_02_CloneBlur.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkinPiece.h"
#include "EditorSkinPieceView.h"
#include "PageEff_02_CloneBlur.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "Global.h"
#include "TextureManager.h"
#include "SelColorDlg.h"
#include "DxMethods.h"

// CPageEff_02_CloneBlur dialog

IMPLEMENT_DYNAMIC(CPageEff_02_CloneBlur, CPropertyPage)

CPageEff_02_CloneBlur::CPageEff_02_CloneBlur(LOGFONT logfont)
	: CPropertyPage(CPageEff_02_CloneBlur::IDD)
	, m_pFont ( NULL )
	, m_pPiece( NULL )
	, m_pEff( NULL )
	, m_bADD( FALSE )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageEff_02_CloneBlur::~CPageEff_02_CloneBlur()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEff_02_CloneBlur::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_LIST_TRACE_UP_CB, m_List_TraceUp );
	DDX_Control( pDX, IDC_LIST_TRACE_DOWN_CB, m_List_TraceDown );
}


BEGIN_MESSAGE_MAP(CPageEff_02_CloneBlur, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_TEXTURE_CB, OnBnClickedButtonTextureCb)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_TRACE_UP_CB, OnNMDblclkListTraceUpCb)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_TRACE_DOWN_CB, OnNMDblclkListTraceDownCb)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_UP_CB, OnBnClickedButtonColorUpCb)
	ON_BN_CLICKED(IDC_BUTTON_COLOR_DOWN_CB, OnBnClickedButtonColorDownCb)
END_MESSAGE_MAP()


// CPageEff_02_CloneBlur message handlers
BOOL CPageEff_02_CloneBlur::OnInitDialog()
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

	SetWin_Combo_Init( this, IDC_COMBO_BLEND_CB, GLOBAL_EFFTOOL::strEFFBLUR_BLEND, TOOL_EFFBLUR_BLEND_SIZE );

	ResetTool();

	return TRUE;
}

void CPageEff_02_CloneBlur::DataSet( DxSkinPiece* pPiece, DxEffCharCloneBlur* pEff, BOOL bADD )
{
	m_pPiece = pPiece;
	m_pEff = pEff;
	m_bADD = bADD;

	ResetTool();
	DataShow();
	TraceShow();
}

void CPageEff_02_CloneBlur::ResetTool()
{
	m_List_TraceUp.DeleteAllItems();
	m_List_TraceDown.DeleteAllItems();

	SetWin_Text( this, IDC_EDIT_TEXTURE_CB, "" );
	SetWin_Text( this, IDC_EDIT_TRACE_UP_CB, "" );
	SetWin_Text( this, IDC_EDIT_TRACE_DOWN_CB, "" );
	SetWin_Num_int( this, IDC_EDIT_COLOR_A_UP_CB, 0 );
	SetWin_Num_int( this, IDC_EDIT_COLOR_R_UP_CB, 0 );
	SetWin_Num_int( this, IDC_EDIT_COLOR_G_UP_CB, 0 );
	SetWin_Num_int( this, IDC_EDIT_COLOR_B_UP_CB, 0 );
	SetWin_Num_int( this, IDC_EDIT_COLOR_A_DOWN_CB, 0 );
	SetWin_Num_int( this, IDC_EDIT_COLOR_R_DOWN_CB, 0 );
	SetWin_Num_int( this, IDC_EDIT_COLOR_G_DOWN_CB, 0 );
	SetWin_Num_int( this, IDC_EDIT_COLOR_B_DOWN_CB, 0 );
	SetWin_Combo_Sel( this, IDC_COMBO_BLEND_CB, 0 );
	SetWin_Check( this, IDC_CHECK_USE_BEZIER_CB, FALSE );
	SetWin_Num_int( this, IDC_EDIT_FACES_CB, 0 );
	SetWin_Num_float( this, IDC_EDIT_WIDTH_CB, 1.0f );
}

void CPageEff_02_CloneBlur::DataShow()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	EFFCHAR_PROPERTY_CLONEBLUR* pProp = (EFFCHAR_PROPERTY_CLONEBLUR*)m_pEff->GetProperty();
	SetWin_Text( this, IDC_EDIT_TEXTURE_CB, pProp->m_szTexture );
	SetWin_Text( this, IDC_EDIT_TRACE_UP_CB, pProp->m_szTraceUP );
	SetWin_Text( this, IDC_EDIT_TRACE_DOWN_CB, pProp->m_szTraceDOWN );
	SetWin_Num_int( this, IDC_EDIT_COLOR_R_UP_CB, RGBA_GETRED( pProp->m_vColorUP ) );
	SetWin_Num_int( this, IDC_EDIT_COLOR_G_UP_CB, RGBA_GETGREEN( pProp->m_vColorUP ) );
	SetWin_Num_int( this, IDC_EDIT_COLOR_B_UP_CB, RGBA_GETBLUE( pProp->m_vColorUP ) );
	SetWin_Num_int( this, IDC_EDIT_COLOR_A_UP_CB, RGBA_GETALPHA( pProp->m_vColorUP ) );
	SetWin_Num_int( this, IDC_EDIT_COLOR_R_DOWN_CB, RGBA_GETRED( pProp->m_vColorDOWN ) );
	SetWin_Num_int( this, IDC_EDIT_COLOR_G_DOWN_CB, RGBA_GETGREEN( pProp->m_vColorDOWN ) );
	SetWin_Num_int( this, IDC_EDIT_COLOR_B_DOWN_CB, RGBA_GETBLUE( pProp->m_vColorDOWN ) );
	SetWin_Num_int( this, IDC_EDIT_COLOR_A_DOWN_CB, RGBA_GETALPHA( pProp->m_vColorDOWN ) );
	SetWin_Combo_Sel( this, IDC_COMBO_BLEND_CB, pProp->m_nBlend-1 );
	SetWin_Check( this, IDC_CHECK_USE_BEZIER_CB, pProp->m_bBezier );
	SetWin_Num_int( this, IDC_EDIT_FACES_CB, pProp->m_dwFaces );
	SetWin_Num_float( this, IDC_EDIT_WIDTH_CB,  pProp->m_fWidth );	
}

BOOL CPageEff_02_CloneBlur::DataSave()
{
	if ( !m_pPiece )	return FALSE;
	if ( !m_pEff )		return FALSE;

	std::string strFile = GetWin_Text( this, IDC_EDIT_TEXTURE_CB );
	std::string strTraceUp = GetWin_Text( this, IDC_EDIT_TRACE_UP_CB );
	std::string strTraceDown = GetWin_Text( this, IDC_EDIT_TRACE_DOWN_CB );

	EFFCHAR_PROPERTY_CLONEBLUR sProperty;
	StringCchCopy( sProperty.m_szTexture, MAX_PATH, strFile.c_str() );
	StringCchCopy( sProperty.m_szTraceUP, STRACE_NSIZE, strTraceUp.c_str() );
	StringCchCopy( sProperty.m_szTraceDOWN, STRACE_NSIZE, strTraceDown.c_str() );

	sProperty.m_vColorUP = RGBA_MAKE(
		GetWin_Num_int( this, IDC_EDIT_COLOR_R_UP_CB ), 
		GetWin_Num_int( this, IDC_EDIT_COLOR_G_UP_CB ), 
		GetWin_Num_int( this, IDC_EDIT_COLOR_B_UP_CB ),
		GetWin_Num_int( this, IDC_EDIT_COLOR_A_UP_CB ));
	sProperty.m_vColorDOWN = RGBA_MAKE( 
		GetWin_Num_int( this, IDC_EDIT_COLOR_R_DOWN_CB ), 
		GetWin_Num_int( this, IDC_EDIT_COLOR_G_DOWN_CB ), 
		GetWin_Num_int( this, IDC_EDIT_COLOR_B_DOWN_CB ), 
		GetWin_Num_int( this, IDC_EDIT_COLOR_A_DOWN_CB ));

	sProperty.m_nBlend = GetWin_Combo_Sel( this, IDC_COMBO_BLEND_CB ) +1;
	sProperty.m_bBezier = GetWin_Check( this, IDC_CHECK_USE_BEZIER_CB );
	sProperty.m_dwFaces = GetWin_Num_int( this, IDC_EDIT_FACES_CB );
	sProperty.m_fWidth = GetWin_Num_float( this, IDC_EDIT_WIDTH_CB );

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

void CPageEff_02_CloneBlur::TraceShow()
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

void CPageEff_02_CloneBlur::OnBnClickedButtonApply()
{
	DataSave();
}

void CPageEff_02_CloneBlur::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		m_pPiece = NULL;
		m_pEff = NULL;

		if ( m_pSheetTab )
			m_pSheetTab->ShowMain();
	}
}

void CPageEff_02_CloneBlur::OnBnClickedButtonClose()
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

void CPageEff_02_CloneBlur::OnBnClickedButtonTextureCb()
{
	CString szFilter = "Texture Image (*.bmp,*.dds,*.tga,*.jpg)|*.bmp;*.dds;*.tga;*.jpg|";
	CFileDialog dlg(TRUE,".",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = TextureManager::GetTexturePath(); 
	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, IDC_EDIT_TEXTURE_CB, dlg.GetFileName().GetString() );
}

void CPageEff_02_CloneBlur::OnNMDblclkListTraceUpCb(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	int nSelect = m_List_TraceUp.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	std::string str = m_List_TraceUp.GetItemText( nSelect, 0 );
	SetWin_Text( this, IDC_EDIT_TRACE_UP_CB, str.c_str());
}

void CPageEff_02_CloneBlur::OnNMDblclkListTraceDownCb(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	int nSelect = m_List_TraceDown.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	std::string str = m_List_TraceDown.GetItemText( nSelect, 0 );
	SetWin_Text( this, IDC_EDIT_TRACE_DOWN_CB, str.c_str());
}

void CPageEff_02_CloneBlur::OnBnClickedButtonColorUpCb()
{
	BYTE nCUR_R = GetWin_Num_int( this, IDC_EDIT_COLOR_R_UP_CB );
	BYTE nCUR_G = GetWin_Num_int( this, IDC_EDIT_COLOR_G_UP_CB );
	BYTE nCUR_B = GetWin_Num_int( this, IDC_EDIT_COLOR_B_UP_CB );

	CSelColorDlg dlg( nCUR_R, nCUR_G, nCUR_B );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_int( this, IDC_EDIT_COLOR_R_UP_CB, GetRValue( crNewColor ) );
		SetWin_Num_int( this, IDC_EDIT_COLOR_G_UP_CB, GetGValue( crNewColor ) );
		SetWin_Num_int( this, IDC_EDIT_COLOR_B_UP_CB, GetBValue( crNewColor ) );
	}
}

void CPageEff_02_CloneBlur::OnBnClickedButtonColorDownCb()
{
	BYTE nCUR_R = GetWin_Num_int( this, IDC_EDIT_COLOR_R_DOWN_CB );
	BYTE nCUR_G = GetWin_Num_int( this, IDC_EDIT_COLOR_G_DOWN_CB );
	BYTE nCUR_B = GetWin_Num_int( this, IDC_EDIT_COLOR_B_DOWN_CB );

	CSelColorDlg dlg( nCUR_R, nCUR_G, nCUR_B );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_int( this, IDC_EDIT_COLOR_R_DOWN_CB, GetRValue( crNewColor ) );
		SetWin_Num_int( this, IDC_EDIT_COLOR_G_DOWN_CB, GetGValue( crNewColor ) );
		SetWin_Num_int( this, IDC_EDIT_COLOR_B_DOWN_CB, GetBValue( crNewColor ) );
	}
}
