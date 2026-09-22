// PageEff_28_AroundEff.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkinPiece.h"
#include "EditorSkinPieceView.h"
#include "PageEff_28_AroundEff.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "SelColorDlg.h"
#include "DxMethods.h"
#include "DxViewPort.h"

extern std::string	g_strPICK_BONE;
// CPageEff_28_AroundEff dialog

IMPLEMENT_DYNAMIC(CPageEff_28_AroundEff, CPropertyPage)

CPageEff_28_AroundEff::CPageEff_28_AroundEff(LOGFONT logfont)
	: CPropertyPage(CPageEff_28_AroundEff::IDD)
	, m_pFont ( NULL )
	, m_pPiece( NULL )
	, m_pEff( NULL )
	, m_bADD( FALSE )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageEff_28_AroundEff::~CPageEff_28_AroundEff()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEff_28_AroundEff::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_LIST_POS, m_List_Pos );
}


BEGIN_MESSAGE_MAP(CPageEff_28_AroundEff, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_CAR_TEXTURE, OnBnClickedButtonCarTexture)
	ON_BN_CLICKED(IDC_BUTTON_CAR_COLOR, OnBnClickedButtonCarColor)
	ON_BN_CLICKED(IDC_BUTTON_CAR_EDIT_BONE, OnBnClickedButtonCarEditBone)
	ON_BN_CLICKED(IDC_BUTTON_CAR_EDIT_POS, OnBnClickedButtonCarEditPos)
	ON_BN_CLICKED(IDC_BUTTON_CAR_EDIT_INSERT, OnBnClickedButtonCarEditInsert)
	ON_BN_CLICKED(IDC_BUTTON_CAR_EDIT_DELETE, OnBnClickedButtonCarEditDelete)
	ON_NOTIFY(NM_CLICK, IDC_LIST_POS, OnNMClickListPos)
END_MESSAGE_MAP()


