// PageEff_27_Line2Bone.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkinPiece.h"
#include "EditorSkinPieceView.h"
#include "PageEff_27_Line2Bone.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "Global.h"
#include "SelColorDlg.h"
#include "DxMethods.h"
#include "HLibDataConvert.h"

extern std::string	g_strPICK_BONE;
// CPageEff_27_Line2Bone dialog

IMPLEMENT_DYNAMIC(CPageEff_27_Line2Bone, CPropertyPage)

CPageEff_27_Line2Bone::CPageEff_27_Line2Bone(LOGFONT logfont)
	: CPropertyPage(CPageEff_27_Line2Bone::IDD)
	, m_pFont ( NULL )
	, m_pPiece( NULL )
	, m_pEff( NULL )
	, m_bADD( FALSE )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageEff_27_Line2Bone::~CPageEff_27_Line2Bone()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEff_27_Line2Bone::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_LIST_POS, m_List_Pos );
}


BEGIN_MESSAGE_MAP(CPageEff_27_Line2Bone, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_CLB_TEXTURE1, OnBnClickedButtonClbTexture1)
	ON_BN_CLICKED(IDC_BUTTON_CLB_TEXTURE2, OnBnClickedButtonClbTexture2)
	ON_BN_CLICKED(IDC_BUTTON_CLB_COLOR1, OnBnClickedButtonClbColor1)
	ON_BN_CLICKED(IDC_BUTTON_CLB_COLOR2, OnBnClickedButtonClbColor2)
	ON_BN_CLICKED(IDC_BUTTON_CLB_BONE_1, OnBnClickedButtonClbBone1)
	ON_BN_CLICKED(IDC_BUTTON_CLB_BONE_2, OnBnClickedButtonClbBone2)
	ON_BN_CLICKED(IDC_BUTTON_CLB_INSERT, OnBnClickedButtonClbInsert)
	ON_BN_CLICKED(IDC_BUTTON_CLB_DELETE, OnBnClickedButtonClbDelete)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_POS, OnNMDblclkListPos)
	ON_NOTIFY(NM_CLICK, IDC_LIST_POS, OnNMClickListPos)
END_MESSAGE_MAP()


