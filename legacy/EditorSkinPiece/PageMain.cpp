// PageMain.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkinPiece.h"
#include "PageMain.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "DxEffChar.h"
#include "EditorSkinPieceView.h"
#include "DxSkinMeshMan.h"
#include "TextureManager.h"
#include "Global.h"

// CPageMain dialog

IMPLEMENT_DYNAMIC(CPageMain, CPropertyPage)

CPageMain::CPageMain(LOGFONT logfont)
	: CPropertyPage(CPageMain::IDD)
	, m_pFont ( NULL )
	, m_pPiece( NULL )
	, m_nMaterialEdit( -1 )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageMain::~CPageMain()
{
	SAFE_DELETE ( m_pFont );
}

void CPageMain::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_TRACE, m_List_Trace );
	DDX_Control(pDX, IDC_LIST_MESH, m_List_Mesh );
	DDX_Control(pDX, IDC_LIST_EFFECT, m_List_Effect );
	DDX_Control(pDX, IDC_LIST_MATERIAL, m_List_Material );
}


BEGIN_MESSAGE_MAP(CPageMain, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_SKIN, OnBnClickedButtonSkin)
	ON_BN_CLICKED(IDC_BUTTON_SKELETON, OnBnClickedButtonSkeleton)
	ON_BN_CLICKED(IDC_BUTTON_CREATESKIN, OnBnClickedButtonCreateskin)
	ON_BN_CLICKED(IDC_BUTTON_MESH, OnBnClickedButtonMesh)
	ON_BN_CLICKED(IDC_BUTTON_TRACE_ADD, OnBnClickedButtonTraceAdd)
	ON_BN_CLICKED(IDC_BUTTON_TRACE_DEL, OnBnClickedButtonTraceDel)
	ON_BN_CLICKED(IDC_BUTTON_TRACE_DELALL, OnBnClickedButtonTraceDelall)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_TRACE, OnNMDblclkListTrace)
	ON_BN_CLICKED(IDC_BUTTON_EFF_ADD, OnBnClickedButtonEffAdd)
	ON_BN_CLICKED(IDC_BUTTON_EFF_EDIT, OnBnClickedButtonEffEdit)
	ON_BN_CLICKED(IDC_BUTTON_EFF_DEL, OnBnClickedButtonEffDel)
	ON_BN_CLICKED(IDC_BUTTON_EFF_UP, OnBnClickedButtonEffUp)
	ON_BN_CLICKED(IDC_BUTTON_EFF_DOWN, OnBnClickedButtonEffDown)
	ON_BN_CLICKED(IDC_BUTTON_EFF_OK, OnBnClickedButtonEffOk)
	ON_BN_CLICKED(IDC_BUTTON_EFF_SORT, OnBnClickedButtonEffSort)
	ON_CBN_SELCHANGE(IDC_COMBO_PIECE, OnCbnSelchangeComboPiece)
	ON_CBN_SELCHANGE(IDC_COMBO_SLOT, OnCbnSelchangeComboSlot)
	ON_BN_CLICKED(IDC_BUTTON_MATERIAL_SAVE, OnBnClickedButtonMaterialSave)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_MATERIAL, OnNMDblclkListMaterial)
	ON_BN_CLICKED(IDC_BUTTON_MATERIAL_TEXTURE, OnBnClickedButtonMaterialTexture)
END_MESSAGE_MAP()


