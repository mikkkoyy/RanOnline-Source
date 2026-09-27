// PageEff_19_Specular2.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkinPiece.h"
#include "EditorSkinPieceView.h"
#include "PageEff_19_Specular2.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "Global.h"
#include "SelColorDlg.h"
#include "HLibDataConvert.h"

// CPageEff_19_Specular2 dialog

IMPLEMENT_DYNAMIC(CPageEff_19_Specular2, CPropertyPage)

CPageEff_19_Specular2::CPageEff_19_Specular2(LOGFONT logfont)
	: CPropertyPage(CPageEff_19_Specular2::IDD)
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

CPageEff_19_Specular2::~CPageEff_19_Specular2()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEff_19_Specular2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_LIST_MATERIAL, m_List_Material );
}


BEGIN_MESSAGE_MAP(CPageEff_19_Specular2, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_CS2_COLOR, OnBnClickedButtonCs2Color)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_MATERIAL, OnNMDblclkListMaterial)
	ON_BN_CLICKED(IDC_BUTTON_MATERIAL_TEXTURE, OnBnClickedButtonMaterialTexture)
	ON_BN_CLICKED(IDC_BUTTON_MATERIAL_OK, OnBnClickedButtonMaterialOk)
	ON_BN_CLICKED(IDC_BUTTON_CS2_TEXTURE, OnBnClickedButtonCs2Texture)
END_MESSAGE_MAP()


// CPageEff_19_Specular2 message handlers
BOOL CPageEff_19_Specular2::OnInitDialog()
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

	SetWin_Combo_Init( this, IDC_COMBO_CS2_COLOROP, GLOBAL_EFFTOOL::strEFFLEVEL_MODULATE, TOOL_EFFLEVEL_MODULATE_SIZE );

	ResetTool();

	return TRUE;
}

void CPageEff_19_Specular2::DataSet( DxSkinPiece* pPiece, DxEffCharSpecular2* pEff, BOOL bADD )
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

void CPageEff_19_Specular2::ResetTool()
{
	SetWin_Combo_Sel( this, IDC_COMBO_CS2_COLOROP, 0 );
	SetWin_Num_float( this, IDC_EDIT_CS2_COLOR_A, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CS2_COLOR_R, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CS2_COLOR_G, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CS2_COLOR_B, 1.0f );
	SetWin_Check( this, IDC_CHECK_CS2_CUBEMAP, FALSE );

	SetWin_Check( this, IDC_CHECK_CS2_USE_CUBETEXTURE, FALSE );
	SetWin_Text( this, IDC_EDIT_CS2_CUBETEXTURE, "" );

	//material
	m_List_Material.DeleteAllItems();
	SetWin_Enable( this, IDC_BUTTON_MATERIAL_TEXTURE, FALSE );
	SetWin_Enable( this, IDC_CHECK_MATERIAL_USE, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATERIAL_TEXTURE, FALSE );
	SetWin_Enable( this, IDC_EDIT_MATERIAL_POWER, FALSE );
	SetWin_Enable( this, IDC_BUTTON_MATERIAL_OK, FALSE );

	SetWin_Check( this, IDC_CHECK_MATERIAL_USE, FALSE );
	SetWin_Text( this, IDC_EDIT_MATERIAL_TEXTURE, "" );
	SetWin_Num_float( this, IDC_EDIT_MATERIAL_POWER, 1.0f );

	m_nMatEdit = -1;
}

void CPageEff_19_Specular2::DataShow()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	EFFCHAR_PROPERTY_SPECULAR2* pProp = (EFFCHAR_PROPERTY_SPECULAR2*)m_pEff->GetProperty();
	SetWin_Combo_Sel( this, IDC_COMBO_CS2_COLOROP, pProp->m_dwColorOP - 4 );
	SetWin_Num_float( this, IDC_EDIT_CS2_COLOR_A, pProp->m_cSpecular.a );
	SetWin_Num_float( this, IDC_EDIT_CS2_COLOR_R, pProp->m_cSpecular.r );
	SetWin_Num_float( this, IDC_EDIT_CS2_COLOR_G, pProp->m_cSpecular.g );
	SetWin_Num_float( this, IDC_EDIT_CS2_COLOR_B, pProp->m_cSpecular.b );
	SetWin_Check( this, IDC_CHECK_CS2_CUBEMAP, pProp->m_dwFlag&EFFCHARSPEC2_CUBE );

	SetWin_Check( this, IDC_CHECK_CS2_USE_CUBETEXTURE, pProp->m_bCubeTexture );
	SetWin_Text( this, IDC_EDIT_CS2_CUBETEXTURE, pProp->m_szCubeTexture );
}

