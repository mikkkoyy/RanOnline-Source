// PageEff_11_Neon.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkinPiece.h"
#include "EditorSkinPieceView.h"
#include "PageEff_11_Neon.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "SelColorDlg.h"

// CPageEff_11_Neon dialog

IMPLEMENT_DYNAMIC(CPageEff_11_Neon, CPropertyPage)

CPageEff_11_Neon::CPageEff_11_Neon(LOGFONT logfont)
	: CPropertyPage(CPageEff_11_Neon::IDD)
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

CPageEff_11_Neon::~CPageEff_11_Neon()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEff_11_Neon::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_LIST_MATERIAL, m_List_Material );
}


BEGIN_MESSAGE_MAP(CPageEff_11_Neon, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_CHECK_CN_USE_GENERAL, OnBnClickedCheckCnUseGeneral)
	ON_BN_CLICKED(IDC_CHECK_CN_USE_BURN, OnBnClickedCheckCnUseBurn)
	ON_BN_CLICKED(IDC_CHECK_CN_USE_ALLTEX, OnBnClickedCheckCnUseAlltex)
	ON_BN_CLICKED(IDC_CHECK_CN_USE_SELECTTEX, OnBnClickedCheckCnUseSelecttex)
	ON_BN_CLICKED(IDC_CHECK_CN_USE_GLOW_SRC, OnBnClickedCheckCnUseGlowSrc)
	ON_BN_CLICKED(IDC_CHECK_CN_USE_GLOW_USR, OnBnClickedCheckCnUseGlowUsr)
	ON_BN_CLICKED(IDC_BUTTON_CN_TEXTURE, OnBnClickedButtonCnTexture)
	ON_BN_CLICKED(IDC_BUTTON_CN_COLOR, OnBnClickedButtonCnColor)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_MATERIAL, OnNMDblclkListMaterial)
	ON_BN_CLICKED(IDC_BUTTON_MATERIAL_OK, OnBnClickedButtonMaterialOk)
END_MESSAGE_MAP()


// CPageEff_11_Neon message handlers
BOOL CPageEff_11_Neon::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_List_Material.SetExtendedStyle ( m_List_Material.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List_Material.GetClientRect ( &rectCtrl );
	m_List_Material.InsertColumn( 0, "Used", LVCFMT_LEFT, ( 10* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List_Material.InsertColumn( 1, "Material", LVCFMT_LEFT, ( 90* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	ResetTool();

	return TRUE;
}

void CPageEff_11_Neon::DataSet( DxSkinPiece* pPiece, DxEffCharNeon* pEff, BOOL bADD )
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

void CPageEff_11_Neon::ResetTool()
{
	SetWin_Check( this, IDC_CHECK_CN_USE_GENERAL, FALSE );
	SetWin_Check( this, IDC_CHECK_CN_USE_BURN, FALSE );
	SetWin_Check( this, IDC_CHECK_CN_USE_ALLTEX, FALSE );
	SetWin_Check( this, IDC_CHECK_CN_USE_SELECTTEX, FALSE );
	SetWin_Check( this, IDC_CHECK_CN_USE_GLOW_SRC, FALSE );
	SetWin_Check( this, IDC_CHECK_CN_USE_GLOW_USR, FALSE );

	SetWin_Text( this, IDC_EDIT_CN_TEXTURE, "" );

	SetWin_Num_int( this, IDC_EDIT_CN_COLOR_A, 1 );
	SetWin_Num_int( this, IDC_EDIT_CN_COLOR_R, 1 );
	SetWin_Num_int( this, IDC_EDIT_CN_COLOR_G, 1 );
	SetWin_Num_int( this, IDC_EDIT_CN_COLOR_B, 1 );

	//material
	m_List_Material.DeleteAllItems();
	SetWin_Enable( this, IDC_CHECK_MATERIAL_USE, FALSE );
	SetWin_Enable( this, IDC_BUTTON_MATERIAL_OK, FALSE );

	SetWin_Check( this, IDC_CHECK_MATERIAL_USE, FALSE );
	m_nMatEdit = -1;
}

void CPageEff_11_Neon::DataShow()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	EFFCHAR_PROPERTY_NEON* pProp = (EFFCHAR_PROPERTY_NEON*)m_pEff->GetProperty();

	SetWin_Check( this, IDC_CHECK_CN_USE_GENERAL, pProp->m_dwFlag&USE_GENERAL );
	SetWin_Check( this, IDC_CHECK_CN_USE_BURN, pProp->m_dwFlag&USE_BURN );
	SetWin_Check( this, IDC_CHECK_CN_USE_ALLTEX, pProp->m_dwFlag&USE_ALL_TEX );
	SetWin_Check( this, IDC_CHECK_CN_USE_SELECTTEX, pProp->m_dwFlag&USE_SELECT_TEX );
	SetWin_Check( this, IDC_CHECK_CN_USE_GLOW_SRC, pProp->m_dwFlag&USE_GLOW_SRCTEX );
	SetWin_Check( this, IDC_CHECK_CN_USE_GLOW_USR, pProp->m_dwFlag&USE_GLOW_USER );

	SetWin_Num_int( this, IDC_EDIT_CN_COLOR_R, RGBA_GETRED( pProp->m_cColor ) );
	SetWin_Num_int( this, IDC_EDIT_CN_COLOR_G, RGBA_GETGREEN( pProp->m_cColor ) );
	SetWin_Num_int( this, IDC_EDIT_CN_COLOR_B, RGBA_GETBLUE( pProp->m_cColor ) );
	SetWin_Num_int( this, IDC_EDIT_CN_COLOR_A, RGBA_GETALPHA( pProp->m_cColor ) );

	SetWin_Text( this, IDC_EDIT_CN_TEXTURE, pProp->m_szTexture );
}

BOOL CPageEff_11_Neon::DataSave()
{
	if ( !m_pPiece )	return FALSE;
	if ( !m_pEff )		return FALSE;

	EFFCHAR_PROPERTY_NEON sProperty;
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CN_USE_GENERAL ), sProperty.m_dwFlag, USE_GENERAL );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CN_USE_BURN ), sProperty.m_dwFlag, USE_BURN );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CN_USE_ALLTEX ), sProperty.m_dwFlag, USE_ALL_TEX );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CN_USE_SELECTTEX ), sProperty.m_dwFlag, USE_SELECT_TEX );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CN_USE_GLOW_SRC ), sProperty.m_dwFlag, USE_GLOW_SRCTEX );
	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CN_USE_GLOW_USR ), sProperty.m_dwFlag, USE_GLOW_USER );

	sProperty.m_cColor = RGBA_MAKE(
		GetWin_Num_int( this, IDC_EDIT_CN_COLOR_R ), 
		GetWin_Num_int( this, IDC_EDIT_CN_COLOR_G ), 
		GetWin_Num_int( this, IDC_EDIT_CN_COLOR_B ),
		GetWin_Num_int( this, IDC_EDIT_CN_COLOR_A ));

	std::string str = GetWin_Text( this, IDC_EDIT_CN_TEXTURE );
	StringCchCopy( sProperty.m_szTexture , MAX_PATH, str.c_str() );


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