// CPageEff_27_Line2Bone message handlers
BOOL CPageEff_27_Line2Bone::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_List_Pos.SetExtendedStyle ( m_List_Pos.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List_Pos.GetClientRect ( &rectCtrl );
	m_List_Pos.InsertColumn( 0, "Index", LVCFMT_LEFT, ( 20* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List_Pos.InsertColumn( 1, "Bone1", LVCFMT_LEFT, ( 40* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List_Pos.InsertColumn( 2, "Bone2", LVCFMT_LEFT, ( 40* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	SetWin_Combo_Init( this, IDC_COMBO_CLB_COLOR_2_TYPE, GLOBAL_EFFTOOL::strEFFL2BONE_COLORSET, TOOL_EFFL2BONE_COLORSET_SIZE );

	ResetTool();

	return TRUE;
}

void CPageEff_27_Line2Bone::DataSet( DxSkinPiece* pPiece, DxEffCharLine2BoneEff* pEff, BOOL bADD )
{
	m_pPiece = pPiece;
	m_pEff = pEff;
	m_bADD = bADD;

	ResetTool();
	DataShow();
	PositionShow();
	CEditorSkinPieceView::GetView()->m_bRenderBone = TRUE;
}

void CPageEff_27_Line2Bone::ResetTool()
{
	SetWin_Num_int( this, IDC_EDIT_CLB_PARTICLEPERSEC, 0 );
	SetWin_Num_float( this, IDC_EDIT_CLB_CREATELENGTH, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CLB_SHAKE, 1.0f );

	SetWin_Num_float( this, IDC_EDIT_CLB_LIFE_MIN, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CLB_LIFE_MAX, 1.0f );

	SetWin_Check( this, IDC_CHECK_CLB_USE_SEQ, FALSE );
	SetWin_Num_int( this, IDC_EDIT_CLB_SEQ_COL, 0 );
	SetWin_Num_int( this, IDC_EDIT_CLB_SEQ_ROW, 0 );

	SetWin_Check( this, IDC_CHECK_CLB_CAMMOVE, FALSE );

	SetWin_Num_float( this, IDC_EDIT_CLB_SCALE_X, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CLB_SCALE_Y, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CLB_SCALE_Z, 1.0f );

	SetWin_Num_float( this, IDC_EDIT_CLB_X_AXIS_X, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CLB_X_AXIS_Y, 1.0f );

	SetWin_Num_float( this, IDC_EDIT_CLB_Y_AXIS_X, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CLB_Y_AXIS_Y, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CLB_Y_AXIS_Z, 1.0f );

	SetWin_Num_float( this, IDC_EDIT_CLB_DISTANCE_MIN, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CLB_DISTANCE_MAX, 1.0f );

	SetWin_Num_float( this, IDC_EDIT_CLB_TEXROTATE_MIN, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CLB_TEXROTATE_MAX, 1.0f );

	SetWin_Text( this, IDC_EDIT_CLB_TEXTURE1, "" );
	SetWin_Text( this, IDC_EDIT_CLB_TEXTURE2, "" );

	SetWin_Num_int( this, IDC_EDIT_CLB_COLOR_1_A, 0 );
	SetWin_Num_int( this, IDC_EDIT_CLB_COLOR_1_R, 0 );
	SetWin_Num_int( this, IDC_EDIT_CLB_COLOR_1_G, 0 );
	SetWin_Num_int( this, IDC_EDIT_CLB_COLOR_1_B, 0 );

	SetWin_Num_int( this, IDC_EDIT_CLB_COLOR_2_A, 0 );
	SetWin_Num_int( this, IDC_EDIT_CLB_COLOR_2_R, 0 );
	SetWin_Num_int( this, IDC_EDIT_CLB_COLOR_2_G, 0 );
	SetWin_Num_int( this, IDC_EDIT_CLB_COLOR_2_B, 0 );

	SetWin_Combo_Sel( this, IDC_COMBO_CLB_COLOR_2_TYPE, 0 );

	//list
	m_List_Pos.DeleteAllItems();
	SetWin_Text( this, IDC_EDIT_CLB_BONE1, "" );
	SetWin_Text( this, IDC_EDIT_CLB_BONE2, "" );
}

void CPageEff_27_Line2Bone::DataShow()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	EFFCHAR_PROPERTY_LINE2BONEEFF* pProp = (EFFCHAR_PROPERTY_LINE2BONEEFF*)m_pEff->GetProperty();
	SetWin_Num_int( this, IDC_EDIT_CLB_PARTICLEPERSEC, pProp->m_dwParticlePec );
	SetWin_Num_float( this, IDC_EDIT_CLB_CREATELENGTH, pProp->m_fCreateLength );
	SetWin_Num_float( this, IDC_EDIT_CLB_SHAKE, pProp->m_fShake );
	
	SetWin_Num_float( this, IDC_EDIT_CLB_LIFE_MIN, pProp->m_vLife.x );
	SetWin_Num_float( this, IDC_EDIT_CLB_LIFE_MAX, pProp->m_vLife.y );

	SetWin_Check( this, IDC_CHECK_CLB_USE_SEQ, pProp->m_dwFlag&USE_SEQUENCE );
	SetWin_Num_int( this, IDC_EDIT_CLB_SEQ_COL, pProp->m_dwSequenceCol );
	SetWin_Num_int( this, IDC_EDIT_CLB_SEQ_ROW, pProp->m_dwSequenceRow );

	SetWin_Check( this, IDC_CHECK_CLB_CAMMOVE, pProp->m_bWithCamMove );

	SetWin_Num_float( this, IDC_EDIT_CLB_SCALE_X, pProp->m_vSize.x );
	SetWin_Num_float( this, IDC_EDIT_CLB_SCALE_Y, pProp->m_vSize.y );
	SetWin_Num_float( this, IDC_EDIT_CLB_SCALE_Z, pProp->m_vSize.z );

	SetWin_Num_float( this, IDC_EDIT_CLB_X_AXIS_X, pProp->m_vScaleX.x );
	SetWin_Num_float( this, IDC_EDIT_CLB_X_AXIS_Y, pProp->m_vScaleX.y );

	SetWin_Num_float( this, IDC_EDIT_CLB_Y_AXIS_X, pProp->m_vScaleY.x );
	SetWin_Num_float( this, IDC_EDIT_CLB_Y_AXIS_Y, pProp->m_vScaleY.y );
	SetWin_Num_float( this, IDC_EDIT_CLB_Y_AXIS_Z, pProp->m_vScaleY.z );

	SetWin_Num_float( this, IDC_EDIT_CLB_DISTANCE_MIN, pProp->m_fMinMaxDist.min );
	SetWin_Num_float( this, IDC_EDIT_CLB_DISTANCE_MAX, pProp->m_fMinMaxDist.max );

	SetWin_Num_float( this, IDC_EDIT_CLB_TEXROTATE_MIN, pProp->m_fMinMaxTexRotate.min );
	SetWin_Num_float( this, IDC_EDIT_CLB_TEXROTATE_MAX, pProp->m_fMinMaxTexRotate.max );

	SetWin_Text( this, IDC_EDIT_CLB_TEXTURE1, pProp->m_szTexture );
	SetWin_Text( this, IDC_EDIT_CLB_TEXTURE2, pProp->m_szTexture2 );
	
	SetWin_Num_int( this, IDC_EDIT_CLB_COLOR_1_R, RGBA_GETRED( pProp->m_dwColor ) );
	SetWin_Num_int( this, IDC_EDIT_CLB_COLOR_1_G, RGBA_GETGREEN( pProp->m_dwColor ) );
	SetWin_Num_int( this, IDC_EDIT_CLB_COLOR_1_B, RGBA_GETBLUE( pProp->m_dwColor ) );
	SetWin_Num_int( this, IDC_EDIT_CLB_COLOR_1_A, RGBA_GETALPHA( pProp->m_dwColor ) );

	SetWin_Num_int( this, IDC_EDIT_CLB_COLOR_2_R, RGBA_GETRED( pProp->m_dwColor2 ) );
	SetWin_Num_int( this, IDC_EDIT_CLB_COLOR_2_G, RGBA_GETGREEN( pProp->m_dwColor2 ) );
	SetWin_Num_int( this, IDC_EDIT_CLB_COLOR_2_B, RGBA_GETBLUE( pProp->m_dwColor2 ) );
	SetWin_Num_int( this, IDC_EDIT_CLB_COLOR_2_A, RGBA_GETALPHA( pProp->m_dwColor2 ) );

	SetWin_Combo_Sel( this, IDC_COMBO_CLB_COLOR_2_TYPE, GLOBAL_EFFTOOL::strEFFL2BONE_COLORSET[pProp->m_dwOtherColor].c_str() );
}

BOOL CPageEff_27_Line2Bone::DataSave()
{
	if ( !m_pPiece )	return FALSE;
	if ( !m_pEff )		return FALSE;

	EFFCHAR_PROPERTY_LINE2BONEEFF sProperty;

	sProperty.m_dwParticlePec = GetWin_Num_int( this, IDC_EDIT_CLB_PARTICLEPERSEC );
	sProperty.m_fCreateLength = GetWin_Num_float( this, IDC_EDIT_CLB_CREATELENGTH );
	sProperty.m_fShake = GetWin_Num_float( this, IDC_EDIT_CLB_SHAKE );
	
	sProperty.m_vLife.x = GetWin_Num_float( this, IDC_EDIT_CLB_LIFE_MIN );
	sProperty.m_vLife.y = GetWin_Num_float( this, IDC_EDIT_CLB_LIFE_MAX );

	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CLB_USE_SEQ ), sProperty.m_dwFlag, USE_SEQUENCE );
	sProperty.m_dwSequenceCol = GetWin_Num_int( this, IDC_EDIT_CLB_SEQ_COL );
	sProperty.m_dwSequenceRow = GetWin_Num_int( this, IDC_EDIT_CLB_SEQ_ROW );

	sProperty.m_bWithCamMove = ( GetWin_Check( this, IDC_CHECK_CLB_CAMMOVE ) == TRUE );

	sProperty.m_vSize.x = GetWin_Num_float( this, IDC_EDIT_CLB_SCALE_X );
	sProperty.m_vSize.y = GetWin_Num_float( this, IDC_EDIT_CLB_SCALE_Y );;
	sProperty.m_vSize.z = GetWin_Num_float( this, IDC_EDIT_CLB_SCALE_Z );

	sProperty.m_vScaleX.x = GetWin_Num_float( this, IDC_EDIT_CLB_X_AXIS_X );
	sProperty.m_vScaleX.y = GetWin_Num_float( this, IDC_EDIT_CLB_X_AXIS_Y );

	sProperty.m_vScaleY.x = GetWin_Num_float( this, IDC_EDIT_CLB_Y_AXIS_X );
	sProperty.m_vScaleY.y = GetWin_Num_float( this, IDC_EDIT_CLB_Y_AXIS_Y );
	sProperty.m_vScaleY.z = GetWin_Num_float( this, IDC_EDIT_CLB_Y_AXIS_Z );

	sProperty.m_fMinMaxDist.min = GetWin_Num_float( this, IDC_EDIT_CLB_DISTANCE_MIN );
	sProperty.m_fMinMaxDist.max = GetWin_Num_float( this, IDC_EDIT_CLB_DISTANCE_MAX );

	sProperty.m_fMinMaxTexRotate.min = GetWin_Num_float( this, IDC_EDIT_CLB_TEXROTATE_MIN );
	sProperty.m_fMinMaxTexRotate.max = GetWin_Num_float( this, IDC_EDIT_CLB_TEXROTATE_MAX );

	std::string strTexture1 = GetWin_Text( this, IDC_EDIT_CLB_TEXTURE1 );
	std::string strTexture2 = GetWin_Text( this, IDC_EDIT_CLB_TEXTURE2 );

	StringCchCopy( sProperty.m_szTexture, BONELISTEFF_TEXNAME_SIZE, strTexture1.c_str() );
	StringCchCopy( sProperty.m_szTexture2 , BONELISTEFF_TEXNAME_SIZE, strTexture2.c_str() );

	sProperty.m_dwColor = RGBA_MAKE(
		GetWin_Num_int( this, IDC_EDIT_CLB_COLOR_1_R ), 
		GetWin_Num_int( this, IDC_EDIT_CLB_COLOR_1_G ), 
		GetWin_Num_int( this, IDC_EDIT_CLB_COLOR_1_B ),
		GetWin_Num_int( this, IDC_EDIT_CLB_COLOR_1_A ));

	sProperty.m_dwColor2 = RGBA_MAKE(
		GetWin_Num_int( this, IDC_EDIT_CLB_COLOR_2_R ), 
		GetWin_Num_int( this, IDC_EDIT_CLB_COLOR_2_G ), 
		GetWin_Num_int( this, IDC_EDIT_CLB_COLOR_2_B ),
		GetWin_Num_int( this, IDC_EDIT_CLB_COLOR_2_A ));

	sProperty.m_dwOtherColor = GetWin_Combo_Sel( this, IDC_COMBO_CLB_COLOR_2_TYPE );

	m_pEff->SetProperty( &sProperty );
	HRESULT hr = m_pEff->Create( CEditorSkinPieceView::GetView()->Get3DDevice() );
	if ( FAILED(hr) )
	{
		return FALSE;
	}

	if ( m_bADD )
	{
		m_pPiece->AddEffList( m_pEff );
		m_bADD = FALSE;
	}

	return TRUE;
}

void CPageEff_27_Line2Bone::PositionShow()
{
	m_List_Pos.DeleteAllItems();

	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	m_List_Pos.SetRedraw( FALSE );

	DWORD dwNUM = m_pEff->GetLineSize();
	for( DWORD i=0; i< dwNUM; ++ i )
	{
		std::string strBone1 = "";
		std::string strBone2 = "";
		m_pEff->GetBoneName( i, strBone1, strBone2 );

		m_List_Pos.InsertItem( i, _HLIB::cstring_dword(i).GetString() );
		m_List_Pos.SetItemText( i, 1, strBone1.c_str() );
		m_List_Pos.SetItemText( i, 2, strBone2.c_str() );
		m_List_Pos.SetItemData( i, i );
	}

	m_List_Pos.SetRedraw( TRUE );

	SetWin_Text( this, IDC_EDIT_CLB_BONE1, "" );
	SetWin_Text( this, IDC_EDIT_CLB_BONE2, "" );
}

void CPageEff_27_Line2Bone::OnBnClickedButtonApply()
{
	DataSave();
}

void CPageEff_27_Line2Bone::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		m_pPiece = NULL;
		m_pEff = NULL;

		if ( m_pSheetTab )
			m_pSheetTab->ShowMain();
	}
}

void CPageEff_27_Line2Bone::OnBnClickedButtonClose()
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

void CPageEff_27_Line2Bone::OnBnClickedButtonClbTexture1()
{
	CString szFilter = "Texture Image (*.bmp,*.dds,*.tga,*.jpg)|*.bmp;*.dds;*.tga;*.jpg|";
	CFileDialog dlg(TRUE,".",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = TextureManager::GetTexturePath(); 
	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, IDC_EDIT_CLB_TEXTURE1, dlg.GetFileName().GetString() );
}

void CPageEff_27_Line2Bone::OnBnClickedButtonClbTexture2()
{
	CString szFilter = "Texture Image (*.bmp,*.dds,*.tga,*.jpg)|*.bmp;*.dds;*.tga;*.jpg|";
	CFileDialog dlg(TRUE,".",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = TextureManager::GetTexturePath(); 
	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, IDC_EDIT_CLB_TEXTURE2, dlg.GetFileName().GetString() );
}

void CPageEff_27_Line2Bone::OnBnClickedButtonClbColor1()
{
	BYTE nCUR_R = GetWin_Num_int( this, IDC_EDIT_CLB_COLOR_1_R );
	BYTE nCUR_G = GetWin_Num_int( this, IDC_EDIT_CLB_COLOR_1_G );
	BYTE nCUR_B = GetWin_Num_int( this, IDC_EDIT_CLB_COLOR_1_B );

	CSelColorDlg dlg( nCUR_R, nCUR_G, nCUR_B );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_int( this, IDC_EDIT_CLB_COLOR_1_R, GetRValue( crNewColor ) );
		SetWin_Num_int( this, IDC_EDIT_CLB_COLOR_1_G, GetGValue( crNewColor ) );
		SetWin_Num_int( this, IDC_EDIT_CLB_COLOR_1_B, GetBValue( crNewColor ) );
	}
}

