// PageEff_22_Particle.cpp : implementation file
//

#include "stdafx.h"
#include "EditorSkinPiece.h"
#include "EditorSkinPieceView.h"
#include "PageEff_22_Particle.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "SelColorDlg.h"
#include "DxMethods.h"
#include "HLibDataConvert.h"

// CPageEff_22_Particle dialog

IMPLEMENT_DYNAMIC(CPageEff_22_Particle, CPropertyPage)

CPageEff_22_Particle::CPageEff_22_Particle(LOGFONT logfont)
	: CPropertyPage(CPageEff_22_Particle::IDD)
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

CPageEff_22_Particle::~CPageEff_22_Particle()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEff_22_Particle::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_LIST_POS, m_List_Pos );
}


BEGIN_MESSAGE_MAP(CPageEff_22_Particle, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_CPT_TEXTURE, OnBnClickedButtonCptTexture)
	ON_BN_CLICKED(IDC_BUTTON_CPT_COLOR, OnBnClickedButtonCptColor)
	ON_BN_CLICKED(IDC_BUTTON_CPT_LIST_ADD, OnBnClickedButtonCptListAdd)
	ON_BN_CLICKED(IDC_BUTTON_CPT_LIST_DEL, OnBnClickedButtonCptListDel)
	ON_BN_CLICKED(IDC_BUTTON_CPT_LIST_UP, OnBnClickedButtonCptListUp)
	ON_BN_CLICKED(IDC_BUTTON_CPT_LIST_DOWN, OnBnClickedButtonCptListDown)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_POS, OnNMDblclkListPos)
	ON_NOTIFY(NM_CLICK, IDC_LIST_POS, OnNMClickListPos)
	ON_BN_CLICKED(IDC_BUTTON_CPT_LIST_APPLY, OnBnClickedButtonCptListApply)
	ON_BN_CLICKED(IDC_BUTTON_CPT_LIST_EDIT, OnBnClickedButtonCptListEdit)
END_MESSAGE_MAP()


// CPageEff_22_Particle message handlers
BOOL CPageEff_22_Particle::OnInitDialog()
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

void CPageEff_22_Particle::DataSet( DxSkinPiece* pPiece, DxEffCharParticle* pEff, BOOL bADD )
{
	m_pPiece = pPiece;
	m_pEff = pEff;
	m_bADD = bADD;

	ResetTool();
	DataShow();
	PositionShow();
}

void CPageEff_22_Particle::ResetTool()
{
	SetWin_Num_int( this, IDC_EDIT_CPT_PARTICLEPERSEC, 0 );

	SetWin_Check( this, IDC_CHECK_CPT_RANDOMRANGE, FALSE );
	SetWin_Num_float( this, IDC_EDIT_CPT_RANDOMRANGE, 1.0f );

	SetWin_Num_float( this, IDC_EDIT_CPT_LIFE_MIN, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CPT_LIFE_MAX, 1.0f );

	SetWin_Num_float( this, IDC_EDIT_CPT_SCALE_X, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CPT_SCALE_Y, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CPT_SCALE_Z, 1.0f );

	SetWin_Check( this, IDC_CHECK_CPT_ROTATE, FALSE );
	SetWin_Num_float( this, IDC_EDIT_CPT_ROTATE_MIN, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CPT_ROTATE_MAX, 1.0f );

	SetWin_Check( this, IDC_CHECK_CPT_SEQUENCE, FALSE );
	SetWin_Num_int( this, IDC_EDIT_CPT_COL, 1 );
	SetWin_Num_int( this, IDC_EDIT_CPT_ROW, 1 );
	
	SetWin_Check( this, IDC_CHECK_CPT_REMAIN, FALSE );
	SetWin_Num_float( this, IDC_EDIT_CPT_VEL_MIN, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CPT_VEL_MAX, 1.0f );

	SetWin_Text( this, IDC_EDIT_CPT_TEXTURE, "" );

	SetWin_Num_float( this, IDC_EDIT_CPT_COLOR_A, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CPT_COLOR_R, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CPT_COLOR_G, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CPT_COLOR_B, 1.0f );

	m_List_Pos.DeleteAllItems();
	PosEdit( FALSE );
}