// CPageMain message handlers
BOOL CPageMain::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_List_Trace.SetExtendedStyle ( m_List_Trace.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List_Trace.GetClientRect ( &rectCtrl );
	m_List_Trace.InsertColumn( 0, "Trace", LVCFMT_LEFT, ( 35* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List_Trace.InsertColumn( 1, "TracePos", LVCFMT_LEFT, ( 65* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	m_List_Material.SetExtendedStyle ( m_List_Material.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List_Material.GetClientRect ( &rectCtrl );
	m_List_Material.InsertColumn( 0, "Material", LVCFMT_LEFT, ( 43* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List_Material.InsertColumn( 1, "Mask", LVCFMT_LEFT, ( 43* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List_Material.InsertColumn( 2, "ZBias", LVCFMT_LEFT, ( 14* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	
	m_List_Mesh.SetExtendedStyle ( m_List_Mesh.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List_Mesh.GetClientRect ( &rectCtrl );
	m_List_Mesh.InsertColumn( 0, "SkinMesh", LVCFMT_LEFT, ( 100* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	m_List_Effect.SetExtendedStyle ( m_List_Effect.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List_Effect.GetClientRect ( &rectCtrl );
	m_List_Effect.InsertColumn( 0, "EffectChar", LVCFMT_LEFT, ( 100* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	std::string strPIECE[PIECE_SIZE];
	for( int i=0; i<PIECE_SIZE; ++i )
	{
		strPIECE[i] = SKINTYPE_STRING::m_szPIECETYPE[i];
	}

	std::string strSLOT[EMPEACE_WEAPON_SIZE];
	for( int i=0; i<EMPEACE_WEAPON_SIZE; ++i )
	{
		strSLOT[i] = SKINTYPE_STRING::m_szWHEREBACKTYPE[i];
	}

	SetWin_Combo_Init ( this, IDC_COMBO_PIECE, strPIECE, PIECE_SIZE );
	SetWin_Combo_Init ( this, IDC_COMBO_SLOT, strSLOT, EMPEACE_WEAPON_SIZE );
	SetWin_Combo_Init ( this, IDC_COMBO_ZBIAS, GLOBAL_EFFTOOL::strEFFMAIN_ZBIAS, TOOL_MAIN_ZBIAS_SIZE );

	CComboBox *pCombo = (CComboBox*) ( GetDlgItem(IDC_COMBO_EFFECT) );
	if ( pCombo )
	{
		DxEffCharType* pType = DxEffCharMan::GetInstance().GetEffectList();
		while( pType )
		{
			int nIndex = pCombo->AddString ( pType->NAME );
			pCombo->SetItemDataPtr ( nIndex, pType );
			pType = pType->pNext;
		}

		pCombo->SetCurSel( 0 );
	}

	ResetTool();

	return TRUE;  
}

void CPageMain::SetData( DxSkinPiece* pDATA )
{
	m_pPiece = pDATA;

	ResetTool();
	ShowData();
}

void CPageMain::ResetTool()
{
	m_List_Trace.DeleteAllItems();
	m_List_Mesh.DeleteAllItems();
	m_List_Effect.DeleteAllItems();
	m_List_Material.DeleteAllItems();

	SetWin_Text( this, IDC_EDIT_FILENAME, "" );
	SetWin_Text( this, IDC_EDIT_SKIN, "" );
	SetWin_Text( this, IDC_EDIT_SKELETON, "" );
	SetWin_Text( this, IDC_EDIT_MESH, "" );
	SetWin_Text( this, IDC_EDIT_ZTEXTURE, "" );

	SetWin_Combo_Sel( this, IDC_COMBO_PIECE, 0 );
	SetWin_Combo_Sel( this, IDC_COMBO_SLOT, 0 );
	SetWin_Combo_Sel( this, IDC_COMBO_EFFECT, 0 );
	SetWin_Combo_Sel( this, IDC_COMBO_ZBIAS, 0 );

	SetWin_Enable( this, IDC_EDIT_ZTEXTURE, FALSE );
	SetWin_Enable( this, IDC_COMBO_ZBIAS, FALSE );
	SetWin_Enable( this, IDC_BUTTON_MATERIAL_SAVE, FALSE );
	SetWin_Enable( this, IDC_BUTTON_MATERIAL_TEXTURE, FALSE );

	SetWin_Enable( this, IDC_COMBO_EFFECT, FALSE );
	SetWin_Enable( this, IDC_BUTTON_EFF_OK, FALSE );

	m_nMaterialEdit = -1;
}

void CPageMain::ShowData()
{
	if ( !m_pPiece )	return;

	SetWin_Text( this, IDC_EDIT_FILENAME, m_pPiece->m_szFileName );
	SetWin_Text( this, IDC_EDIT_SKIN, m_pPiece->m_szXFileName );
	SetWin_Text( this, IDC_EDIT_SKELETON, m_pPiece->m_szSkeleton );
	SetWin_Text( this, IDC_EDIT_MESH, m_pPiece->m_szMeshName );

	SetWin_Combo_Sel( this, IDC_COMBO_PIECE, m_pPiece->m_emType );
	SetWin_Combo_Sel( this, IDC_COMBO_SLOT, m_pPiece->m_emWeaponWhereBack );

	ShowMesh();
	ShowTrace();
	ShowMaterial();
	ShowEffectChar();
}

void CPageMain::ShowMesh()
{
	m_List_Mesh.DeleteAllItems();

	if ( !m_pPiece )	return;
	if ( !m_pPiece->m_pSkinMesh )	return;

	m_List_Mesh.SetRedraw( FALSE );

	SMeshContainer* pMesh = m_pPiece->m_pSkinMesh->GetMeshClobalList();
	int i = 0;
	while ( pMesh )
	{
		m_List_Mesh.InsertItem( i, pMesh->Name );
		i ++;
		pMesh = pMesh->pGlobalNext;
	}

	m_List_Mesh.SetRedraw( TRUE );
}

void CPageMain::ShowTrace()
{
	int nSelect = m_List_Trace.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	m_List_Trace.DeleteAllItems();

	if ( !m_pPiece )	return;

	m_List_Trace.SetRedraw( FALSE );

	for ( int i=0; i<MAX_TRACE; i++ )
	{
		m_List_Trace.InsertItem( i, szTRACE_NAME[i] );
		SVERTEXINFLU* pTrace = m_pPiece->GetTracePos ( szTRACE_NAME[i] );
		if ( pTrace )
		{
			char szTempChar[512];
			sprintf( szTempChar, "(%d) %4.2f~%4.2f~%4.2f", pTrace->m_dwIndex, pTrace->m_vVector.x, pTrace->m_vVector.y, pTrace->m_vVector.z );
			m_List_Trace.SetItemText( i, 1, szTempChar );
			m_List_Trace.SetItemData( i, (DWORD_PTR) pTrace );
		}
	}

	m_List_Trace.SetRedraw( TRUE );

	if ( nSelect == -1 && m_List_Trace.GetItemCount() > 0 )
		nSelect = 0;

	if ( nSelect != -1 )
	{
		m_List_Trace.EnsureVisible( nSelect, TRUE );
		m_List_Trace.SetItemState( nSelect, LVIS_SELECTED, LVIS_SELECTED );
	}

	CEditorSkinPieceView::GetView()->m_strTraceSelect = "";
}

void CPageMain::ShowMaterial()
{
	int nSelect = m_List_Material.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	m_List_Material.DeleteAllItems();

	if ( !m_pPiece )	return;
	if ( !m_pPiece->m_pmcMesh )	return;
	
	m_List_Material.SetRedraw( FALSE );

	for ( DWORD i=0; i<m_pPiece->m_dwMaterialNum; ++i )
	{
		char szTempChar[512];
		m_List_Material.InsertItem( i, m_pPiece->m_pmcMesh->pMaterials[i].pTextureFilename );
		m_List_Material.SetItemText( i, 1, m_pPiece->m_pMaterialPiece[i].m_strTexture.c_str() );
		sprintf( szTempChar, "%d", m_pPiece->m_pMaterialPiece[i].m_dwZBias );
		m_List_Material.SetItemText( i, 2, szTempChar );
	}

	m_List_Material.SetRedraw( TRUE );

	if ( nSelect == -1 && m_List_Material.GetItemCount() > 0 )
		nSelect = 0;

	if ( nSelect != -1 )
	{
		m_List_Material.EnsureVisible( nSelect, TRUE );
		m_List_Material.SetItemState( nSelect, LVIS_SELECTED, LVIS_SELECTED );
	}

	SetWin_Enable( this, IDC_EDIT_ZTEXTURE, FALSE );
	SetWin_Enable( this, IDC_COMBO_ZBIAS, FALSE );
	SetWin_Enable( this, IDC_BUTTON_MATERIAL_SAVE, FALSE );
	SetWin_Enable( this, IDC_BUTTON_MATERIAL_TEXTURE, FALSE );
	m_nMaterialEdit = -1;
}

void CPageMain::ShowEffectChar()
{
	int nSelect = m_List_Effect.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	m_List_Effect.DeleteAllItems();

	if ( !m_pPiece )	return;

	m_List_Effect.SetRedraw( FALSE );

	for ( int i=0; i<(int)m_pPiece->m_vecEFFECT.size(); ++i )
	{
		char szTempChar[512];
		sprintf( szTempChar, "(%d) %s", m_pPiece->m_vecEFFECT[i]->GetTypeID(), m_pPiece->m_vecEFFECT[i]->GetName() );
		m_List_Effect.InsertItem( i, szTempChar );
		m_List_Effect.SetItemData( i, (DWORD_PTR) m_pPiece->m_vecEFFECT[i] );
	}

	m_List_Effect.SetRedraw( TRUE );

	if ( nSelect == -1 && m_List_Effect.GetItemCount() > 0 )
		nSelect = 0;

	if ( nSelect != -1 )
	{
		m_List_Effect.EnsureVisible( nSelect, TRUE );
		m_List_Effect.SetItemState( nSelect, LVIS_SELECTED, LVIS_SELECTED );
	}

	SetWin_Enable( this, IDC_COMBO_EFFECT, FALSE );
	SetWin_Enable( this, IDC_BUTTON_EFF_OK, FALSE );
}

void CPageMain::OnBnClickedButtonSkin()
{
	CString szFilterInput = "Skin (*.x)|*.x;|";
	CFileDialog dlgInput(true,"*.X*",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilterInput, this );
	dlgInput.m_ofn.lpstrInitialDir = DxSkinMeshMan::GetInstance().GetPath();
	if ( dlgInput.DoModal() != IDOK ) return;

	SetWin_Text( this, IDC_EDIT_SKIN, dlgInput.GetFileName() );
}

void CPageMain::OnBnClickedButtonSkeleton()
{
	CString szFilterInput = "Skeleton (*.x)|*.x;|";
	CFileDialog dlgInput(true,"*.X*",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilterInput, this );
	dlgInput.m_ofn.lpstrInitialDir = DxBoneCollector::GetInstance().GetPath();
	if ( dlgInput.DoModal() != IDOK ) return;

	SetWin_Text( this, IDC_EDIT_SKELETON, dlgInput.GetFileName() );
}

void CPageMain::OnBnClickedButtonCreateskin()
{
	if ( !m_pPiece )	return;

	std::string strSkin = GetWin_Text( this, IDC_EDIT_SKIN );
	std::string strSkeleton = GetWin_Text( this, IDC_EDIT_SKELETON );

	m_pPiece->ClearAll();
	HRESULT hr = m_pPiece->SetSkinMesh( strSkin.c_str(), strSkeleton.c_str(), CEditorSkinPieceView::GetView()->Get3DDevice(), false );
	if ( FAILED( hr ) )
	{
		CDebugSet::MsgBox( "Failed Setting Skin Mesh! Skin:%s Skeleton:%s", strSkin.c_str(), strSkeleton.c_str() );
		return;
	}

	ShowData();
}

void CPageMain::OnBnClickedButtonMesh()
{
	if ( !m_pPiece )	return;

	int nSelect = m_List_Mesh.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	std::string str = m_List_Mesh.GetItemText( nSelect, 0 );
	SMeshContainer* pMesh = m_pPiece->m_pSkinMesh->FindMeshContainer( (char*)str.c_str() );
	if ( pMesh )
	{
		m_pPiece->SelectMesh( pMesh );
		ShowData();
	}
}

void CPageMain::OnBnClickedButtonTraceAdd()
{
	if ( !m_pPiece )	return;
	if ( !m_pPiece->m_pmcMesh )	return;

	int nSelect = m_List_Trace.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	std::string str = m_List_Trace.GetItemText( nSelect, 0 );
	D3DXVECTOR3 vPOS = CEditorSkinPieceView::GetView()->m_vCOL;
	DWORD dwCOLID = CEditorSkinPieceView::GetView()->m_dwCOL;

	if ( vPOS != D3DXVECTOR3( 0.0f, 0.0f, 0.0f ) && dwCOLID != COL_ERR )
	{
		SVERTEXINFLU sVertex;
		m_pPiece->DelTracePos( str.c_str() );
		m_pPiece->SetTracePos( str.c_str(), sVertex );

		SVERTEXINFLU* pVertex = m_pPiece->GetTracePos( str.c_str() );
		if ( pVertex )
			m_pPiece->m_pmcMesh->GetVertexInfluences( dwCOLID, pVertex );
		
		ShowTrace();
	}
}

void CPageMain::OnBnClickedButtonTraceDel()
{
	if ( !m_pPiece )	return;

	int nSelect = m_List_Trace.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	std::string str = m_List_Trace.GetItemText( nSelect, 0 );
	m_pPiece->DelTracePos( str.c_str() );
	ShowTrace();
}

void CPageMain::OnBnClickedButtonTraceDelall()
{
	if ( !m_pPiece )	return;

	for ( int i=0; i<MAX_TRACE; i++ )
	{
		m_pPiece->DelTracePos( szTRACE_NAME[i] );
	}

	ShowTrace();
}

void CPageMain::OnNMDblclkListTrace(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if ( !m_pPiece )	return;

	int nSelect = m_List_Trace.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	std::string str = m_List_Trace.GetItemText( nSelect, 0 );
	CEditorSkinPieceView::GetView()->m_strTraceSelect = str.c_str();
}

void CPageMain::OnBnClickedButtonEffAdd()
{
	if ( !m_pPiece )	return;
	SetWin_Enable( this, IDC_COMBO_EFFECT, TRUE );
	SetWin_Enable( this, IDC_BUTTON_EFF_OK, TRUE );
}

void CPageMain::OnBnClickedButtonEffEdit()
{
	if ( !m_pPiece )	return;
	int nSelect = m_List_Effect.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	DxEffChar* pEff = ( DxEffChar* )m_List_Effect.GetItemData( nSelect );
	if ( pEff )
	{
		pEff->SetLinkObj ( NULL, m_pPiece );
		m_pSheetTab->EffEdit( m_pPiece, pEff );
	}
}

void CPageMain::OnBnClickedButtonEffDel()
{
	if ( !m_pPiece )	return;
	int nSelect = m_List_Effect.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	DxEffChar* pEff = ( DxEffChar* )m_List_Effect.GetItemData( nSelect );
	if ( pEff )
	{
		m_pPiece->DelEffList( pEff );
		ShowEffectChar();
	}
}

void CPageMain::OnBnClickedButtonEffUp()
{
	if ( !m_pPiece )	return;
	int nSelect = m_List_Effect.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	m_pPiece->EffUp( nSelect );
	ShowEffectChar();
}

void CPageMain::OnBnClickedButtonEffDown()
{
	if ( !m_pPiece )	return;
	int nSelect = m_List_Effect.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	m_pPiece->EffDown( nSelect );
	ShowEffectChar();
}

void CPageMain::OnBnClickedButtonEffOk()
{
	if ( !m_pPiece )	return;

	CComboBox *pCombo = (CComboBox*) ( GetDlgItem(IDC_COMBO_EFFECT) );
	if ( pCombo )
	{
		int nIndex = pCombo->GetCurSel ();
		DxEffCharType* pType = (DxEffCharType*)pCombo->GetItemData ( nIndex );
		if ( pType )
		{
			DxEffChar*	pEFF = DxEffCharMan::GetInstance().CreateEffInstance( pType->TYPEID );
			if ( pEFF )
			{
				pEFF->SetLinkObj ( NULL, m_pPiece );
				m_pSheetTab->EffEdit( m_pPiece, pEFF, TRUE );
				SetWin_Enable( this, IDC_COMBO_EFFECT, FALSE );
				SetWin_Enable( this, IDC_BUTTON_EFF_OK, FALSE );
			}
		}
	}
}

void CPageMain::OnBnClickedButtonEffSort()
{
	if ( !m_pPiece )	return;
	std::sort ( m_pPiece->m_vecEFFECT.begin(), m_pPiece->m_vecEFFECT.end(), DXEFFCHAR_OPER() );
	ShowEffectChar();
}

void CPageMain::OnCbnSelchangeComboPiece()
{
	if ( !m_pPiece )	return;
	m_pPiece->m_emType = (EMPIECECHAR)GetWin_Combo_Sel ( this, IDC_COMBO_PIECE );
}

void CPageMain::OnCbnSelchangeComboSlot()
{
	if ( !m_pPiece )	return;
	m_pPiece->m_emWeaponWhereBack = (EMPEACEZONEWEAPON)GetWin_Combo_Sel ( this, IDC_COMBO_SLOT );
}

void CPageMain::OnBnClickedButtonMaterialSave()
{
	if ( !m_pPiece )	return;

	if( m_nMaterialEdit == -1 || m_nMaterialEdit >= (int)m_pPiece->m_dwMaterialNum ) return;

	m_pPiece->m_pMaterialPiece[m_nMaterialEdit].m_strTexture = GetWin_Text( this, IDC_EDIT_ZTEXTURE );
	m_pPiece->m_pMaterialPiece[m_nMaterialEdit].m_dwZBias = GetWin_Combo_Sel( this, IDC_COMBO_ZBIAS );
	m_pPiece->m_pMaterialPiece[m_nMaterialEdit].LoadTexture( CEditorSkinPieceView::GetView()->Get3DDevice() );
	ShowMaterial();
}

void CPageMain::OnNMDblclkListMaterial(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if ( !m_pPiece )	return;

	int nSelect = m_List_Material.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	if( nSelect >= (int)m_pPiece->m_dwMaterialNum )	return;

	m_nMaterialEdit = nSelect;

	SetWin_Enable( this, IDC_EDIT_ZTEXTURE, TRUE );
	SetWin_Enable( this, IDC_COMBO_ZBIAS, TRUE );
	SetWin_Enable( this, IDC_BUTTON_MATERIAL_SAVE, TRUE );
	SetWin_Enable( this, IDC_BUTTON_MATERIAL_TEXTURE, TRUE );

	SetWin_Text( this, IDC_EDIT_ZTEXTURE, m_pPiece->m_pMaterialPiece[m_nMaterialEdit].m_strTexture.c_str() );
	SetWin_Combo_Sel( this, IDC_COMBO_ZBIAS, GLOBAL_EFFTOOL::strEFFMAIN_ZBIAS[m_pPiece->m_pMaterialPiece[m_nMaterialEdit].m_dwZBias].c_str() );
}

void CPageMain::OnBnClickedButtonMaterialTexture()
{
	CString szFilter = "Texture Image (*.bmp,*.dds,*.tga,*.jpg)|*.bmp;*.dds;*.tga;*.jpg|";
	CFileDialog dlg(TRUE,".",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = TextureManager::GetTexturePath(); 
	if ( dlg.DoModal() == IDOK )	SetWin_Text ( this, IDC_EDIT_ZTEXTURE, dlg.GetFileName().GetString() );
}



