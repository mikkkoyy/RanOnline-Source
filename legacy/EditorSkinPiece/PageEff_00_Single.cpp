// PageEff_00_Single.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkinPiece.h"
#include "EditorSkinPieceView.h"
#include "PageEff_00_Single.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "Global.h"

// CPageEff_00_Single dialog

IMPLEMENT_DYNAMIC(CPageEff_00_Single, CPropertyPage)

CPageEff_00_Single::CPageEff_00_Single(LOGFONT logfont)
	: CPropertyPage(CPageEff_00_Single::IDD)
	, m_pFont ( NULL )
	, m_pPiece( NULL )
	, m_pEff( NULL )
	, m_bADD( FALSE )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageEff_00_Single::~CPageEff_00_Single()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEff_00_Single::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_LIST_TRACE_1, m_List_Dot1 );
	DDX_Control( pDX, IDC_LIST_TRACE_2, m_List_Dot2 );
}


BEGIN_MESSAGE_MAP(CPageEff_00_Single, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_APPLY,OnBnClickedButtonEditApply)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_SAVE,OnBnClickedButtonEditSave)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_CLOSE,OnBnClickedButtonEditClose)
	ON_BN_CLICKED(IDC_BUTTON_EFFECT,OnBnClickedButtonEffect)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_TRACE_1,OnNMDblclkListTrace1)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_TRACE_2,OnNMDblclkListTrace2)
	ON_BN_CLICKED(IDC_CHECK_DOT_1,OnBnClickedCheckDot1)
	ON_BN_CLICKED(IDC_CHECK_DOT_2,OnBnClickedCheckDot2)
	ON_BN_CLICKED(IDC_CHECK_DOT_MATRIX,OnBnClickedCheckDotMatrix)
	ON_BN_CLICKED(IDC_CHECK_DOT_MATRIX2,OnBnClickedCheckDotMatrix2)
END_MESSAGE_MAP()