void CPageEff_11_Neon::MaterialShow()
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

void CPageEff_11_Neon::OnBnClickedButtonApply()
{
	DataSave();
}

void CPageEff_11_Neon::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		m_pPiece = NULL;
		m_pEff = NULL;

		if ( m_pSheetTab )
			m_pSheetTab->ShowMain();
	}
}

void CPageEff_11_Neon::OnBnClickedButtonClose()
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

void CPageEff_11_Neon::OnBnClickedCheckCnUseGeneral()
{
	SetWin_Check( this, IDC_CHECK_CN_USE_GENERAL, TRUE );
	SetWin_Check( this, IDC_CHECK_CN_USE_BURN, FALSE );
}

void CPageEff_11_Neon::OnBnClickedCheckCnUseBurn()
{
	SetWin_Check( this, IDC_CHECK_CN_USE_GENERAL, FALSE );
	SetWin_Check( this, IDC_CHECK_CN_USE_BURN, TRUE );
}

void CPageEff_11_Neon::OnBnClickedCheckCnUseAlltex()
{
	SetWin_Check( this, IDC_CHECK_CN_USE_ALLTEX, TRUE );
	SetWin_Check( this, IDC_CHECK_CN_USE_SELECTTEX, FALSE );
}

void CPageEff_11_Neon::OnBnClickedCheckCnUseSelecttex()
{
	SetWin_Check( this, IDC_CHECK_CN_USE_ALLTEX, FALSE );
	SetWin_Check( this, IDC_CHECK_CN_USE_SELECTTEX, TRUE );
}

void CPageEff_11_Neon::OnBnClickedCheckCnUseGlowSrc()
{
	SetWin_Check( this, IDC_CHECK_CN_USE_GLOW_SRC, TRUE );
	SetWin_Check( this, IDC_CHECK_CN_USE_GLOW_USR, FALSE );
}

void CPageEff_11_Neon::OnBnClickedCheckCnUseGlowUsr()
{
	SetWin_Check( this, IDC_CHECK_CN_USE_GLOW_SRC, FALSE );
	SetWin_Check( this, IDC_CHECK_CN_USE_GLOW_USR, TRUE );
}

void CPageEff_11_Neon::OnBnClickedButtonCnTexture()
{
	CString szFilter = "Texture Image (*.bmp,*.dds,*.tga,*.jpg)|*.bmp;*.dds;*.tga;*.jpg|";
	CFileDialog dlg(TRUE,".",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = TextureManager::GetTexturePath(); 
	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, IDC_EDIT_CN_TEXTURE, dlg.GetFileName().GetString() );
}

void CPageEff_11_Neon::OnBnClickedButtonCnColor()
{
	BYTE nCUR_R = GetWin_Num_int( this, IDC_EDIT_CN_COLOR_R );
	BYTE nCUR_G = GetWin_Num_int( this, IDC_EDIT_CN_COLOR_G );
	BYTE nCUR_B = GetWin_Num_int( this, IDC_EDIT_CN_COLOR_B );

	CSelColorDlg dlg( nCUR_R, nCUR_G, nCUR_B );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_int( this, IDC_EDIT_CN_COLOR_R, GetRValue( crNewColor ) );
		SetWin_Num_int( this, IDC_EDIT_CN_COLOR_G, GetGValue( crNewColor ) );
		SetWin_Num_int( this, IDC_EDIT_CN_COLOR_B, GetBValue( crNewColor ) );
	}
}

void CPageEff_11_Neon::OnNMDblclkListMaterial(NMHDR *pNMHDR, LRESULT *pResult)
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

void CPageEff_11_Neon::OnBnClickedButtonMaterialOk()
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