void CPageEff_22_Particle::DataShow()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	EFFCHAR_PROPERTY_PARTICLE* pProp = (EFFCHAR_PROPERTY_PARTICLE*)m_pEff->GetProperty();

	SetWin_Num_int( this, IDC_EDIT_CPT_PARTICLEPERSEC, pProp->m_dwParticlePec );

	SetWin_Check( this, IDC_CHECK_CPT_RANDOMRANGE, pProp->m_dwFlag&USE_RANDOMRANGE);
	SetWin_Num_float( this, IDC_EDIT_CPT_RANDOMRANGE, pProp->m_fRandomRange );

	SetWin_Num_float( this, IDC_EDIT_CPT_LIFE_MIN, pProp->m_fLifeMin );
	SetWin_Num_float( this, IDC_EDIT_CPT_LIFE_MAX, pProp->m_fLifeMax );

	SetWin_Num_float( this, IDC_EDIT_CPT_SCALE_X, pProp->m_vSize.x );
	SetWin_Num_float( this, IDC_EDIT_CPT_SCALE_Y, pProp->m_vSize.y );
	SetWin_Num_float( this, IDC_EDIT_CPT_SCALE_Z, pProp->m_vSize.z );

	SetWin_Check( this, IDC_CHECK_CPT_ROTATE, pProp->m_dwFlag&USE_ROTATE);
	SetWin_Num_float( this, IDC_EDIT_CPT_ROTATE_MIN, pProp->m_vRotate.x );
	SetWin_Num_float( this, IDC_EDIT_CPT_ROTATE_MAX, pProp->m_vRotate.y );

	SetWin_Check( this, IDC_CHECK_CPT_SEQUENCE, pProp->m_dwFlag&USE_SEQUENCE);
	SetWin_Num_int( this, IDC_EDIT_CPT_COL, pProp->m_dwSequenceCol );
	SetWin_Num_int( this, IDC_EDIT_CPT_ROW, pProp->m_dwSequenceRow );

	SetWin_Check( this, IDC_CHECK_CPT_REMAIN, pProp->m_dwFlag&USE_REMAIN);
	SetWin_Num_float( this, IDC_EDIT_CPT_VEL_MIN, pProp->m_vVelocity.x );
	SetWin_Num_float( this, IDC_EDIT_CPT_VEL_MAX, pProp->m_vVelocity.y );

	SetWin_Text( this, IDC_EDIT_CPT_TEXTURE, pProp->m_szTexture );

	SetWin_Num_int( this, IDC_EDIT_CPT_COLOR_R, RGBA_GETRED( pProp->m_dwColor ) );
	SetWin_Num_int( this, IDC_EDIT_CPT_COLOR_G, RGBA_GETGREEN( pProp->m_dwColor ) );
	SetWin_Num_int( this, IDC_EDIT_CPT_COLOR_B, RGBA_GETBLUE( pProp->m_dwColor ) );
	SetWin_Num_int( this, IDC_EDIT_CPT_COLOR_A, RGBA_GETALPHA( pProp->m_dwColor ) );
}

