// PageEff_17_MultiTex.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkinPiece.h"
#include "EditorSkinPieceView.h"
#include "PageEff_17_MultiTex.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "SelColorDlg.h"
#include "Global.h"

// CPageEff_17_MultiTex dialog

IMPLEMENT_DYNAMIC(CPageEff_17_MultiTex, CPropertyPage)

CPageEff_17_MultiTex::CPageEff_17_MultiTex(LOGFONT logfont)
	: CPropertyPage(CPageEff_17_MultiTex::IDD)
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

CPageEff_17_MultiTex::~CPageEff_17_MultiTex()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEff_17_MultiTex::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_LIST_MATERIAL, m_List_Material );
}


BEGIN_MESSAGE_MAP(CPageEff_17_MultiTex, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_CMT_TEXTURE_1, OnBnClickedButtonCmtTexture1)
	ON_BN_CLICKED(IDC_BUTTON_CMT_COLOR_1, OnBnClickedButtonCmtColor1)
	ON_BN_CLICKED(IDC_BUTTON_CMT_TEXTURE_2, OnBnClickedButtonCmtTexture2)
	ON_BN_CLICKED(IDC_BUTTON_CMT_COLOR_2, OnBnClickedButtonCmtColor2)
	ON_BN_CLICKED(IDC_BUTTON_CMT_TEXMAP, OnBnClickedButtonCmtTexmap)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_MATERIAL, OnNMDblclkListMaterial)
	ON_BN_CLICKED(IDC_BUTTON_MATERIAL_OK, OnBnClickedButtonMaterialOk)
	ON_BN_CLICKED(IDC_CHECK_CMT_USE_1, OnBnClickedCheckCmtUse1)
	ON_BN_CLICKED(IDC_CHECK_CMT_USE_2, OnBnClickedCheckCmtUse2)
	ON_BN_CLICKED(IDC_CHECK_CMT_USE_ALLTEX, OnBnClickedCheckCmtUseAlltex)
	ON_BN_CLICKED(IDC_CHECK_CMT_USE_SELECTTEX, OnBnClickedCheckCmtUseSelecttex)
END_MESSAGE_MAP()


