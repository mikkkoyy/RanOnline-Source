// PageEff_25_UserColor.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkinPiece.h"
#include "EditorSkinPieceView.h"
#include "PageEff_25_UserColor.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "SelColorDlg.h"
#include "HLibDataConvert.h"

// CPageEff_25_UserColor dialog

IMPLEMENT_DYNAMIC(CPageEff_25_UserColor, CPropertyPage)

CPageEff_25_UserColor::CPageEff_25_UserColor(LOGFONT logfont)
	: CPropertyPage(CPageEff_25_UserColor::IDD)
	, m_pFont ( NULL )
	, m_pPiece( NULL )
	, m_pEff( NULL )
	, m_bADD( FALSE )
	, m_nMatEdit( -1 )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageEff_25_UserColor::~CPageEff_25_UserColor()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEff_25_UserColor::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_LIST_MATERIAL, m_List_Material );
}


BEGIN_MESSAGE_MAP(CPageEff_25_UserColor, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_CUC_COLOR, OnBnClickedButtonCucColor)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_MATERIAL, OnNMDblclkListMaterial)
	ON_BN_CLICKED(IDC_BUTTON_MATERIAL_TEXTURE, OnBnClickedButtonMaterialTexture)
	ON_BN_CLICKED(IDC_BUTTON_MATERIAL_OK, OnBnClickedButtonMaterialOk)
END_MESSAGE_MAP()