void CPageEff_27_Line2Bone::OnBnClickedButtonClbColor2()
{
	BYTE nCUR_R = GetWin_Num_int( this, IDC_EDIT_CLB_COLOR_2_R );
	BYTE nCUR_G = GetWin_Num_int( this, IDC_EDIT_CLB_COLOR_2_G );
	BYTE nCUR_B = GetWin_Num_int( this, IDC_EDIT_CLB_COLOR_2_B );

	CSelColorDlg dlg( nCUR_R, nCUR_G, nCUR_B );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_int( this, IDC_EDIT_CLB_COLOR_2_R, GetRValue( crNewColor ) );
		SetWin_Num_int( this, IDC_EDIT_CLB_COLOR_2_G, GetGValue( crNewColor ) );
		SetWin_Num_int( this, IDC_EDIT_CLB_COLOR_2_B, GetBValue( crNewColor ) );
	}
}

void CPageEff_27_Line2Bone::OnBnClickedButtonClbBone1()
{
	if ( g_strPICK_BONE.size() <= 0 )
	{
		CDebugSet::MsgBox( "No Bone Selected!" );
		return;
	}else{
		SetWin_Text( this, IDC_EDIT_CLB_BONE1, g_strPICK_BONE.c_str() );
	}
}

void CPageEff_27_Line2Bone::OnBnClickedButtonClbBone2()
{
	if ( g_strPICK_BONE.size() <= 0 )
	{
		CDebugSet::MsgBox( "No Bone Selected!" );
		return;
	}else{
		SetWin_Text( this, IDC_EDIT_CLB_BONE2, g_strPICK_BONE.c_str() );
	}
}

void CPageEff_27_Line2Bone::OnBnClickedButtonClbInsert()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	std::string strBone1 = GetWin_Text( this, IDC_EDIT_CLB_BONE1 );
	std::string strBone2 = GetWin_Text( this, IDC_EDIT_CLB_BONE2 );

	m_pEff->InsertLine( strBone1.c_str(), strBone2.c_str() );
	PositionShow();
}

void CPageEff_27_Line2Bone::OnBnClickedButtonClbDelete()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	int nSelect = m_List_Pos.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	if ( m_pPiece && m_pEff )
	{
		m_pEff->DeleteLine( nSelect );
		PositionShow();
	}
}

void CPageEff_27_Line2Bone::OnNMDblclkListPos(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void CPageEff_27_Line2Bone::OnNMClickListPos(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	int nSelect = m_List_Pos.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	m_pEff->SelectLine( nSelect );
}