BOOL CPageEff_22_Particle::DataSave()
{
	if ( !m_pPiece )	return FALSE;
	if ( !m_pEff )		return FALSE;

	EFFCHAR_PROPERTY_PARTICLE sProperty;
	sProperty.m_dwParticlePec = GetWin_Num_int( this, IDC_EDIT_CPT_PARTICLEPERSEC );

	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CPT_RANDOMRANGE ), sProperty.m_dwFlag, USE_RANDOMRANGE );
	sProperty.m_fRandomRange = GetWin_Num_float( this, IDC_EDIT_CPT_RANDOMRANGE );

	sProperty.m_fLifeMin = GetWin_Num_float( this, IDC_EDIT_CPT_LIFE_MIN );
	sProperty.m_fLifeMax = GetWin_Num_float( this, IDC_EDIT_CPT_LIFE_MAX );

	sProperty.m_vSize.x = GetWin_Num_float( this, IDC_EDIT_CPT_SCALE_X );
	sProperty.m_vSize.y = GetWin_Num_float( this, IDC_EDIT_CPT_SCALE_Y );
	sProperty.m_vSize.z = GetWin_Num_float( this, IDC_EDIT_CPT_SCALE_Z );

	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CPT_ROTATE ), sProperty.m_dwFlag, USE_ROTATE );
	sProperty.m_vRotate.x = GetWin_Num_float( this, IDC_EDIT_CPT_ROTATE_MIN );
	sProperty.m_vRotate.y = GetWin_Num_float( this, IDC_EDIT_CPT_ROTATE_MAX );

	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CPT_SEQUENCE ), sProperty.m_dwFlag, USE_SEQUENCE );
	sProperty.m_dwSequenceCol = GetWin_Num_int( this, IDC_EDIT_CPT_COL );
	sProperty.m_dwSequenceRow = GetWin_Num_int( this, IDC_EDIT_CPT_ROW );

	SetCheck_Flags ( GetWin_Check ( this, IDC_CHECK_CPT_REMAIN ), sProperty.m_dwFlag, USE_REMAIN );
	sProperty.m_vVelocity.x = GetWin_Num_float( this, IDC_EDIT_CPT_VEL_MIN );
	sProperty.m_vVelocity.y = GetWin_Num_float( this, IDC_EDIT_CPT_VEL_MAX );
	
	std::string strTEX = GetWin_Text( this, IDC_EDIT_CPT_TEXTURE );
	StringCchCopy( sProperty.m_szTexture, TEX_SIZE_64, strTEX.c_str() );
	
	sProperty.m_dwColor = RGBA_MAKE(
		GetWin_Num_int( this, IDC_EDIT_CPT_COLOR_R ), 
		GetWin_Num_int( this, IDC_EDIT_CPT_COLOR_G ), 
		GetWin_Num_int( this, IDC_EDIT_CPT_COLOR_B ),
		GetWin_Num_int( this, IDC_EDIT_CPT_COLOR_A ));
	
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

void CPageEff_22_Particle::PositionShow()
{
	m_List_Pos.DeleteAllItems();

	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	m_List_Pos.SetRedraw( FALSE );

	DWORD dwSIZE = m_pEff->GetPoistionSize();
	for( DWORD i=0; i<dwSIZE; ++ i )
	{
		m_List_Pos.InsertItem( i, _HLIB::cstring_dword(i).GetString() );
		m_List_Pos.SetItemText( i, 1, m_pEff->GetPositionString( i ).c_str());
		m_List_Pos.SetItemData( i, i );
	}

	m_List_Pos.SetRedraw( TRUE );

	PosEdit( FALSE );
	CEditorSkinPieceView::GetView()->m_bEditMRS = FALSE;
	CEditorSkinPieceView::GetView()->m_DxEditMRS.SetMatrix( NULL );
	m_matEdit = D3DXMATRIX();
	m_pEff->EditApplyResetData();
}

void CPageEff_22_Particle::PosEdit( BOOL bEDIT )
{
	SetWin_Enable( this, IDC_BUTTON_CPT_LIST_APPLY, bEDIT );

	SetWin_Enable( this, IDC_BUTTON_CPT_LIST_ADD, !bEDIT );
	SetWin_Enable( this, IDC_BUTTON_CPT_LIST_EDIT, !bEDIT );
	SetWin_Enable( this, IDC_BUTTON_CPT_LIST_DEL, !bEDIT );
	SetWin_Enable( this, IDC_BUTTON_CPT_LIST_UP, !bEDIT );
	SetWin_Enable( this, IDC_BUTTON_CPT_LIST_DOWN, !bEDIT );
}