// CPageEff_17_MultiTex message handlers
BOOL CPageEff_17_MultiTex::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_List_Material.SetExtendedStyle ( m_List_Material.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List_Material.GetClientRect ( &rectCtrl );
	m_List_Material.InsertColumn( 0, "Used", LVCFMT_LEFT, ( 10* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List_Material.InsertColumn( 1, "Material", LVCFMT_LEFT, ( 90* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	SetWin_Combo_Init( this, IDC_COMBO_CMT_BLEND, GLOBAL_EFFTOOL::strEFFMARK_BLEND, TOOL_EFFMARK_BLEND_SIZE );

	ResetTool();

	return TRUE;
}

void CPageEff_17_MultiTex::DataSet( DxSkinPiece* pPiece, DxEffCharMultiTex* pEff, BOOL bADD )
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

void CPageEff_17_MultiTex::ResetTool()
{
	SetWin_Check( this, IDC_CHECK_CMT_USE_1, FALSE );
	SetWin_Check( this, IDC_CHECK_CMT_USE_2, FALSE );
	SetWin_Combo_Sel( this, IDC_COMBO_CMT_BLEND, 0 );

	SetWin_Text( this, IDC_EDIT_CMT_TEXTURE_1, "" );
	SetWin_Num_float( this, IDC_EDIT_CMT_COLOR_A_1, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CMT_COLOR_R_1, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CMT_COLOR_G_1, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CMT_COLOR_B_1, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CMT_TEX_X_1, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CMT_TEX_Y_1, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CMT_TEXUV_X_1, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CMT_TEXUV_Y_1, 1.0f );

	SetWin_Text( this, IDC_EDIT_CMT_TEXTURE_2, "" );
	SetWin_Num_float( this, IDC_EDIT_CMT_COLOR_A_2, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CMT_COLOR_R_2, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CMT_COLOR_G_2, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CMT_COLOR_B_2, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CMT_TEX_X_2, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CMT_TEX_Y_2, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CMT_TEXUV_X_2, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CMT_TEXUV_Y_2, 1.0f );

	SetWin_Check( this, IDC_CHECK_CMT_USE_ALLTEX, FALSE );
	SetWin_Check( this, IDC_CHECK_CMT_USE_SELECTTEX, FALSE );

	SetWin_Check( this, IDC_CHECK_CMT_USE_TEXMAP, FALSE );
	SetWin_Text( this, IDC_EDIT_CMT_TEXMAP, "" );

	//material
	m_List_Material.DeleteAllItems();
	SetWin_Enable( this, IDC_CHECK_MATERIAL_USE, FALSE );
	SetWin_Enable( this, IDC_BUTTON_MATERIAL_OK, FALSE );

	SetWin_Check( this, IDC_CHECK_MATERIAL_USE, FALSE );
	m_nMatEdit = -1;
}

void CPageEff_17_MultiTex::DataShow()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	EFFCHAR_PROPERTY_MULTITEX* pProp = (EFFCHAR_PROPERTY_MULTITEX*)m_pEff->GetProperty();
	SetWin_Check( this, IDC_CHECK_CMT_USE_1, ( pProp->m_nTexNum == 1 ) );
	SetWin_Check( this, IDC_CHECK_CMT_USE_2, ( pProp->m_nTexNum == 2 ) );
	SetWin_Combo_Sel( this, IDC_COMBO_CMT_BLEND, pProp->m_nBlend );

	SetWin_Text( this, IDC_EDIT_CMT_TEXTURE_1, pProp->m_szTex1 );
	SetWin_Num_float( this, IDC_EDIT_CMT_COLOR_A_1, pProp->m_cDiffuse1.a );
	SetWin_Num_float( this, IDC_EDIT_CMT_COLOR_R_1, pProp->m_cDiffuse1.r );
	SetWin_Num_float( this, IDC_EDIT_CMT_COLOR_G_1, pProp->m_cDiffuse1.g );
	SetWin_Num_float( this, IDC_EDIT_CMT_COLOR_B_1, pProp->m_cDiffuse1.b );
	SetWin_Num_float( this, IDC_EDIT_CMT_TEX_X_1, pProp->m_vTex01.x );
	SetWin_Num_float( this, IDC_EDIT_CMT_TEX_Y_1, pProp->m_vTex01.y );
	SetWin_Num_float( this, IDC_EDIT_CMT_TEXUV_X_1, pProp->m_vTexUV01.x );
	SetWin_Num_float( this, IDC_EDIT_CMT_TEXUV_Y_1, pProp->m_vTexUV01.y );

	SetWin_Text( this, IDC_EDIT_CMT_TEXTURE_2, pProp->m_szTex2 );
	SetWin_Num_float( this, IDC_EDIT_CMT_COLOR_A_2, pProp->m_cDiffuse2.a );
	SetWin_Num_float( this, IDC_EDIT_CMT_COLOR_R_2, pProp->m_cDiffuse2.r );
	SetWin_Num_float( this, IDC_EDIT_CMT_COLOR_G_2, pProp->m_cDiffuse2.g );
	SetWin_Num_float( this, IDC_EDIT_CMT_COLOR_B_2, pProp->m_cDiffuse2.b );
	SetWin_Num_float( this, IDC_EDIT_CMT_TEX_X_2, pProp->m_vTex02.x );
	SetWin_Num_float( this, IDC_EDIT_CMT_TEX_Y_2, pProp->m_vTex02.y );
	SetWin_Num_float( this, IDC_EDIT_CMT_TEXUV_X_2, pProp->m_vTexUV02.x );
	SetWin_Num_float( this, IDC_EDIT_CMT_TEXUV_Y_2, pProp->m_vTexUV02.y );

	SetWin_Check( this, IDC_CHECK_CMT_USE_ALLTEX, pProp->m_dwFlag&USE_ALL_TEX );
	SetWin_Check( this, IDC_CHECK_CMT_USE_SELECTTEX, pProp->m_dwFlag&USE_SELECT_TEX );

	SetWin_Check( this, IDC_CHECK_CMT_USE_TEXMAP, pProp->m_dwFlag&USETEXMAP );
	SetWin_Text( this, IDC_EDIT_CMT_TEXMAP, pProp->m_szTexture );
}

BOOL CPageEff_17_MultiTex::DataSave()
{
	if ( !m_pPiece )	return FALSE;
	if ( !m_pEff )		return FALSE;

	EFFCHAR_PROPERTY_MULTITEX sProperty;

	BOOL bUse1 = GetWin_Check( this, IDC_CHECK_CMT_USE_1 );
	BOOL bUse2 = GetWin_Check( this, IDC_CHECK_CMT_USE_2 );
	std::string strTex1 = GetWin_Text( this, IDC_EDIT_CMT_TEXTURE_1 );
	std::string strTex2 = GetWin_Text( this, IDC_EDIT_CMT_TEXTURE_2 );
	std::string strTexture = GetWin_Text( this, IDC_EDIT_CMT_TEXMAP );

	sProperty.m_nTexNum = (bUse1)? 1: (bUse2)? 2:0;
	sProperty.m_nBlend = GetWin_Combo_Sel( this, IDC_COMBO_CMT_BLEND );

	StringCchCopy( sProperty.m_szTex1, MAX_PATH, strTex1.c_str() );
	sProperty.m_cDiffuse1.a = GetWin_Num_float( this, IDC_EDIT_CMT_COLOR_A_1 );
	sProperty.m_cDiffuse1.r = GetWin_Num_float( this, IDC_EDIT_CMT_COLOR_R_1 );
	sProperty.m_cDiffuse1.g = GetWin_Num_float( this, IDC_EDIT_CMT_COLOR_G_1 );
	sProperty.m_cDiffuse1.b = GetWin_Num_float( this, IDC_EDIT_CMT_COLOR_B_1 );
	sProperty.m_vTex01.x = GetWin_Num_float( this, IDC_EDIT_CMT_TEX_X_1 );
	sProperty.m_vTex01.y = GetWin_Num_float( this, IDC_EDIT_CMT_TEX_Y_1 );
	sProperty.m_vTexUV01.x = GetWin_Num_float( this, IDC_EDIT_CMT_TEXUV_X_1 );
	sProperty.m_vTexUV01.y = GetWin_Num_float( this, IDC_EDIT_CMT_TEXUV_Y_1 );

	StringCchCopy( sProperty.m_szTex2, MAX_PATH, strTex2.c_str() );
	sProperty.m_cDiffuse2.a = GetWin_Num_float( this, IDC_EDIT_CMT_COLOR_A_2 );
	sProperty.m_cDiffuse2.r = GetWin_Num_float( this, IDC_EDIT_CMT_COLOR_R_2 );
	sProperty.m_cDiffuse2.g = GetWin_Num_float( this, IDC_EDIT_CMT_COLOR_G_2 );
	sProperty.m_cDiffuse2.b = GetWin_Num_float( this, IDC_EDIT_CMT_COLOR_B_2 );
	sProperty.m_vTex02.x = GetWin_Num_float( this, IDC_EDIT_CMT_TEX_X_2 );
	sProperty.m_vTex02.y = GetWin_Num_float( this, IDC_EDIT_CMT_TEX_Y_2 );
	sProperty.m_vTexUV02.x = GetWin_Num_float( this, IDC_EDIT_CMT_TEXUV_X_2 );
	sProperty.m_vTexUV02.y = GetWin_Num_float( this, IDC_EDIT_CMT_TEXUV_Y_2 );

	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CMT_USE_ALLTEX ), sProperty.m_dwFlag, USE_ALL_TEX);
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CMT_USE_SELECTTEX ), sProperty.m_dwFlag, USE_SELECT_TEX );

	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CMT_USE_TEXMAP ), sProperty.m_dwFlag, USETEXMAP );
	StringCchCopy( sProperty.m_szTexture, MAX_PATH, strTexture.c_str() );

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

