// PageEff_24_BoneList.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkinPiece.h"
#include "EditorSkinPieceView.h"
#include "PageEff_24_BoneList.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "SelColorDlg.h"
#include "DxMethods.h"
#include "HLibDataConvert.h"

extern std::string	g_strPICK_BONE;
// CPageEff_24_BoneList dialog

IMPLEMENT_DYNAMIC(CPageEff_24_BoneList, CPropertyPage)

CPageEff_24_BoneList::CPageEff_24_BoneList(LOGFONT logfont)
	: CPropertyPage(CPageEff_24_BoneList::IDD)
	, m_pFont ( NULL )
	, m_pPiece( NULL )
	, m_pEff( NULL )
	, m_bADD( FALSE )
{
	D3DXMatrixIdentity ( &m_matEdit );
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageEff_24_BoneList::~CPageEff_24_BoneList()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEff_24_BoneList::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_LIST_POS, m_List_Pos );
}


BEGIN_MESSAGE_MAP(CPageEff_24_BoneList, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_CBL_TEXTURE, OnBnClickedButtonCblTexture)
	ON_BN_CLICKED(IDC_BUTTON_CBL_COLOR, OnBnClickedButtonCblColor)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_POS, OnNMDblclkListPos)
	ON_BN_CLICKED(IDC_BUTTON_CBL_LIST_ADD, OnBnClickedButtonCblListAdd)
	ON_BN_CLICKED(IDC_BUTTON_CBL_LIST_EDIT, OnBnClickedButtonCblListEdit)
	ON_BN_CLICKED(IDC_BUTTON_CBL_LIST_DEL, OnBnClickedButtonCblListDel)
	ON_BN_CLICKED(IDC_BUTTON_CBL_LIST_UP, OnBnClickedButtonCblListUp)
	ON_BN_CLICKED(IDC_BUTTON_CBL_LIST_DOWN, OnBnClickedButtonCblListDown)
	ON_BN_CLICKED(IDC_BUTTON_EDITPOS_RESET, OnBnClickedButtonEditposReset)
	ON_BN_CLICKED(IDC_BUTTON_EDITPOS_SAVE, OnBnClickedButtonEditposSave)
END_MESSAGE_MAP()


