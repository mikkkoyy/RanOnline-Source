// PageEff_08_Level.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkinPiece.h"
#include "EditorSkinPieceView.h"
#include "PageEff_08_Level.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "Global.h"
#include "SelColorDlg.h"
#include "HLibDataConvert.h"

// CPageEff_08_Level dialog

IMPLEMENT_DYNAMIC(CPageEff_08_Level, CPropertyPage)

CPageEff_08_Level::CPageEff_08_Level(LOGFONT logfont)
	: CPropertyPage(CPageEff_08_Level::IDD)
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

CPageEff_08_Level::~CPageEff_08_Level()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEff_08_Level::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_LIST_MATERIAL, m_List_Material );
}


BEGIN_MESSAGE_MAP(CPageEff_08_Level, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_CL_COLOR, OnBnClickedButtonClColor)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_MATERIAL, OnNMDblclkListMaterial)
	ON_BN_CLICKED(IDC_BUTTON_MATERIAL_TEXTURE, OnBnClickedButtonMaterialTexture)
	ON_BN_CLICKED(IDC_BUTTON_MATERIAL_OK, OnBnClickedButtonMaterialOk)
END_MESSAGE_MAP()


// CPageEff_08_Level message handlers
BOOL CPageEff_08_Level::OnInitDialog()
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

	SetWin_Combo_Init( this, IDC_COMBO_CL_COLOROP, GLOBAL_EFFTOOL::strEFFLEVEL_MODULATE, TOOL_EFFLEVEL_MODULATE_SIZE );

	ResetTool();

	return TRUE;
}

void CPageEff_08_Level::DataSet( DxSkinPiece* pPiece, DxEffCharLevel* pEff, BOOL bADD )
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

void CPageEff_08_Level::ResetTool()
{
	SetWin_Combo_Sel( this, IDC_COMBO_CL_COLOROP, 0 );
	SetWin_Num_float( this, IDC_EDIT_CL_COLOR_A, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CL_COLOR_R, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CL_COLOR_G, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CL_COLOR_B, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CL_POWER, 1.0f );

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

void CPageEff_08_Level::DataShow()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	EFFCHAR_PROPERTY_LEVEL* pProp = (EFFCHAR_PROPERTY_LEVEL*)m_pEff->GetProperty();
	SetWin_Combo_Sel( this, IDC_COMBO_CL_COLOROP, pProp->m_dwColorOP - 4 );
	SetWin_Num_float( this, IDC_EDIT_CL_COLOR_A, pProp->m_cSpecular.a );
	SetWin_Num_float( this, IDC_EDIT_CL_COLOR_R, pProp->m_cSpecular.r );
	SetWin_Num_float( this, IDC_EDIT_CL_COLOR_G, pProp->m_cSpecular.g );
	SetWin_Num_float( this, IDC_EDIT_CL_COLOR_B, pProp->m_cSpecular.b );
	SetWin_Num_float( this, IDC_EDIT_CL_POWER, pProp->m_fPower );
}

BOOL CPageEff_08_Level::DataSave()
{
	if ( !m_pPiece )	return FALSE;
	if ( !m_pEff )		return FALSE;

	EFFCHAR_PROPERTY_LEVEL sProperty;
	sProperty.m_dwColorOP = GetWin_Combo_Sel( this, IDC_COMBO_CL_COLOROP ) +4 ;
	sProperty.m_cSpecular.a = GetWin_Num_float( this, IDC_EDIT_CL_COLOR_A );
	sProperty.m_cSpecular.r = GetWin_Num_float( this, IDC_EDIT_CL_COLOR_R );
	sProperty.m_cSpecular.g = GetWin_Num_float( this, IDC_EDIT_CL_COLOR_G );
	sProperty.m_cSpecular.b = GetWin_Num_float( this, IDC_EDIT_CL_COLOR_B );
	sProperty.m_fPower = GetWin_Num_float( this, IDC_EDIT_CL_POWER );


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

void CPageEff_08_Level::MaterialShow()
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

void CPageEff_08_Level::OnBnClickedButtonApply()
{
	DataSave();
}

void CPageEff_08_Level::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		m_pPiece = NULL;
		m_pEff = NULL;

		if ( m_pSheetTab )
			m_pSheetTab->ShowMain();
	}
}

void CPageEff_08_Level::OnBnClickedButtonClose()
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

void CPageEff_08_Level::OnBnClickedButtonClColor()
{
	float fColor_r = GetWin_Num_float( this, IDC_EDIT_CL_COLOR_R );
	float fColor_g = GetWin_Num_float( this, IDC_EDIT_CL_COLOR_G );
	float fColor_b = GetWin_Num_float( this, IDC_EDIT_CL_COLOR_B );

	CSelColorDlg dlg( BYTE( fColor_r * 255.0f ), BYTE( fColor_g * 255.0f ), BYTE( fColor_b * 255.0f ) );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_float( this, IDC_EDIT_CL_COLOR_R, float( GetRValue( crNewColor ) / 255.0f ) );
		SetWin_Num_float( this, IDC_EDIT_CL_COLOR_G, float( GetGValue( crNewColor ) / 255.0f ) );
		SetWin_Num_float( this, IDC_EDIT_CL_COLOR_B, float( GetBValue( crNewColor ) / 255.0f ) );
	}
}

void CPageEff_08_Level::OnNMDblclkListMaterial(NMHDR *pNMHDR, LRESULT *pResult)
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

void CPageEff_08_Level::OnBnClickedButtonMaterialTexture()
{
	CString szFilter = "Texture Image (*.bmp,*.dds,*.tga,*.jpg)|*.bmp;*.dds;*.tga;*.jpg|";
	CFileDialog dlg(TRUE,".",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = TextureManager::GetTexturePath(); 
	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, IDC_EDIT_MATERIAL_TEXTURE, dlg.GetFileName().GetString() );
}

void CPageEff_08_Level::OnBnClickedButtonMaterialOk()
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