void CPageEff_17_MultiTex::MaterialShow()
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
	SetWin_Enable( this, IDC_CHECK_MATERIAL_USE, FALSE );
	SetWin_Enable( this, IDC_BUTTON_MATERIAL_OK, FALSE );

	SetWin_Check( this, IDC_CHECK_MATERIAL_USE, FALSE );
	m_nMatEdit = -1;
}

void CPageEff_17_MultiTex::OnBnClickedButtonApply()
{
	DataSave();
}

void CPageEff_17_MultiTex::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		m_pPiece = NULL;
		m_pEff = NULL;

		if ( m_pSheetTab )
			m_pSheetTab->ShowMain();
	}
}

void CPageEff_17_MultiTex::OnBnClickedButtonClose()
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

void CPageEff_17_MultiTex::OnBnClickedButtonCmtTexture1()
{
	CString szFilter = "Texture Image (*.bmp,*.dds,*.tga,*.jpg)|*.bmp;*.dds;*.tga;*.jpg|";
	CFileDialog dlg(TRUE,".",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = TextureManager::GetTexturePath(); 
	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, IDC_EDIT_CMT_TEXTURE_1, dlg.GetFileName().GetString() );
}

void CPageEff_17_MultiTex::OnBnClickedButtonCmtColor1()
{
	float fColor_r = GetWin_Num_float( this, IDC_EDIT_CMT_COLOR_R_1 );
	float fColor_g = GetWin_Num_float( this, IDC_EDIT_CMT_COLOR_G_1 );
	float fColor_b = GetWin_Num_float( this, IDC_EDIT_CMT_COLOR_B_1 );

	CSelColorDlg dlg( BYTE( fColor_r * 255.0f ), BYTE( fColor_g * 255.0f ), BYTE( fColor_b * 255.0f ) );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_float( this, IDC_EDIT_CMT_COLOR_R_1, float( GetRValue( crNewColor ) / 255.0f ) );
		SetWin_Num_float( this, IDC_EDIT_CMT_COLOR_G_1, float( GetGValue( crNewColor ) / 255.0f ) );
		SetWin_Num_float( this, IDC_EDIT_CMT_COLOR_B_1, float( GetBValue( crNewColor ) / 255.0f ) );
	}
}