// CPageEff_24_BoneList message handlers
BOOL CPageEff_24_BoneList::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_List_Pos.SetExtendedStyle ( m_List_Pos.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List_Pos.GetClientRect ( &rectCtrl );
	m_List_Pos.InsertColumn( 0, "Index", LVCFMT_LEFT, ( 20* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List_Pos.InsertColumn( 1, "POS(X/Y/Z)", LVCFMT_LEFT, ( 80* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	ResetTool();

	return TRUE;
}

void CPageEff_24_BoneList::DataSet( DxSkinPiece* pPiece, DxEffCharBoneListEff* pEff, BOOL bADD )
{
	m_pPiece = pPiece;
	m_pEff = pEff;
	m_bADD = bADD;

	ResetTool();
	DataShow();
	PositionShow();
	CEditorSkinPieceView::GetView()->m_bRenderBone = TRUE;
}

void CPageEff_24_BoneList::ResetTool()
{
	SetWin_Num_int( this, IDC_EDIT_CBL_PARTICLEPERSEC, 0 );
	SetWin_Num_float( this, IDC_EDIT_CBL_CREATELENGTH, 1.0f );

	SetWin_Num_float( this, IDC_EDIT_CBL_LIFE_MIN, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CBL_LIFE_MAX, 1.0f );

	SetWin_Num_float( this, IDC_EDIT_CBL_SCALE_X, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CBL_SCALE_Y, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CBL_SCALE_Z, 1.0f );

	SetWin_Check( this, IDC_CHECK_CBL_ROTATE, FALSE );
	SetWin_Num_float( this, IDC_EDIT_CBL_ROTATE_MIN, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CBL_ROTATE_MAX, 1.0f );

	SetWin_Check( this, IDC_CHECK_CBL_SEQUENCE, FALSE );
	SetWin_Num_int( this, IDC_EDIT_CBL_COL, 0 );
	SetWin_Num_int( this, IDC_EDIT_CBL_ROW, 0 );

	SetWin_Check( this, IDC_CHECK_CBL_REMAIN, FALSE );
	SetWin_Num_float( this, IDC_EDIT_CBL_VEL_MIN, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CBL_VEL_MAX, 1.0f );

	SetWin_Text( this, IDC_EDIT_CBL_TEXTURE, "" );

	SetWin_Num_int( this, IDC_EDIT_CBL_COLOR_A, 0 );
	SetWin_Num_int( this, IDC_EDIT_CBL_COLOR_R, 0 );
	SetWin_Num_int( this, IDC_EDIT_CBL_COLOR_G, 0 );
	SetWin_Num_int( this, IDC_EDIT_CBL_COLOR_B, 0 );

	m_List_Pos.DeleteAllItems();

	PosEdit( FALSE );
}

void CPageEff_24_BoneList::DataShow()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	EFFCHAR_PROPERTY_BONELISTEFF* pProp = (EFFCHAR_PROPERTY_BONELISTEFF*)m_pEff->GetProperty();

	SetWin_Num_int( this, IDC_EDIT_CBL_PARTICLEPERSEC, pProp->m_dwParticlePec );
	SetWin_Num_float( this, IDC_EDIT_CBL_CREATELENGTH, pProp->m_fCreateLength );

	SetWin_Num_float( this, IDC_EDIT_CBL_LIFE_MIN, pProp->m_vLife.x );
	SetWin_Num_float( this, IDC_EDIT_CBL_LIFE_MAX, pProp->m_vLife.y );

	SetWin_Num_float( this, IDC_EDIT_CBL_SCALE_X, pProp->m_vSize.x );
	SetWin_Num_float( this, IDC_EDIT_CBL_SCALE_Y, pProp->m_vSize.y );
	SetWin_Num_float( this, IDC_EDIT_CBL_SCALE_Z, pProp->m_vSize.z );

	SetWin_Check( this, IDC_CHECK_CBL_ROTATE, pProp->m_dwFlag&USE_ROTATE );
	SetWin_Num_float( this, IDC_EDIT_CBL_ROTATE_MIN, pProp->m_vRotate.x );
	SetWin_Num_float( this, IDC_EDIT_CBL_ROTATE_MAX, pProp->m_vRotate.y );

	SetWin_Check( this, IDC_CHECK_CBL_SEQUENCE, pProp->m_dwFlag&USE_SEQUENCE );
	SetWin_Num_int( this, IDC_EDIT_CBL_COL, pProp->m_dwSequenceCol );
	SetWin_Num_int( this, IDC_EDIT_CBL_ROW, pProp->m_dwSequenceRow );

	SetWin_Check( this, IDC_CHECK_CBL_REMAIN, pProp->m_dwFlag&USE_REMAIN );
	SetWin_Num_float( this, IDC_EDIT_CBL_VEL_MIN, pProp->m_vVelocity.x );
	SetWin_Num_float( this, IDC_EDIT_CBL_VEL_MAX, pProp->m_vVelocity.y );
	
	SetWin_Text( this, IDC_EDIT_CBL_TEXTURE, pProp->m_szTexture );
	
	SetWin_Num_int( this, IDC_EDIT_CBL_COLOR_R, RGBA_GETRED( pProp->m_dwColor ) );
	SetWin_Num_int( this, IDC_EDIT_CBL_COLOR_G, RGBA_GETGREEN( pProp->m_dwColor ) );
	SetWin_Num_int( this, IDC_EDIT_CBL_COLOR_B, RGBA_GETBLUE( pProp->m_dwColor ) );
	SetWin_Num_int( this, IDC_EDIT_CBL_COLOR_A, RGBA_GETALPHA( pProp->m_dwColor ) );
}

BOOL CPageEff_24_BoneList::DataSave()
{
	if ( !m_pPiece )	return FALSE;
	if ( !m_pEff )		return FALSE;

	EFFCHAR_PROPERTY_BONELISTEFF sProperty;
	sProperty.m_dwParticlePec = GetWin_Num_int( this, IDC_EDIT_CBL_PARTICLEPERSEC );
	sProperty.m_fCreateLength = GetWin_Num_float( this, IDC_EDIT_CBL_CREATELENGTH );

	sProperty.m_vLife.x = GetWin_Num_float( this, IDC_EDIT_CBL_LIFE_MIN );
	sProperty.m_vLife.y = GetWin_Num_float( this, IDC_EDIT_CBL_LIFE_MAX );

	sProperty.m_vSize.x = GetWin_Num_float( this, IDC_EDIT_CBL_SCALE_X );
	sProperty.m_vSize.y = GetWin_Num_float( this, IDC_EDIT_CBL_SCALE_Y );
	sProperty.m_vSize.z = GetWin_Num_float( this, IDC_EDIT_CBL_SCALE_Z );

	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CBL_ROTATE ), sProperty.m_dwFlag, USE_ROTATE );
	sProperty.m_vRotate.x = GetWin_Num_float( this, IDC_EDIT_CBL_ROTATE_MIN );
	sProperty.m_vRotate.y = GetWin_Num_float( this, IDC_EDIT_CBL_ROTATE_MAX );

	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CBL_SEQUENCE ), sProperty.m_dwFlag, USE_SEQUENCE );
	sProperty.m_dwSequenceCol = GetWin_Num_int( this, IDC_EDIT_CBL_COL );
	sProperty.m_dwSequenceRow = GetWin_Num_int( this, IDC_EDIT_CBL_ROW );

	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CBL_REMAIN ), sProperty.m_dwFlag, USE_REMAIN );
	sProperty.m_vVelocity.x = GetWin_Num_float( this, IDC_EDIT_CBL_VEL_MIN );
	sProperty.m_vVelocity.y = GetWin_Num_float( this, IDC_EDIT_CBL_VEL_MAX );
	
	std::string strTex = GetWin_Text( this, IDC_EDIT_CBL_TEXTURE );
	StringCchCopy( sProperty.m_szTexture , BONELISTEFF_TEXNAME_SIZE, strTex.c_str() );
	
	sProperty.m_dwColor = RGBA_MAKE(
		GetWin_Num_int( this, IDC_EDIT_CBL_COLOR_R ), 
		GetWin_Num_int( this, IDC_EDIT_CBL_COLOR_G ), 
		GetWin_Num_int( this, IDC_EDIT_CBL_COLOR_B ),
		GetWin_Num_int( this, IDC_EDIT_CBL_COLOR_A ));
	
	m_pEff->SetProperty( &sProperty );
	m_pEff->EditApplyResetData();

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