// CPageEff_00_Single message handlers
BOOL CPageEff_00_Single::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_List_Dot1.SetExtendedStyle ( m_List_Dot1.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List_Dot1.GetClientRect ( &rectCtrl );
	m_List_Dot1.InsertColumn( 0, "Trace", LVCFMT_LEFT, ( 35* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List_Dot1.InsertColumn( 1, "TracePos", LVCFMT_LEFT, ( 65* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	m_List_Dot2.SetExtendedStyle ( m_List_Dot2.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List_Dot2.GetClientRect ( &rectCtrl );
	m_List_Dot2.InsertColumn( 0, "Trace", LVCFMT_LEFT, ( 35* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List_Dot2.InsertColumn( 1, "TracePos", LVCFMT_LEFT, ( 65* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	SetWin_Combo_Init( this, IDC_COMBO_ANISET, GLOBAL_EFFTOOL::strEFFSINGLE_ANI, TOOL_EFFSINGLE_ANI_SIZE );
	SetWin_Combo_Init( this, IDC_COMBO_WEATHERSET, GLOBAL_EFFTOOL::strEFFSINGLE_WEATHER, TOOL_EFFSINGLE_WEATHER_SIZE );

	ResetTool();

	return TRUE;
}

void CPageEff_00_Single::DataSet( DxSkinPiece* pPiece, DxEffCharSingle* pEff, BOOL bADD )
{
	m_pPiece = pPiece;
	m_pEff = pEff;
	m_bADD = bADD;

	ResetTool();
	DataShow();
	TraceShow();
}

void CPageEff_00_Single::ResetTool()
{
	m_List_Dot1.DeleteAllItems();
	m_List_Dot2.DeleteAllItems();

	SetWin_Text( this, IDC_EDIT_EFFECT, "" );
	SetWin_Text( this, IDC_EDIT_TRACE_1, "" );
	SetWin_Text( this, IDC_EDIT_TRACE_2, "" );
	SetWin_Num_float( this, IDC_EDIT_TRACE_MOVE_1, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_TRACE_MOVE_2, 1.0f );
	SetWin_Check( this, IDC_CHECK_DOT_1, FALSE );
	SetWin_Check( this, IDC_CHECK_DOT_2, FALSE );
	SetWin_Check( this, IDC_CHECK_DOT_MATRIX, FALSE );
	SetWin_Check( this, IDC_CHECK_DOT_MATRIX2, FALSE );
	SetWin_Check( this, IDC_CHECK_MOVE_AUTO, FALSE );
	SetWin_Check( this, IDC_CHECK_MOVE_REPEAT, FALSE );
	SetWin_Check( this, IDC_CHECK_REPEAT, FALSE );
	SetWin_Num_float( this, IDC_EDIT_MOVE_SPEED, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_REPEAT_DELAY, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_SCALE, 1.0f );
	SetWin_Combo_Sel( this, IDC_COMBO_ANISET, 0 );
	SetWin_Combo_Sel( this, IDC_COMBO_WEATHERSET, 0 );
	SetWin_Check( this, IDC_CHECK_MATRIX2_USE2, FALSE );
}

void CPageEff_00_Single::DataShow()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	EFFCHAR_PROPERTY_SINGLE* pProp = (EFFCHAR_PROPERTY_SINGLE*)m_pEff->GetProperty();

	SetWin_Text( this, IDC_EDIT_EFFECT, pProp->m_szFileName );
	SetWin_Text( this, IDC_EDIT_TRACE_1, pProp->m_szTrace );
	SetWin_Text( this, IDC_EDIT_TRACE_2, pProp->m_szTraceBack );
	SetWin_Num_float( this, IDC_EDIT_TRACE_MOVE_1, pProp->m_fDepthUp );
	SetWin_Num_float( this, IDC_EDIT_TRACE_MOVE_2, pProp->m_fDepthDown );
	SetWin_Num_float( this, IDC_EDIT_SCALE, pProp->m_fScale );

	SetWin_Check( this, IDC_CHECK_DOT_1, pProp->m_dwFlag&USE_1POINT );
	SetWin_Check( this, IDC_CHECK_DOT_2, pProp->m_dwFlag&USE_2POINT );
	SetWin_Check( this, IDC_CHECK_DOT_MATRIX, pProp->m_dwFlag&USE_MATRIX );
	SetWin_Check( this, IDC_CHECK_DOT_MATRIX2, pProp->m_dwFlag&USE_MATRIX2 );

	SetWin_Check( this, IDC_CHECK_MOVE_AUTO, pProp->m_dwFlag&USE_AUTOMOVE );
	SetWin_Check( this, IDC_CHECK_MOVE_REPEAT, pProp->m_dwFlag&USE_REPEATMOVE );
	SetWin_Num_float( this, IDC_EDIT_MOVE_SPEED, pProp->m_fRepeatSpeed );

	SetWin_Check( this, IDC_CHECK_REPEAT, pProp->m_dwFlag&USE_REPEAT );
	SetWin_Num_float( this, IDC_EDIT_REPEAT_DELAY, pProp->m_fCoolTime );

	SetWin_Combo_Sel( this, IDC_COMBO_ANISET, GLOBAL_EFFTOOL::strEFFSINGLE_ANI[pProp->m_nAniMType].c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_WEATHERSET, GLOBAL_EFFTOOL::strEFFSINGLE_WEATHER[pProp->m_nWeather].c_str() );

	SetWin_Check( this, IDC_CHECK_MATRIX2_USE2, pProp->m_bNewMatrix );

	if ( pProp->m_dwFlag&USE_MATRIX2  )
	{
		MatrixEdit( TRUE );
	}
}

BOOL CPageEff_00_Single::DataSave()
{
	if ( !m_pPiece )	return FALSE;
	if ( !m_pEff )		return FALSE;

	std::string strFile = GetWin_Text( this, IDC_EDIT_EFFECT );
	std::string strDot1 = GetWin_Text( this, IDC_EDIT_TRACE_1 );
	std::string strDot2 = GetWin_Text( this, IDC_EDIT_TRACE_2 );

	EFFCHAR_PROPERTY_SINGLE sProperty;
	StringCchCopy( sProperty.m_szFileName , MAX_PATH, strFile.c_str() );
	StringCchCopy( sProperty.m_szTrace, STRACE_NSIZE, strDot1.c_str() );
	StringCchCopy( sProperty.m_szTraceBack, STRACE_NSIZE, strDot2.c_str() );

	sProperty.m_fDepthUp = GetWin_Num_float( this, IDC_EDIT_TRACE_MOVE_1 );
	sProperty.m_fDepthDown = GetWin_Num_float( this, IDC_EDIT_TRACE_MOVE_2 );
	sProperty.m_fScale = GetWin_Num_float( this, IDC_EDIT_SCALE );
	sProperty.m_fRepeatSpeed = GetWin_Num_float( this, IDC_EDIT_MOVE_SPEED );
	sProperty.m_fCoolTime = GetWin_Num_float( this, IDC_EDIT_REPEAT_DELAY );

	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_DOT_1 ), sProperty.m_dwFlag,USE_1POINT );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_DOT_2 ), sProperty.m_dwFlag,USE_2POINT );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_DOT_MATRIX ), sProperty.m_dwFlag,USE_MATRIX );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_DOT_MATRIX2 ), sProperty.m_dwFlag,USE_MATRIX2 );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_MOVE_AUTO ), sProperty.m_dwFlag,USE_AUTOMOVE );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_MOVE_REPEAT ), sProperty.m_dwFlag,USE_REPEATMOVE );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_REPEAT ), sProperty.m_dwFlag,USE_REPEAT );

	sProperty.m_nAniMType = GetWin_Combo_Sel( this, IDC_COMBO_ANISET );
	sProperty.m_nWeather = GetWin_Combo_Sel( this, IDC_COMBO_WEATHERSET );

	sProperty.m_bNewMatrix = GetWin_Check( this, IDC_CHECK_MATRIX2_USE2 );

	if ( m_pEff && m_pEff->GetProperty() )
	{
		EFFCHAR_PROPERTY_SINGLE* pProp = (EFFCHAR_PROPERTY_SINGLE*)m_pEff->GetProperty();
		sProperty.m_matEffLocal = pProp->m_matEffLocal;
	}

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