// CPageEff_25_UserColor message handlers
BOOL CPageEff_25_UserColor::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_List_Material.SetExtendedStyle ( m_List_Material.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List_Material.GetClientRect ( &rectCtrl );
	m_List_Material.InsertColumn( 0, "Used", LVCFMT_LEFT, ( 10* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List_Material.InsertColumn( 1, "Material", LVCFMT_LEFT, ( 40* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List_Material.InsertColumn( 2, "EffectTexture", LVCFMT_LEFT, ( 40* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List_Material.InsertColumn( 3, "Power", LVCFMT_LEFT, ( 10* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	ResetTool();

	return TRUE;
}

void CPageEff_25_UserColor::DataSet( DxSkinPiece* pPiece, DxEffCharUserColor* pEff, BOOL bADD )
{
	m_pPiece = pPiece;
	m_pEff = pEff;
	m_bADD = bADD;

	if ( m_pPiece && m_pPiece->m_pmcMesh && m_pEff && m_bADD )
	{
		//materials should be built manually for new effect( m_bAdd )
		DWORD dwMaterialNum = m_pPiece->m_pmcMesh->GetNumMaterials();
		DXMATERIAL_CHAR_EFF* pMaterials = new DXMATERIAL_CHAR_EFF[dwMaterialNum];

		for ( DWORD i=0; i<dwMaterialNum; i++ )
		{
			StringCchCopy( pMaterials[i].szTexture,	MAX_PATH, m_pPiece->m_pmcMesh->pMaterials[i].pTextureFilename );
		}

		m_pEff->SetMaterials( CEditorSkinPieceView::GetView()->Get3DDevice(), dwMaterialNum, pMaterials );
		SAFE_DELETE_ARRAY ( pMaterials );
	}

	ResetTool();
	DataShow();
	MaterialShow();
}

void CPageEff_25_UserColor::ResetTool()
{
	SetWin_Num_float( this, IDC_EDIT_CUC_COLOR_A, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CUC_COLOR_R, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CUC_COLOR_G, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CUC_COLOR_B, 1.0f );

	SetWin_Check( this, IDC_CHECK_CUC_FLAG_USE_COLOR_1, FALSE );
	SetWin_Check( this, IDC_CHECK_CUC_FLAG_USE_COLOR_2, FALSE );

	//material
	m_List_Material.DeleteAllItems();
	SetWin_Enable( this, IDC_BUTTON_MATERIAL_TEXTURE, FALSE );
	SetWin_Enable( this, IDC_CHECK_MATERIAL_USE, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATERIAL_TEXTURE, FALSE );
	SetWin_Enable( this, IDC_BUTTON_MATERIAL_OK, FALSE );

	SetWin_Check( this, IDC_CHECK_MATERIAL_USE, FALSE );
	SetWin_Text( this, IDC_EDIT_MATERIAL_TEXTURE, "" );

	m_nMatEdit = -1;
}

void CPageEff_25_UserColor::DataShow()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	EFFCHAR_PROPERTY_USERCOLOR* pProp = (EFFCHAR_PROPERTY_USERCOLOR*)m_pEff->GetProperty();
	SetWin_Num_float( this, IDC_EDIT_CUC_COLOR_A, pProp->m_cColor.a );
	SetWin_Num_float( this, IDC_EDIT_CUC_COLOR_R, pProp->m_cColor.r );
	SetWin_Num_float( this, IDC_EDIT_CUC_COLOR_G, pProp->m_cColor.g );
	SetWin_Num_float( this, IDC_EDIT_CUC_COLOR_B, pProp->m_cColor.b );

	SetWin_Check( this, IDC_CHECK_CUC_FLAG_USE_COLOR_1, pProp->m_dwFlage&USEUSERCOLOR1 );
	SetWin_Check( this, IDC_CHECK_CUC_FLAG_USE_COLOR_2, pProp->m_dwFlage&USEUSERCOLOR2 );
}

BOOL CPageEff_25_UserColor::DataSave()
{
	if ( !m_pPiece )	return FALSE;
	if ( !m_pEff )		return FALSE;

	EFFCHAR_PROPERTY_USERCOLOR sProperty;
	sProperty.m_cColor.a = GetWin_Num_float( this, IDC_EDIT_CUC_COLOR_A );
	sProperty.m_cColor.r = GetWin_Num_float( this, IDC_EDIT_CUC_COLOR_R );
	sProperty.m_cColor.g = GetWin_Num_float( this, IDC_EDIT_CUC_COLOR_G );
	sProperty.m_cColor.b = GetWin_Num_float( this, IDC_EDIT_CUC_COLOR_B );

	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CUC_FLAG_USE_COLOR_1 ), sProperty.m_dwFlage, USEUSERCOLOR1 );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CUC_FLAG_USE_COLOR_2 ), sProperty.m_dwFlage, USEUSERCOLOR2 );

	//must recopy materials num to new property
	sProperty.m_dwMaterials = m_pEff->GetMaterialsNum();

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

void CPageEff_25_UserColor::MaterialShow()
{
	int nSelect = m_List_Material.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	m_List_Material.DeleteAllItems();

	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	m_List_Material.SetRedraw( FALSE );

	//if GetMaterialsNum() return mismatched array count of GetMaterials() = Crash!
	DWORD dwMaterialNum = m_pEff->GetMaterialsNum();
	DXMATERIAL_CHAR_EFF* pMaterials = m_pEff->GetMaterials();

	std::string str[2] = {"No","Yes"};

	for ( DWORD i=0; i<dwMaterialNum; ++i )
	{
		m_List_Material.InsertItem( i, str[pMaterials[i].bEffUse].c_str() );
		m_List_Material.SetItemText( i, 1, pMaterials[i].szTexture );
		m_List_Material.SetItemText( i, 2, pMaterials[i].szEffTex );
		m_List_Material.SetItemText( i, 3, _HLIB::cstring_floatg(pMaterials[i].d3dMaterial.Power).GetString() );
	}

	m_List_Material.SetRedraw( TRUE );

	if ( nSelect == -1 && m_List_Material.GetItemCount() > 0 )
		nSelect = 0;

	if ( nSelect != -1 )
	{
		m_List_Material.EnsureVisible( nSelect, TRUE );
		m_List_Material.SetItemState( nSelect, LVIS_SELECTED, LVIS_SELECTED );
	}

	//reset material edit
	SetWin_Enable( this, IDC_BUTTON_MATERIAL_TEXTURE, FALSE );
	SetWin_Enable( this, IDC_CHECK_MATERIAL_USE, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATERIAL_TEXTURE, FALSE );
	SetWin_Enable( this, IDC_BUTTON_MATERIAL_OK, FALSE );

	SetWin_Check( this, IDC_CHECK_MATERIAL_USE, FALSE );
	SetWin_Text( this, IDC_EDIT_MATERIAL_TEXTURE, "" );

	m_nMatEdit = -1;
}

void CPageEff_25_UserColor::OnBnClickedButtonApply()
{
	DataSave();
}

void CPageEff_25_UserColor::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		m_pPiece = NULL;
		m_pEff = NULL;

		if ( m_pSheetTab )
			m_pSheetTab->ShowMain();
	}
}

void CPageEff_25_UserColor::OnBnClickedButtonClose()
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

void CPageEff_25_UserColor::OnBnClickedButtonCucColor()
{
	float fColor_r = GetWin_Num_float( this, IDC_EDIT_CUC_COLOR_R );
	float fColor_g = GetWin_Num_float( this, IDC_EDIT_CUC_COLOR_G );
	float fColor_b = GetWin_Num_float( this, IDC_EDIT_CUC_COLOR_B );

	CSelColorDlg dlg( BYTE( fColor_r * 255.0f ), BYTE( fColor_g * 255.0f ), BYTE( fColor_b * 255.0f ) );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_float( this, IDC_EDIT_CUC_COLOR_R, float( GetRValue( crNewColor ) / 255.0f ) );
		SetWin_Num_float( this, IDC_EDIT_CUC_COLOR_G, float( GetGValue( crNewColor ) / 255.0f ) );
		SetWin_Num_float( this, IDC_EDIT_CUC_COLOR_B, float( GetBValue( crNewColor ) / 255.0f ) );
	}
}

void CPageEff_25_UserColor::OnNMDblclkListMaterial(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	int nSelect = m_List_Material.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	DWORD dwMaterialNum = m_pEff->GetMaterialsNum();
	DXMATERIAL_CHAR_EFF* pMaterials = m_pEff->GetMaterials();

	if ( nSelect >= (int)dwMaterialNum )	return;

	SetWin_Enable( this, IDC_BUTTON_MATERIAL_TEXTURE, TRUE );
	SetWin_Enable( this, IDC_CHECK_MATERIAL_USE, TRUE );
	SetWin_Enable( this, IDC_EDIT_MATERIAL_TEXTURE, TRUE );
	SetWin_Enable( this, IDC_BUTTON_MATERIAL_OK, TRUE );

	SetWin_Check( this, IDC_CHECK_MATERIAL_USE, pMaterials[nSelect].bEffUse );
	SetWin_Text( this, IDC_EDIT_MATERIAL_TEXTURE, pMaterials[nSelect].szEffTex );

	m_nMatEdit = nSelect;
}

void CPageEff_25_UserColor::OnBnClickedButtonMaterialTexture()
{
	CString szFilter = "Texture Image (*.bmp,*.dds,*.tga,*.jpg)|*.bmp;*.dds;*.tga;*.jpg|";
	CFileDialog dlg(TRUE,".",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = TextureManager::GetTexturePath(); 
	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, IDC_EDIT_MATERIAL_TEXTURE, dlg.GetFileName().GetString() );
}

void CPageEff_25_UserColor::OnBnClickedButtonMaterialOk()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	DWORD dwMaterialNum = m_pEff->GetMaterialsNum();
	DXMATERIAL_CHAR_EFF* pMaterials = m_pEff->GetMaterials();

	if( m_nMatEdit == -1 ) return;
	if( m_nMatEdit >= (int)dwMaterialNum )	return;

	std::string str = GetWin_Text( this, IDC_EDIT_MATERIAL_TEXTURE );
	StringCchCopy( pMaterials[m_nMatEdit].szEffTex, MAX_PATH, str.c_str() );
	pMaterials[m_nMatEdit].bEffUse = GetWin_Check( this, IDC_CHECK_MATERIAL_USE );
	MaterialShow();
}