BOOL CPageEff_19_Specular2::DataSave()
{
	if ( !m_pPiece )	return FALSE;
	if ( !m_pEff )		return FALSE;

	EFFCHAR_PROPERTY_SPECULAR2 sProperty;
	sProperty.m_dwColorOP = GetWin_Combo_Sel( this, IDC_COMBO_CS2_COLOROP) +4 ;
	sProperty.m_cSpecular.a = GetWin_Num_float( this, IDC_EDIT_CS2_COLOR_A );
	sProperty.m_cSpecular.r = GetWin_Num_float( this, IDC_EDIT_CS2_COLOR_R );
	sProperty.m_cSpecular.g = GetWin_Num_float( this, IDC_EDIT_CS2_COLOR_G );
	sProperty.m_cSpecular.b = GetWin_Num_float( this, IDC_EDIT_CS2_COLOR_B );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CS2_CUBEMAP), sProperty.m_dwFlag, EFFCHARSPEC2_CUBE );

	sProperty.m_bCubeTexture = GetWin_Check( this, IDC_CHECK_CS2_USE_CUBETEXTURE );
	std::string strCubeTexture = GetWin_Text( this, IDC_EDIT_CS2_CUBETEXTURE );
	StringCchCopy( sProperty.m_szCubeTexture, MAX_PATH, strCubeTexture.c_str() );

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

void CPageEff_19_Specular2::MaterialShow()
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
	SetWin_Enable( this, IDC_EDIT_MATERIAL_POWER, FALSE );
	SetWin_Enable( this, IDC_BUTTON_MATERIAL_OK, FALSE );

	SetWin_Check( this, IDC_CHECK_MATERIAL_USE, FALSE );
	SetWin_Text( this, IDC_EDIT_MATERIAL_TEXTURE, "" );
	SetWin_Num_float( this, IDC_EDIT_MATERIAL_POWER, 1.0f );

	m_nMatEdit = -1;
}

void CPageEff_19_Specular2::OnBnClickedButtonApply()
{
	DataSave();
}

void CPageEff_19_Specular2::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		m_pPiece = NULL;
		m_pEff = NULL;

		if ( m_pSheetTab )
			m_pSheetTab->ShowMain();
	}
}

void CPageEff_19_Specular2::OnBnClickedButtonClose()
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

void CPageEff_19_Specular2::OnBnClickedButtonCs2Color()
{
	float fColor_r = GetWin_Num_float( this, IDC_EDIT_CS2_COLOR_R );
	float fColor_g = GetWin_Num_float( this, IDC_EDIT_CS2_COLOR_G );
	float fColor_b = GetWin_Num_float( this, IDC_EDIT_CS2_COLOR_B );

	CSelColorDlg dlg( BYTE( fColor_r * 255.0f ), BYTE( fColor_g * 255.0f ), BYTE( fColor_b * 255.0f ) );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_float( this, IDC_EDIT_CS2_COLOR_R, float( GetRValue( crNewColor ) / 255.0f ) );
		SetWin_Num_float( this, IDC_EDIT_CS2_COLOR_G, float( GetGValue( crNewColor ) / 255.0f ) );
		SetWin_Num_float( this, IDC_EDIT_CS2_COLOR_B, float( GetBValue( crNewColor ) / 255.0f ) );
	}
}

void CPageEff_19_Specular2::OnNMDblclkListMaterial(NMHDR *pNMHDR, LRESULT *pResult)
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
	SetWin_Enable( this, IDC_EDIT_MATERIAL_POWER, TRUE );
	SetWin_Enable( this, IDC_BUTTON_MATERIAL_OK, TRUE );

	SetWin_Check( this, IDC_CHECK_MATERIAL_USE, pMaterials[nSelect].bEffUse );
	SetWin_Text( this, IDC_EDIT_MATERIAL_TEXTURE, pMaterials[nSelect].szEffTex );
	SetWin_Num_float( this, IDC_EDIT_MATERIAL_POWER, pMaterials[nSelect].d3dMaterial.Power );

	m_nMatEdit = nSelect;
}

void CPageEff_19_Specular2::OnBnClickedButtonMaterialTexture()
{
	CString szFilter = "Texture Image (*.bmp,*.dds,*.tga,*.jpg)|*.bmp;*.dds;*.tga;*.jpg|";
	CFileDialog dlg(TRUE,".",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = TextureManager::GetTexturePath(); 
	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, IDC_EDIT_MATERIAL_TEXTURE, dlg.GetFileName().GetString() );
}

void CPageEff_19_Specular2::OnBnClickedButtonMaterialOk()
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
	pMaterials[m_nMatEdit].d3dMaterial.Power = GetWin_Num_float( this, IDC_EDIT_MATERIAL_POWER );
	MaterialShow();
}

void CPageEff_19_Specular2::OnBnClickedButtonCs2Texture()
{
	CString szFilter = "Texture Image (*.bmp,*.dds,*.tga,*.jpg)|*.bmp;*.dds;*.tga;*.jpg|";
	CFileDialog dlg(TRUE,".",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = TextureManager::GetTexturePath(); 
	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, IDC_EDIT_CS2_CUBETEXTURE, dlg.GetFileName().GetString() );
}