void CPageEff_00_Single::TraceShow()
{
	m_List_Dot1.DeleteAllItems();
	m_List_Dot2.DeleteAllItems();

	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	m_List_Dot1.SetRedraw( FALSE );
	m_List_Dot2.SetRedraw( FALSE );

	for ( int i =0; i < TOOL_EFFSINGLE_DOT_SIZE; ++i )
	{
		m_List_Dot1.InsertItem( i, GLOBAL_EFFTOOL::strEFFSINGLE_DOT[i].c_str() );
		m_List_Dot2.InsertItem( i, GLOBAL_EFFTOOL::strEFFSINGLE_DOT[i].c_str() );
	}

	for( int i = 0; i < m_List_Dot1.GetItemCount(); i++ )
	{
		std::string str = m_List_Dot1.GetItemText( i, 0 );
		SVERTEXINFLU* pTrace = m_pPiece->GetTracePos ( str.c_str() );
		if ( pTrace )
		{
			char szTempChar[512];
			sprintf( szTempChar, "(%d) %4.2f~%4.2f~%4.2f", pTrace->m_dwIndex, pTrace->m_vVector.x, pTrace->m_vVector.y, pTrace->m_vVector.z );
			m_List_Dot1.SetItemText( i, 1, szTempChar );
		}
	}

	for( int i = 0; i < m_List_Dot2.GetItemCount(); i++ )
	{
		std::string str = m_List_Dot2.GetItemText( i, 0 );
		SVERTEXINFLU* pTrace = m_pPiece->GetTracePos ( str.c_str() );
		if ( pTrace )
		{
			char szTempChar[512];
			sprintf( szTempChar, "(%d) %4.2f~%4.2f~%4.2f", pTrace->m_dwIndex, pTrace->m_vVector.x, pTrace->m_vVector.y, pTrace->m_vVector.z );
			m_List_Dot2.SetItemText( i, 1, szTempChar );
		}
	}

	m_List_Dot1.SetRedraw( TRUE );
	m_List_Dot2.SetRedraw( TRUE );
}