void CPageEff_17_MultiTex::OnBnClickedButtonCmtTexture2()
{
	CString szFilter = "Texture Image (*.bmp,*.dds,*.tga,*.jpg)|*.bmp;*.dds;*.tga;*.jpg|";
	CFileDialog dlg(TRUE,".",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = TextureManager::GetTexturePath(); 
	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, IDC_EDIT_CMT_TEXTURE_2, dlg.GetFileName().GetString() );
}

void CPageEff_17_MultiTex::OnBnClickedButtonCmtColor2()
{
	float fColor_r = GetWin_Num_float( this, IDC_EDIT_CMT_COLOR_R_2 );
	float fColor_g = GetWin_Num_float( this, IDC_EDIT_CMT_COLOR_G_2 );
	float fColor_b = GetWin_Num_float( this, IDC_EDIT_CMT_COLOR_B_2 );

	CSelColorDlg dlg( BYTE( fColor_r * 255.0f ), BYTE( fColor_g * 255.0f ), BYTE( fColor_b * 255.0f ) );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_float( this, IDC_EDIT_CMT_COLOR_R_2, float( GetRValue( crNewColor ) / 255.0f ) );
		SetWin_Num_float( this, IDC_EDIT_CMT_COLOR_G_2, float( GetGValue( crNewColor ) / 255.0f ) );
		SetWin_Num_float( this, IDC_EDIT_CMT_COLOR_B_2, float( GetBValue( crNewColor ) / 255.0f ) );
	}
}

void CPageEff_17_MultiTex::OnBnClickedButtonCmtTexmap()
{
	CString szFilter = "Texture Image (*.bmp,*.dds,*.tga,*.jpg)|*.bmp;*.dds;*.tga;*.jpg|";
	CFileDialog dlg(TRUE,".",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = TextureManager::GetTexturePath(); 
	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, IDC_EDIT_CMT_TEXMAP, dlg.GetFileName().GetString() );
}

void CPageEff_17_MultiTex::OnNMDblclkListMaterial(NMHDR *pNMHDR, LRESULT *pResult)
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

	SetWin_Enable( this, IDC_CHECK_MATERIAL_USE, TRUE );
	SetWin_Enable( this, IDC_BUTTON_MATERIAL_OK, TRUE );

	SetWin_Check( this, IDC_CHECK_MATERIAL_USE, pMaterials[nSelect].bEffUse );

	m_nMatEdit = nSelect;
}

void CPageEff_17_MultiTex::OnBnClickedButtonMaterialOk()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	DWORD dwMaterialNum = m_pEff->GetMaterialsNum();
	DXMATERIAL_CHAR_EFF* pMaterials = m_pEff->GetMaterials();

	if( m_nMatEdit == -1 ) return;
	if( m_nMatEdit >= (int)dwMaterialNum )	return;

	pMaterials[m_nMatEdit].bEffUse = GetWin_Check( this, IDC_CHECK_MATERIAL_USE );
	MaterialShow();
}

void CPageEff_17_MultiTex::OnBnClickedCheckCmtUse1()
{
	SetWin_Check( this, IDC_CHECK_CMT_USE_1, TRUE );
	SetWin_Check( this, IDC_CHECK_CMT_USE_2, FALSE );
}

void CPageEff_17_MultiTex::OnBnClickedCheckCmtUse2()
{
	SetWin_Check( this, IDC_CHECK_CMT_USE_1, FALSE );
	SetWin_Check( this, IDC_CHECK_CMT_USE_2, TRUE );
}

void CPageEff_17_MultiTex::OnBnClickedCheckCmtUseAlltex()
{
	SetWin_Check( this, IDC_CHECK_CMT_USE_ALLTEX, TRUE );
	SetWin_Check( this, IDC_CHECK_CMT_USE_SELECTTEX, FALSE );
}

void CPageEff_17_MultiTex::OnBnClickedCheckCmtUseSelecttex()
{
	SetWin_Check( this, IDC_CHECK_CMT_USE_ALLTEX, FALSE );
	SetWin_Check( this, IDC_CHECK_CMT_USE_SELECTTEX, TRUE );
}