void CPageEff_22_Particle::PositionEdit()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	int nSelect = m_List_Pos.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	m_pEff->SetPoistionEditPos( nSelect );

	PosEdit( TRUE );

	D3DXVECTOR3 vecPOS = m_pEff->GetInfluencePosition();
	D3DXMatrixTranslation( &m_matEdit, vecPOS.x, vecPOS.y, vecPOS.z );
	CEditorSkinPieceView::GetView()->m_bEditMRS = TRUE;
	CEditorSkinPieceView::GetView()->m_DxEditMRS.SetMatrix( &m_matEdit );
}

void CPageEff_22_Particle::OnBnClickedButtonApply()
{
	DataSave();
}

void CPageEff_22_Particle::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		m_pPiece = NULL;
		m_pEff = NULL;

		if ( m_pSheetTab )
			m_pSheetTab->ShowMain();
	}
}

void CPageEff_22_Particle::OnBnClickedButtonClose()
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

void CPageEff_22_Particle::OnBnClickedButtonCptTexture()
{
	CString szFilter = "Texture Image (*.bmp,*.dds,*.tga,*.jpg)|*.bmp;*.dds;*.tga;*.jpg|";
	CFileDialog dlg(TRUE,".",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter, this );
	dlg.m_ofn.lpstrInitialDir = TextureManager::GetTexturePath(); 
	if ( dlg.DoModal() == IDOK )	
		SetWin_Text ( this, IDC_EDIT_CPT_TEXTURE, dlg.GetFileName().GetString() );
}

void CPageEff_22_Particle::OnBnClickedButtonCptColor()
{
	BYTE nCUR_R = GetWin_Num_int( this, IDC_EDIT_CPT_COLOR_R );
	BYTE nCUR_G = GetWin_Num_int( this, IDC_EDIT_CPT_COLOR_G );
	BYTE nCUR_B = GetWin_Num_int( this, IDC_EDIT_CPT_COLOR_B );

	CSelColorDlg dlg( nCUR_R, nCUR_G, nCUR_B );
	if (dlg.DoModal () == IDOK)
	{
		COLORREF crNewColor = dlg.GetColor();
		SetWin_Num_int( this, IDC_EDIT_CPT_COLOR_R, GetRValue( crNewColor ) );
		SetWin_Num_int( this, IDC_EDIT_CPT_COLOR_G, GetGValue( crNewColor ) );
		SetWin_Num_int( this, IDC_EDIT_CPT_COLOR_B, GetBValue( crNewColor ) );
	}
}

void CPageEff_22_Particle::OnBnClickedButtonCptListAdd()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	D3DXVECTOR3 vPOS = CEditorSkinPieceView::GetView()->m_vCOL;
	DWORD dwCOLID = CEditorSkinPieceView::GetView()->m_dwCOL;
	if ( vPOS != D3DXVECTOR3( 0.0f, 0.0f, 0.0f ) && dwCOLID != COL_ERR )
	{
		m_pEff->InsertPoint( vPOS );
		PositionShow();
	}
}

void CPageEff_22_Particle::OnBnClickedButtonCptListDel()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	int nSelect = m_List_Pos.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	m_pEff->DeletePoint( nSelect );
	PositionShow();
}

void CPageEff_22_Particle::OnBnClickedButtonCptListUp()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	int nSelect = m_List_Pos.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	m_pEff->DataUp( nSelect );
	PositionShow();
}

void CPageEff_22_Particle::OnBnClickedButtonCptListDown()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	int nSelect = m_List_Pos.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;

	m_pEff->DataDown( nSelect );
	PositionShow();
}

void CPageEff_22_Particle::OnNMDblclkListPos(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;	

	PositionEdit();
}

void CPageEff_22_Particle::OnNMClickListPos(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void CPageEff_22_Particle::OnBnClickedButtonCptListApply()
{
	if ( !m_pPiece )	return;
	if ( !m_pEff )		return;

	m_pEff->SetMatrixToMakePos( m_matEdit );
	PositionShow();
}

void CPageEff_22_Particle::OnBnClickedButtonCptListEdit()
{
	PositionEdit();
}