void CPageEff_24_BoneList::PositionShow()
{
	m_List_Pos.DeleteAllItems();

	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	m_List_Pos.SetRedraw( FALSE );

	DWORD dwSIZE = m_pEff->GetCustomBoneSize();
	for( DWORD i=0; i<dwSIZE; ++ i )
	{
		m_List_Pos.InsertItem( i, _HLIB::cstring_dword(i).GetString() );
		m_List_Pos.SetItemText( i, 1, m_pEff->GetBoneString( i ).c_str());
		m_List_Pos.SetItemData( i, i );
	}

	m_List_Pos.SetRedraw( TRUE );

	PosEdit( FALSE );

	m_pEff->EditApplyResetData();

	CEditorSkinPieceView::GetView()->m_bEditMRS = FALSE;
	CEditorSkinPieceView::GetView()->m_DxEditMRS.SetMatrix( NULL );
	m_matEdit = D3DXMATRIX();
}

void CPageEff_24_BoneList::PosEdit( BOOL bEDIT )
{
	SetWin_Check( this, IDC_CHECK_EDITPOS_USEOFFSET, FALSE );
	SetWin_Num_float( this, IDC_EDIT_EDITPOS_OFFSET_X, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_EDITPOS_OFFSET_Y, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_EDITPOS_OFFSET_Z, 1.0f );

	SetWin_Enable( this, IDC_CHECK_EDITPOS_USEOFFSET, bEDIT );
	SetWin_Enable( this, IDC_EDIT_EDITPOS_OFFSET_X, bEDIT );
	SetWin_Enable( this, IDC_EDIT_EDITPOS_OFFSET_Y, bEDIT );
	SetWin_Enable( this, IDC_EDIT_EDITPOS_OFFSET_Z, bEDIT );
	SetWin_Enable( this, IDC_BUTTON_EDITPOS_RESET, bEDIT );
	SetWin_Enable( this, IDC_BUTTON_EDITPOS_SAVE, bEDIT );

	SetWin_Enable( this, IDC_BUTTON_CBL_LIST_ADD, !bEDIT );
	SetWin_Enable( this, IDC_BUTTON_CBL_LIST_EDIT, !bEDIT );
	SetWin_Enable( this, IDC_BUTTON_CBL_LIST_DEL, !bEDIT );
	SetWin_Enable( this, IDC_BUTTON_CBL_LIST_UP, !bEDIT );
	SetWin_Enable( this, IDC_BUTTON_CBL_LIST_DOWN, !bEDIT );
}