void CPageEff_00_Single::OnBnClickedButtonEditApply()
{
	DataSave();
}

void CPageEff_00_Single::OnBnClickedButtonEditSave()
{
	if ( DataSave() )
	{
		m_pPiece = NULL;
		m_pEff = NULL;
	
		if ( m_pSheetTab )
			m_pSheetTab->ShowMain();
	}
}

void CPageEff_00_Single::OnBnClickedButtonEditClose()
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

void CPageEff_00_Single::OnBnClickedButtonEffect()
{
	CString szFilter = "Effect File (*.egp)|*.egp|";
	CFileDialog dlg(TRUE,".egp",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this);
	dlg.m_ofn.lpstrInitialDir = DxEffSinglePropGMan::GetInstance().GetPath ();
	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, IDC_EDIT_EFFECT, dlg.GetFileName().GetString() );
}

void CPageEff_00_Single::OnNMDblclkListTrace1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	int nSelect = m_List_Dot1.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	std::string str = m_List_Dot1.GetItemText( nSelect, 0 );
	SetWin_Text( this, IDC_EDIT_TRACE_1, str.c_str());
}

void CPageEff_00_Single::OnNMDblclkListTrace2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	int nSelect = m_List_Dot2.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	std::string str = m_List_Dot2.GetItemText( nSelect, 0 );
	SetWin_Text( this, IDC_EDIT_TRACE_2, str.c_str());
}

void CPageEff_00_Single::OnBnClickedCheckDot1()
{
	SetWin_Check( this, IDC_CHECK_DOT_1, TRUE );
	SetWin_Check( this, IDC_CHECK_DOT_2, FALSE );
	SetWin_Check( this, IDC_CHECK_DOT_MATRIX, FALSE );
	SetWin_Check( this, IDC_CHECK_DOT_MATRIX2, FALSE );

	MatrixEdit( FALSE );
}

void CPageEff_00_Single::OnBnClickedCheckDot2()
{
	SetWin_Check( this, IDC_CHECK_DOT_1, FALSE );
	SetWin_Check( this, IDC_CHECK_DOT_2, TRUE );
	SetWin_Check( this, IDC_CHECK_DOT_MATRIX, FALSE );
	SetWin_Check( this, IDC_CHECK_DOT_MATRIX2, FALSE );

	MatrixEdit( FALSE );
}

void CPageEff_00_Single::OnBnClickedCheckDotMatrix()
{
	SetWin_Check( this, IDC_CHECK_DOT_1, FALSE );
	SetWin_Check( this, IDC_CHECK_DOT_2, FALSE );
	SetWin_Check( this, IDC_CHECK_DOT_MATRIX, TRUE );
	SetWin_Check( this, IDC_CHECK_DOT_MATRIX2, FALSE );

	MatrixEdit( FALSE );
}

void CPageEff_00_Single::OnBnClickedCheckDotMatrix2()
{
	SetWin_Check( this, IDC_CHECK_DOT_1, FALSE );
	SetWin_Check( this, IDC_CHECK_DOT_2, FALSE );
	SetWin_Check( this, IDC_CHECK_DOT_MATRIX, FALSE );
	SetWin_Check( this, IDC_CHECK_DOT_MATRIX2, TRUE );

	MatrixEdit( TRUE );
}

void CPageEff_00_Single::MatrixEdit( BOOL bEDIT )
{
	if ( !bEDIT )
	{
		CEditorSkinPieceView::GetView()->m_bEditMRS = FALSE;
		CEditorSkinPieceView::GetView()->m_DxEditMRS.SetMatrix( NULL );
		return;
	}

	if ( !m_pEff )	return;
	if ( !m_pEff->GetProperty() )	return;

	EFFCHAR_PROPERTY_SINGLE* pProp = (EFFCHAR_PROPERTY_SINGLE*)m_pEff->GetProperty();

	CEditorSkinPieceView::GetView()->m_bEditMRS = TRUE;
	CEditorSkinPieceView::GetView()->m_DxEditMRS.SetMatrix( &pProp->m_matEffLocal );

}