// CPageEff_28_AroundEff message handlers
BOOL CPageEff_28_AroundEff::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	m_List_Pos.SetExtendedStyle ( m_List_Pos.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List_Pos.GetClientRect ( &rectCtrl );
	m_List_Pos.InsertColumn( 0, "Index", LVCFMT_LEFT, ( 10* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List_Pos.InsertColumn( 1, "Bone", LVCFMT_LEFT, ( 26* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List_Pos.InsertColumn( 2, "POS(X/Y/Z)", LVCFMT_LEFT, ( 27* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List_Pos.InsertColumn( 3, "Norm(X/Y/Z)", LVCFMT_LEFT, ( 27* ( rectCtrl.right - rectCtrl.left ) ) / 100 );
	m_List_Pos.InsertColumn( 4, "MDist", LVCFMT_LEFT, ( 10* ( rectCtrl.right - rectCtrl.left ) ) / 100 );

	ResetTool();

	return TRUE;
}

void CPageEff_28_AroundEff::DataSet( DxSkinPiece* pPiece, DxEffCharAroundEff* pEff, BOOL bADD )
{
	m_pPiece = pPiece;
	m_pEff = pEff;
	m_bADD = bADD;

	ResetTool();
	DataShow();
	PositionShow();
	CEditorSkinPieceView::GetView()->m_bRenderBone = TRUE;
}

void CPageEff_28_AroundEff::ResetTool()
{
	SetWin_Num_float( this, IDC_EDIT_CAR_LIFE_MIN, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CAR_LIFE_MAX, 1.0f );

	SetWin_Num_float( this, IDC_EDIT_CAR_REPLAY_MIN, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CAR_REPLAY_MAX, 1.0f );

	SetWin_Num_float( this, IDC_EDIT_CAR_X_SIZE_X, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CAR_X_SIZE_Y, 1.0f );

	SetWin_Num_float( this, IDC_EDIT_CAR_Y_SIZE_X, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CAR_Y_SIZE_Y, 1.0f );

	SetWin_Num_float( this, IDC_EDIT_CAR_TEXSPEED_MIN, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CAR_TEXSPEED_MAX, 1.0f );

	SetWin_Check( this, IDC_CHECK_CAR_USE_SEQ, FALSE );
	SetWin_Num_int( this, IDC_EDIT_CAR_SEQ_COL, 0 );
	SetWin_Num_int( this, IDC_EDIT_CAR_SEQ_ROW, 0 );

	SetWin_Text( this, IDC_EDIT_CAR_TEXTURE, "" );

	SetWin_Num_int( this, IDC_EDIT_CAR_COLOR_A, 0 );
	SetWin_Num_int( this, IDC_EDIT_CAR_COLOR_R, 0 );
	SetWin_Num_int( this, IDC_EDIT_CAR_COLOR_G, 0 );
	SetWin_Num_int( this, IDC_EDIT_CAR_COLOR_B, 0 );

	//data edit
	SetWin_Text( this, IDC_EDIT_CAR_EDIT_BONE, "" );
	SetWin_Num_float( this, IDC_EDIT_CAR_EDIT_POS_X, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_CAR_EDIT_POS_Y, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_CAR_EDIT_POS_Z, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_CAR_EDIT_NORM_X, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_CAR_EDIT_NORM_Y, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_CAR_EDIT_NORM_Z, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_CAR_EDIT_MESHDIST, 1.0f );
}

void CPageEff_28_AroundEff::DataShow()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	EFFCHAR_PROPERTY_CHAR_AROUND* pProp = (EFFCHAR_PROPERTY_CHAR_AROUND*)m_pEff->GetProperty();
	SetWin_Num_float( this, IDC_EDIT_CAR_LIFE_MIN, pProp->m_fMinLifeTime );
	SetWin_Num_float( this, IDC_EDIT_CAR_LIFE_MAX, pProp->m_fMaxLifeTime );

	SetWin_Num_float( this, IDC_EDIT_CAR_REPLAY_MIN, pProp->m_fMinRivalTime );
	SetWin_Num_float( this, IDC_EDIT_CAR_REPLAY_MAX, pProp->m_fMaxRivalTime );

	SetWin_Num_float( this, IDC_EDIT_CAR_X_SIZE_X, pProp->m_vMinSize.x );
	SetWin_Num_float( this, IDC_EDIT_CAR_X_SIZE_Y, pProp->m_vMinSize.y );

	SetWin_Num_float( this, IDC_EDIT_CAR_Y_SIZE_X, pProp->m_vMaxSize.x );
	SetWin_Num_float( this, IDC_EDIT_CAR_Y_SIZE_Y, pProp->m_vMaxSize.y );

	SetWin_Num_float( this, IDC_EDIT_CAR_TEXSPEED_MIN, pProp->m_fMinTextureSpeed );
	SetWin_Num_float( this, IDC_EDIT_CAR_TEXSPEED_MAX, pProp->m_fMaxTextureSpeed );

	SetWin_Check( this, IDC_CHECK_CAR_USE_SEQ, pProp->m_dwFlag&USE_SEQUENCE );
	SetWin_Num_int( this, IDC_EDIT_CAR_SEQ_COL, pProp->m_dwSequenceCol );
	SetWin_Num_int( this, IDC_EDIT_CAR_SEQ_ROW, pProp->m_dwSequenceRow );

	SetWin_Text( this, IDC_EDIT_CAR_TEXTURE, pProp->m_szTexture );

	SetWin_Num_int( this, IDC_EDIT_CAR_COLOR_R, RGBA_GETRED( pProp->m_dwColor ) );
	SetWin_Num_int( this, IDC_EDIT_CAR_COLOR_G, RGBA_GETGREEN( pProp->m_dwColor ) );
	SetWin_Num_int( this, IDC_EDIT_CAR_COLOR_B, RGBA_GETBLUE( pProp->m_dwColor ) );
	SetWin_Num_int( this, IDC_EDIT_CAR_COLOR_A, RGBA_GETALPHA( pProp->m_dwColor ) );
}

BOOL CPageEff_28_AroundEff::DataSave()
{
	if ( !m_pPiece )	return FALSE;
	if ( !m_pEff )		return FALSE;

	EFFCHAR_PROPERTY_CHAR_AROUND sProperty;

	sProperty.m_fMinLifeTime = GetWin_Num_float( this, IDC_EDIT_CAR_LIFE_MIN );
	sProperty.m_fMaxLifeTime = GetWin_Num_float( this, IDC_EDIT_CAR_LIFE_MAX );

	sProperty.m_fMinRivalTime = GetWin_Num_float( this, IDC_EDIT_CAR_REPLAY_MIN );
	sProperty.m_fMaxRivalTime = GetWin_Num_float( this, IDC_EDIT_CAR_REPLAY_MAX );

	sProperty.m_vMinSize.x = GetWin_Num_float( this, IDC_EDIT_CAR_X_SIZE_X );
	sProperty.m_vMinSize.y = GetWin_Num_float( this, IDC_EDIT_CAR_X_SIZE_Y );

	sProperty.m_vMaxSize.x = GetWin_Num_float( this, IDC_EDIT_CAR_Y_SIZE_X );
	sProperty.m_vMaxSize.y = GetWin_Num_float( this, IDC_EDIT_CAR_Y_SIZE_Y );

	sProperty.m_fMinTextureSpeed = GetWin_Num_float( this, IDC_EDIT_CAR_TEXSPEED_MIN );
	sProperty.m_fMaxTextureSpeed = GetWin_Num_float( this, IDC_EDIT_CAR_TEXSPEED_MAX );

	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CAR_USE_SEQ ), sProperty.m_dwFlag, USE_SEQUENCE );
	sProperty.m_dwSequenceCol = GetWin_Num_int( this, IDC_EDIT_CAR_SEQ_COL );
	sProperty.m_dwSequenceRow = GetWin_Num_int( this, IDC_EDIT_CAR_SEQ_ROW );

	std::string str = GetWin_Text( this, IDC_EDIT_CAR_TEXTURE );
	StringCchCopy( sProperty.m_szTexture, AROUNDEFFECT_TEXNAME_SIZE, str.c_str() );

	sProperty.m_dwColor = RGBA_MAKE(
		GetWin_Num_int( this, IDC_EDIT_CAR_COLOR_R ), 
		GetWin_Num_int( this, IDC_EDIT_CAR_COLOR_G ), 
		GetWin_Num_int( this, IDC_EDIT_CAR_COLOR_B ),
		GetWin_Num_int( this, IDC_EDIT_CAR_COLOR_A ));


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

void CPageEff_28_AroundEff::PositionShow()
{
	m_List_Pos.DeleteAllItems();

	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	vector<SAROUNDEFF_DATA> vecData;
	vecData.clear();
	vecData = m_pEff->GetAroundEffect();

	m_List_Pos.SetRedraw( FALSE );

	size_t nSize = vecData.size();
	for( size_t i=0; i<nSize; ++i )
	{
		SAROUNDEFF_DATA sData = vecData[i];
		char szTempChar[1024];
		sprintf( szTempChar, "%d", i );
		m_List_Pos.InsertItem( i, szTempChar );
		m_List_Pos.SetItemText( i, 1, sData.strBoneName.c_str() );
		sprintf( szTempChar, "X:%4.2f~Y:%4.2f~Z:%4.2f", sData.vPos.x, sData.vPos.y, sData.vPos.z );
		m_List_Pos.SetItemText( i, 2, szTempChar );
		sprintf( szTempChar, "X:%4.2f~Y:%4.2f~Z:%4.2f", sData.vNormal.x, sData.vNormal.y, sData.vNormal.z );
		m_List_Pos.SetItemText( i, 3, szTempChar );
		sprintf( szTempChar, "%4.2f", sData.fMeshDist );
		m_List_Pos.SetItemText( i, 4, szTempChar );
		m_List_Pos.SetItemData( i, i );
	}

	m_List_Pos.SetRedraw( TRUE );

	//reset edit
	SetWin_Text( this, IDC_EDIT_CAR_EDIT_BONE, "" );
	SetWin_Num_float( this, IDC_EDIT_CAR_EDIT_POS_X, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_CAR_EDIT_POS_Y, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_CAR_EDIT_POS_Z, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_CAR_EDIT_NORM_X, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_CAR_EDIT_NORM_Y, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_CAR_EDIT_NORM_Z, 0.0f );
	SetWin_Num_float( this, IDC_EDIT_CAR_EDIT_MESHDIST, 1.0f );
}

void CPageEff_28_AroundEff::OnBnClickedButtonApply()
{
	DataSave();
}

void CPageEff_28_AroundEff::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		m_pPiece = NULL;
		m_pEff = NULL;

		if ( m_pSheetTab )
			m_pSheetTab->ShowMain();
	}
}

void CPageEff_28_AroundEff::OnBnClickedButtonClose()
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

void CPageEff_28_AroundEff::OnBnClickedButtonCarTexture()
{
	CString szFilter = "Texture Image (*.bmp,*.dds,*.tga,*.jpg)|*.bmp;*.dds;*.tga;*.jpg|";
	CFileDialog dlg(TRUE,".",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = TextureManager::GetTexturePath(); 
	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, IDC_EDIT_CAR_TEXTURE, dlg.GetFileName().GetString() );
}

void CPageEff_28_AroundEff::OnBnClickedButtonCarColor()
{
	BYTE nCUR_R = GetWin_Num_int( this, IDC_EDIT_CAR_COLOR_R );
	BYTE nCUR_G = GetWin_Num_int( this, IDC_EDIT_CAR_COLOR_G );
	BYTE nCUR_B = GetWin_Num_int( this, IDC_EDIT_CAR_COLOR_B );

	CSelColorDlg dlg( nCUR_R, nCUR_G, nCUR_B );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_int( this, IDC_EDIT_CAR_COLOR_R, GetRValue( crNewColor ) );
		SetWin_Num_int( this, IDC_EDIT_CAR_COLOR_G, GetGValue( crNewColor ) );
		SetWin_Num_int( this, IDC_EDIT_CAR_COLOR_B, GetBValue( crNewColor ) );
	}
}

void CPageEff_28_AroundEff::OnBnClickedButtonCarEditBone()
{
	if ( g_strPICK_BONE.size() <= 0 )
	{
		CDebugSet::MsgBox( "No Bone Selected!" );
		return;
	}else{
		SetWin_Text( this, IDC_EDIT_CAR_EDIT_BONE, g_strPICK_BONE.c_str() );
	}
}

void CPageEff_28_AroundEff::OnBnClickedButtonCarEditPos()
{
	D3DXVECTOR3 vPOS = CEditorSkinPieceView::GetView()->m_vCOL;
	DWORD dwCOLID = CEditorSkinPieceView::GetView()->m_dwCOL;
	if ( vPOS != D3DXVECTOR3( 0.0f, 0.0f, 0.0f ) && dwCOLID != COL_ERR )
	{
		SetWin_Num_float( this, IDC_EDIT_CAR_EDIT_POS_X, vPOS.x );
		SetWin_Num_float( this, IDC_EDIT_CAR_EDIT_POS_Y, vPOS.y );
		SetWin_Num_float( this, IDC_EDIT_CAR_EDIT_POS_Z, vPOS.z );
	}else{
		CDebugSet::MsgBox( "Pos Invalid!" );
		return;
	}
}

void CPageEff_28_AroundEff::OnBnClickedButtonCarEditInsert()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;
	if ( !m_pPiece->m_pSkeleton )	return;

	SAROUNDEFF_DATA sDATA;
	sDATA.strBoneName = GetWin_Text( this, IDC_EDIT_CAR_EDIT_BONE ).GetString();
	sDATA.vPos.x = GetWin_Num_float( this, IDC_EDIT_CAR_EDIT_POS_X );
	sDATA.vPos.y = GetWin_Num_float( this, IDC_EDIT_CAR_EDIT_POS_Y );
	sDATA.vPos.z = GetWin_Num_float( this, IDC_EDIT_CAR_EDIT_POS_Z );
	sDATA.vNormal.x = GetWin_Num_float( this, IDC_EDIT_CAR_EDIT_NORM_X );
	sDATA.vNormal.y = GetWin_Num_float( this, IDC_EDIT_CAR_EDIT_NORM_Y );
	sDATA.vNormal.z = GetWin_Num_float( this, IDC_EDIT_CAR_EDIT_NORM_Z );
	sDATA.fMeshDist = GetWin_Num_float( this, IDC_EDIT_CAR_EDIT_MESHDIST );

	if ( sDATA.strBoneName.size() <= 0 )
	{
		CDebugSet::MsgBox( GetSafeHwnd(), "Invalid Bone!" );
		return;
	}

	m_pEff->InsertAroundEffect( sDATA, TRUE );
	PositionShow();
}

void CPageEff_28_AroundEff::OnBnClickedButtonCarEditDelete()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	int nSelect = m_List_Pos.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	m_pEff->DeleteAroundEffect( nSelect );
	PositionShow();
}

void CPageEff_28_AroundEff::OnNMClickListPos(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;
	if ( !m_pPiece->m_pSkeleton )	return;

	int nSelect = m_List_Pos.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	vector<SAROUNDEFF_DATA> vecData;
	vecData.clear();
	vecData = m_pEff->GetAroundEffect();
	if ( nSelect >= (int)vecData.size() )	return;
	SAROUNDEFF_DATA sData = vecData[nSelect];

	DxBoneTrans*	pBoneTran = m_pPiece->m_pSkeleton->FindBone( sData.strBoneName.c_str() );
	if( pBoneTran )
	{
		D3DXVECTOR3 vBonePos = D3DXVECTOR3( pBoneTran->matCombined._41, pBoneTran->matCombined._42, pBoneTran->matCombined._43 );
		D3DXVECTOR3 vPos = sData.vPos + vBonePos;	
		DxViewPort::GetInstance().CameraJump(vPos);
	}

}