void CPageEff_24_BoneList::PositionEdit()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	int nSelect = m_List_Pos.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	m_pEff->SetCheckBone( nSelect );

	BOOL bUseOffSet = m_pEff->GetEnableBonePos();
	D3DXVECTOR3 vPOS = m_pEff->GetInfluencePosition();
	m_matEdit = m_pEff->GetInfluenceMatrix();

	PosEdit( TRUE );
	SetWin_Check( this, IDC_CHECK_EDITPOS_USEOFFSET, bUseOffSet );
	SetWin_Num_float( this, IDC_EDIT_EDITPOS_OFFSET_X, vPOS.x );
	SetWin_Num_float( this, IDC_EDIT_EDITPOS_OFFSET_Y, vPOS.y );
	SetWin_Num_float( this, IDC_EDIT_EDITPOS_OFFSET_Z, vPOS.z );

	CEditorSkinPieceView::GetView()->m_bEditMRS = TRUE;
	CEditorSkinPieceView::GetView()->m_DxEditMRS.SetMatrix( &m_matEdit );
}

void CPageEff_24_BoneList::OnBnClickedButtonApply()
{
	DataSave();
}

void CPageEff_24_BoneList::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		m_pPiece = NULL;
		m_pEff = NULL;

		if ( m_pSheetTab )
			m_pSheetTab->ShowMain();
	}
}

void CPageEff_24_BoneList::OnBnClickedButtonClose()
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

void CPageEff_24_BoneList::OnBnClickedButtonCblTexture()
{
	CString szFilter = "Texture Image (*.bmp,*.dds,*.tga,*.jpg)|*.bmp;*.dds;*.tga;*.jpg|";
	CFileDialog dlg(TRUE,".",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = TextureManager::GetTexturePath(); 
	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, IDC_EDIT_CBL_TEXTURE, dlg.GetFileName().GetString() );
}

void CPageEff_24_BoneList::OnBnClickedButtonCblColor()
{
	BYTE nCUR_R = GetWin_Num_int( this, IDC_EDIT_CBL_COLOR_R );
	BYTE nCUR_G = GetWin_Num_int( this, IDC_EDIT_CBL_COLOR_G );
	BYTE nCUR_B = GetWin_Num_int( this, IDC_EDIT_CBL_COLOR_B );

	CSelColorDlg dlg( nCUR_R, nCUR_G, nCUR_B );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_int( this, IDC_EDIT_CBL_COLOR_R, GetRValue( crNewColor ) );
		SetWin_Num_int( this, IDC_EDIT_CBL_COLOR_G, GetGValue( crNewColor ) );
		SetWin_Num_int( this, IDC_EDIT_CBL_COLOR_B, GetBValue( crNewColor ) );
	}
}

void CPageEff_24_BoneList::OnNMDblclkListPos(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	PositionEdit();
}

void CPageEff_24_BoneList::OnBnClickedButtonCblListAdd()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	if ( g_strPICK_BONE.size() <= 0 )
	{
		CDebugSet::MsgBox( "No Bone Selected!" );
		return;
	}else{
		m_pEff->InsertBone( g_strPICK_BONE.c_str() );
		PositionShow();
	}
}

void CPageEff_24_BoneList::OnBnClickedButtonCblListEdit()
{
	PositionEdit();
}

void CPageEff_24_BoneList::OnBnClickedButtonCblListDel()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	int nSelect = m_List_Pos.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	m_pEff->DeleteBone( nSelect );
	PositionShow();
}

void CPageEff_24_BoneList::OnBnClickedButtonCblListUp()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	int nSelect = m_List_Pos.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	m_pEff->DataUp( nSelect );
	PositionShow();
}

void CPageEff_24_BoneList::OnBnClickedButtonCblListDown()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	int nSelect = m_List_Pos.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	m_pEff->DataDown( nSelect );
	PositionShow();
}

void CPageEff_24_BoneList::OnBnClickedButtonEditposReset()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	m_pEff->ResetLocalPos();
	PositionShow();
}

void CPageEff_24_BoneList::OnBnClickedButtonEditposSave()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	BOOL bUSE = GetWin_Check( this, IDC_CHECK_EDITPOS_USEOFFSET );
	m_pEff->SetCheckBoneOffSetPos( bUSE );
	m_pEff->SetChangeMatrix( m_matEdit );
	PositionShow();
}
