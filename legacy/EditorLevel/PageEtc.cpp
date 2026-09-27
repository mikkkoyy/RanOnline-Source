// PageEtc.cpp : implementation file
//

#include "stdafx.h"
#include "EditorLevel.h"
#include "PageEtc.h"
#include "SheetWithTab.h"
#include "EtcFunction.h"
#include "EditorLevelView.h"

// CPageEtc dialog

IMPLEMENT_DYNAMIC(CPageEtc, CPropertyPage)

CPageEtc::CPageEtc(LOGFONT logfont)
	: CPropertyPage(CPageEtc::IDD)
	, m_pFont ( NULL )
	, m_pGLLand( NULL )
{
	m_bDlgInit = FALSE;
	m_pFont = new CFont();
	m_pFont->CreateFontIndirect(&logfont);
}

CPageEtc::~CPageEtc()
{
	SAFE_DELETE ( m_pFont );
}

void CPageEtc::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_List);
}


BEGIN_MESSAGE_MAP(CPageEtc, CPropertyPage)
	ON_BN_CLICKED(IDC_CHECK_MOBCHASE, OnBnClickedCheckMobchase)
	ON_BN_CLICKED(IDC_CHECK_LIMITTIME, OnBnClickedCheckLimittime)
	ON_BN_CLICKED(IDC_CHECK_CONTROLCAM, OnBnClickedCheckControlcam)
	ON_BN_CLICKED(IDC_CHECK_LANDEFFECT, OnBnClickedCheckLandeffect)
	ON_BN_CLICKED(IDC_BUTTON_CONTROLCAM, OnBnClickedButtonControlcam)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnNMDblclkList)
	ON_BN_CLICKED(IDC_BUTTON_EFF_ADD, OnBnClickedButtonEffAdd)
	ON_BN_CLICKED(IDC_BUTTON_EFF_EDIT, OnBnClickedButtonEffEdit)
	ON_BN_CLICKED(IDC_BUTTON_EFF_DELETE, OnBnClickedButtonEffDelete)
	ON_BN_CLICKED(IDC_BUTTON_EFF_POSSCREEN, OnBnClickedButtonEffPosscreen)
	ON_BN_CLICKED(IDC_BUTTON_EFF_CLOSE, OnBnClickedButtonEffClose)
	ON_BN_CLICKED(IDC_BUTTON_EFF_SAVE, OnBnClickedButtonEffSave)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnBnClickedButtonClose)
END_MESSAGE_MAP()


// CPageEtc message handlers
BOOL CPageEtc::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_pSheetTab->ChangeDialogFont( this, m_pFont, CDF_TOPLEFT );
	m_bDlgInit = TRUE;

	RECT rectCtrl;
	LONG lnWidth;
	LV_COLUMN lvColumn;

	m_List.SetExtendedStyle ( m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_List.GetClientRect ( &rectCtrl );
	lnWidth = rectCtrl.right - rectCtrl.left;

	const int nColumnCount = 5;	
	char* szColumnName[nColumnCount] = { "#", "Type", "Var", "Min(X/Y)", "Max(X/Y)" };
	int nColumnWidthPercent[nColumnCount] = { 5, 26, 15, 27, 27 };

	for ( int i = 0; i < nColumnCount; i++ )
	{		
		lvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvColumn.fmt = LVCFMT_LEFT;
		lvColumn.pszText = szColumnName[i];
		lvColumn.iSubItem = i;
		lvColumn.cx = ( lnWidth*nColumnWidthPercent[i] ) / 100;
		m_List.InsertColumn (i, &lvColumn );
	}

	SetWin_Combo_Init( this, IDC_COMBO_EFF_TYPE, COMMENT::LANDEFFECT_TYPE, EMLANDEFFECT_SIZE );
	InitUI();

	return TRUE;  
}

void CPageEtc::DataSet( GLLandMan* pLand )
{
	m_pGLLand = pLand;
	
	DataShow();
	InitUI();
}

void CPageEtc::ResetEditor()
{
	m_pGLLand = NULL;
}

void CPageEtc::InitUI()
{
	BOOL bLIMITTIME = GetWin_Check( this, IDC_CHECK_LIMITTIME );
	SetWin_Enable( this, IDC_EDIT_LIMIT_MINUTE, bLIMITTIME );
	SetWin_Enable( this, IDC_EDIT_LIMIT_TERMMINUTE, bLIMITTIME );

	BOOL bCONTROLCAM = GetWin_Check( this, IDC_CHECK_CONTROLCAM );
	SetWin_Enable( this, IDC_EDIT_CAM_POSX, bCONTROLCAM );
	SetWin_Enable( this, IDC_EDIT_CAM_POSY, bCONTROLCAM );
	SetWin_Enable( this, IDC_EDIT_CAM_POSZ, bCONTROLCAM );
	SetWin_Enable( this, IDC_EDIT_CAM_LEFT, bCONTROLCAM );
	SetWin_Enable( this, IDC_EDIT_CAM_RIGHT, bCONTROLCAM );
	SetWin_Enable( this, IDC_EDIT_CAM_DOWN, bCONTROLCAM );
	SetWin_Enable( this, IDC_EDIT_CAM_UP, bCONTROLCAM );
	SetWin_Enable( this, IDC_EDIT_CAM_NEAR, bCONTROLCAM );
	SetWin_Enable( this, IDC_EDIT_CAM_FAR, bCONTROLCAM );
	SetWin_Enable( this, IDC_BUTTON_CONTROLCAM, bCONTROLCAM );


	BOOL bEFFECT = GetWin_Check( this, IDC_CHECK_LANDEFFECT );
	SetWin_Enable( this, IDC_LIST, bEFFECT );
	SetWin_Enable( this, IDC_BUTTON_EFF_ADD, bEFFECT );
	SetWin_Enable( this, IDC_BUTTON_EFF_EDIT, bEFFECT );
	SetWin_Enable( this, IDC_BUTTON_EFF_DELETE, bEFFECT );
}

void CPageEtc::DataShow()
{
	if ( !m_pGLLand )	return;
	if ( !m_pGLLand->GetLevelEtcFunc() )	return;

	SetWin_Check( this, IDC_CHECK_MOBCHASE, m_pGLLand->GetLevelEtcFunc()->m_bUseFunction[EMETCFUNC_MOBCHASE] );
	SetWin_Check( this, IDC_CHECK_LIMITTIME, m_pGLLand->GetLevelEtcFunc()->m_bUseFunction[EMETCFUNC_LIMITTIME] );
	SetWin_Check( this, IDC_CHECK_CONTROLCAM, m_pGLLand->GetLevelEtcFunc()->m_bUseFunction[EMETCFUNC_CONTROLCAM] );
	SetWin_Check( this, IDC_CHECK_LANDEFFECT, m_pGLLand->GetLevelEtcFunc()->m_bUseFunction[EMETCFUNC_LANDEFFECT] );
	SetWin_Num_int( this, IDC_EDIT_LIMIT_MINUTE, m_pGLLand->GetLevelEtcFunc()->m_sLimitTime.nLimitMinute );
	SetWin_Num_int( this, IDC_EDIT_LIMIT_TERMMINUTE, m_pGLLand->GetLevelEtcFunc()->m_sLimitTime.nTellTermMinute );
	SetWin_Num_float( this, IDC_EDIT_CAM_POSX, m_pGLLand->GetLevelEtcFunc()->m_sControlCam.vCamPos.x );
	SetWin_Num_float( this, IDC_EDIT_CAM_POSY, m_pGLLand->GetLevelEtcFunc()->m_sControlCam.vCamPos.y );
	SetWin_Num_float( this, IDC_EDIT_CAM_POSZ, m_pGLLand->GetLevelEtcFunc()->m_sControlCam.vCamPos.z );
	SetWin_Num_float( this, IDC_EDIT_CAM_LEFT, m_pGLLand->GetLevelEtcFunc()->m_sControlCam.fLeft );
	SetWin_Num_float( this, IDC_EDIT_CAM_RIGHT, m_pGLLand->GetLevelEtcFunc()->m_sControlCam.fRight );
	SetWin_Num_float( this, IDC_EDIT_CAM_DOWN, m_pGLLand->GetLevelEtcFunc()->m_sControlCam.fDown );
	SetWin_Num_float( this, IDC_EDIT_CAM_UP, m_pGLLand->GetLevelEtcFunc()->m_sControlCam.fUp );
	SetWin_Num_float( this, IDC_EDIT_CAM_NEAR, m_pGLLand->GetLevelEtcFunc()->m_sControlCam.fNear );
	SetWin_Num_float( this, IDC_EDIT_CAM_FAR, m_pGLLand->GetLevelEtcFunc()->m_sControlCam.fFar );

	EffShow();
}

BOOL CPageEtc::DataSave()
{
	if ( !m_pGLLand )	return FALSE;
	if ( !m_pGLLand->GetLevelEtcFunc() )	return FALSE;

	m_pGLLand->GetLevelEtcFunc()->m_bUseFunction[EMETCFUNC_MOBCHASE] = ( GetWin_Check( this, IDC_CHECK_MOBCHASE ) == TRUE );
	m_pGLLand->GetLevelEtcFunc()->m_bUseFunction[EMETCFUNC_LIMITTIME] = ( GetWin_Check( this, IDC_CHECK_LIMITTIME ) == TRUE );
	m_pGLLand->GetLevelEtcFunc()->m_bUseFunction[EMETCFUNC_CONTROLCAM] = ( GetWin_Check( this, IDC_CHECK_CONTROLCAM ) == TRUE );
	m_pGLLand->GetLevelEtcFunc()->m_bUseFunction[EMETCFUNC_LANDEFFECT] = ( GetWin_Check( this, IDC_CHECK_LANDEFFECT ) == TRUE );
	m_pGLLand->GetLevelEtcFunc()->m_sLimitTime.nLimitMinute = GetWin_Num_int( this, IDC_EDIT_LIMIT_MINUTE );
	m_pGLLand->GetLevelEtcFunc()->m_sLimitTime.nTellTermMinute = GetWin_Num_int( this, IDC_EDIT_LIMIT_TERMMINUTE );
	m_pGLLand->GetLevelEtcFunc()->m_sControlCam.vCamPos.x = GetWin_Num_float( this, IDC_EDIT_CAM_POSX );
	m_pGLLand->GetLevelEtcFunc()->m_sControlCam.vCamPos.y = GetWin_Num_float( this, IDC_EDIT_CAM_POSY );
	m_pGLLand->GetLevelEtcFunc()->m_sControlCam.vCamPos.z = GetWin_Num_float( this, IDC_EDIT_CAM_POSZ );
	m_pGLLand->GetLevelEtcFunc()->m_sControlCam.fLeft = GetWin_Num_float( this, IDC_EDIT_CAM_LEFT );
	m_pGLLand->GetLevelEtcFunc()->m_sControlCam.fRight = GetWin_Num_float( this, IDC_EDIT_CAM_RIGHT );
	m_pGLLand->GetLevelEtcFunc()->m_sControlCam.fDown = GetWin_Num_float( this, IDC_EDIT_CAM_DOWN );
	m_pGLLand->GetLevelEtcFunc()->m_sControlCam.fUp = GetWin_Num_float( this, IDC_EDIT_CAM_UP );
	m_pGLLand->GetLevelEtcFunc()->m_sControlCam.fNear = GetWin_Num_float( this, IDC_EDIT_CAM_NEAR );
	m_pGLLand->GetLevelEtcFunc()->m_sControlCam.fFar = GetWin_Num_float( this, IDC_EDIT_CAM_FAR );

	return TRUE;
}

void CPageEtc::EffShow()
{
	if ( !m_pGLLand )	return;
	if ( !m_pGLLand->GetLevelEtcFunc() )	return;

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	m_List.DeleteAllItems();

	if ( !m_pGLLand )	return;

	m_List.SetRedraw( FALSE );

	char szTempChar[512];
	for( int i = 0; i < (int)m_pGLLand->GetLevelEtcFunc()->m_vecLandEffect.size(); ++ i )
	{
		SLANDEFFECT landeff = m_pGLLand->GetLevelEtcFunc()->m_vecLandEffect[i];
		sprintf( szTempChar, "%d", i );
		m_List.InsertItem( i, szTempChar );
		m_List.SetItemText( i, 1, COMMENT::LANDEFFECT_TYPE[landeff.emLandEffectType].c_str() );
		sprintf( szTempChar, "%g", landeff.fValue );
		m_List.SetItemText( i, 2, szTempChar );
		sprintf( szTempChar, "%.2f ~ %.2f", landeff.vMinPos.x, landeff.vMinPos.y );
		m_List.SetItemText( i, 3, szTempChar );
		sprintf( szTempChar, "%.2f ~ %.2f", landeff.vMaxPos.x, landeff.vMaxPos.y );
		m_List.SetItemText( i, 4, szTempChar );
		m_List.SetItemData( i, i );
	}
	
	m_List.SetRedraw( TRUE );

	if ( nSelect == -1 && m_List.GetItemCount() > 0 )
		nSelect = 0;

	if ( nSelect != -1 )
	{
		m_List.EnsureVisible( nSelect, TRUE );
		m_List.SetItemState( nSelect, LVIS_SELECTED, LVIS_SELECTED );
	}

	BOOL bENABLE = FALSE;
	SetWin_Enable( this, IDC_COMBO_EFF_TYPE, bENABLE );
	SetWin_Enable( this, IDC_EDIT_EFF_VAL, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_EFF_POSSCREEN, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_EFF_CLOSE, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_EFF_SAVE, bENABLE );

	SetWin_Combo_Sel( this, IDC_COMBO_EFF_TYPE, 0 );
	SetWin_Num_float( this, IDC_EDIT_EFF_VAL, 0.0f );
	SetWin_Num_int( this, IDC_EDIT_EFF_NUM, -1 );

	CEditorLevelView::GetView()->EditBoxSet( FALSE );
}

void CPageEtc::JumpTo( D3DXVECTOR2 vMIN, D3DXVECTOR2 vMAX )
{
	D3DXVECTOR2	vec2Center;
	vec2Center.x=((vMIN.x + vMAX.x ) / 2);
	vec2Center.y=((vMIN.y + vMAX.y ) / 2);

	DxViewPort::GetInstance().CameraJump( D3DXVECTOR3( vec2Center.x, 0.0f, vec2Center.y ) );
}

void CPageEtc::OnBnClickedCheckMobchase()
{
	InitUI();
}

void CPageEtc::OnBnClickedCheckLimittime()
{
	InitUI();
}

void CPageEtc::OnBnClickedCheckControlcam()
{
	InitUI();
}

void CPageEtc::OnBnClickedCheckLandeffect()
{
	InitUI();
}

void CPageEtc::OnBnClickedButtonControlcam()
{
	D3DXVECTOR3 vFrom = DxViewPort::GetInstance().GetFromPt();

	SetWin_Num_float( this, IDC_EDIT_CAM_POSX, vFrom.x );
	SetWin_Num_float( this, IDC_EDIT_CAM_POSY, vFrom.y );
	SetWin_Num_float( this, IDC_EDIT_CAM_POSZ, vFrom.z );
	SetWin_Num_float( this, IDC_EDIT_CAM_LEFT, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CAM_RIGHT, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CAM_DOWN, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CAM_UP, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CAM_NEAR, 1.0f );
	SetWin_Num_float( this, IDC_EDIT_CAM_FAR, 1.0f );
}

void CPageEtc::OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if ( !m_pGLLand )	return;
	if ( !m_pGLLand->GetLevelEtcFunc() )	return;

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	if( nSelect >= (int)m_pGLLand->GetLevelEtcFunc()->m_vecLandEffect.size() )	return;
	SLANDEFFECT landEffect = m_pGLLand->GetLevelEtcFunc()->m_vecLandEffect[nSelect];
	JumpTo( landEffect.vMinPos, landEffect.vMaxPos );
}

void CPageEtc::OnBnClickedButtonEffAdd()
{
	if ( !m_pGLLand )	return;

	BOOL bENABLE = TRUE;
	SetWin_Enable( this, IDC_COMBO_EFF_TYPE, bENABLE );
	SetWin_Enable( this, IDC_EDIT_EFF_VAL, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_EFF_POSSCREEN, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_EFF_CLOSE, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_EFF_SAVE, bENABLE );
	SetWin_Num_int( this, IDC_EDIT_EFF_NUM, -1 );

	//use default mouse pos
	D3DXVECTOR3 vPOS = CEditorLevelView::GetView()->m_vPos;
	D3DXVECTOR3 vMIN( vPOS.x - 10.0f, vPOS.y - 10.0f, vPOS.z - 10.0f );
	D3DXVECTOR3 vMAX( vPOS.x + 10.0f, vPOS.y + 10.0f, vPOS.z + 10.0f );

	CEditorLevelView::GetView()->EditBoxSet( TRUE );
	CEditorLevelView::GetView()->m_DxEditBox.SetBox( vMAX, vMIN );
	JumpTo( D3DXVECTOR2( vMIN.x, vMIN.z ), D3DXVECTOR2( vMAX.x, vMAX.z ) );
}

void CPageEtc::OnBnClickedButtonEffEdit()
{
	if ( !m_pGLLand )	return;
	if ( !m_pGLLand->GetLevelEtcFunc() )	return;

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	if( nSelect >= (int)m_pGLLand->GetLevelEtcFunc()->m_vecLandEffect.size() )	return;

	BOOL bENABLE = TRUE;
	SetWin_Enable( this, IDC_COMBO_EFF_TYPE, bENABLE );
	SetWin_Enable( this, IDC_EDIT_EFF_VAL, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_EFF_POSSCREEN, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_EFF_CLOSE, bENABLE );
	SetWin_Enable( this, IDC_BUTTON_EFF_SAVE, bENABLE );

	SLANDEFFECT landEffect = m_pGLLand->GetLevelEtcFunc()->m_vecLandEffect[nSelect];
	SetWin_Combo_Sel( this, IDC_COMBO_EFF_TYPE, COMMENT::LANDEFFECT_TYPE[landEffect.emLandEffectType].c_str() );
	SetWin_Num_float( this, IDC_EDIT_EFF_VAL, landEffect.fValue );
	SetWin_Num_int( this, IDC_EDIT_EFF_NUM, nSelect );

	D3DXVECTOR3 vMIN = D3DXVECTOR3( landEffect.vMinPos.x, 10.0f, landEffect.vMinPos.y );
	D3DXVECTOR3 vMAX = D3DXVECTOR3( landEffect.vMaxPos.x, 10.0f, landEffect.vMaxPos.y );

	CEditorLevelView::GetView()->EditBoxSet( TRUE );
	CEditorLevelView::GetView()->m_DxEditBox.SetBox( vMAX, vMIN );
	JumpTo( D3DXVECTOR2( vMIN.x, vMIN.z ), D3DXVECTOR2( vMAX.x, vMAX.z ) );
}

void CPageEtc::OnBnClickedButtonEffDelete()
{
	if ( !m_pGLLand )	return;
	if ( !m_pGLLand->GetLevelEtcFunc() )	return;

	int nSelect = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if( nSelect == -1 ) return;
	if( nSelect >= (int)m_pGLLand->GetLevelEtcFunc()->m_vecLandEffect.size() )	return;

	m_pGLLand->GetLevelEtcFunc()->m_vecLandEffect.erase( m_pGLLand->GetLevelEtcFunc()->m_vecLandEffect.begin() + nSelect );
	EffShow();
}

void CPageEtc::OnBnClickedButtonEffPosscreen()
{
	D3DXVECTOR3 vPOS = CEditorLevelView::GetView()->m_vPos;
	D3DXVECTOR3 vMIN( vPOS.x - 10.0f, vPOS.y - 10.0f, vPOS.z - 10.0f );
	D3DXVECTOR3 vMAX( vPOS.x + 10.0f, vPOS.y + 10.0f, vPOS.z + 10.0f );

	CEditorLevelView::GetView()->EditBoxSet( TRUE );
	CEditorLevelView::GetView()->m_DxEditBox.SetBox( vMAX, vMIN );
	JumpTo( D3DXVECTOR2( vMIN.x, vMIN.z ), D3DXVECTOR2( vMAX.x, vMAX.z ) );
}

void CPageEtc::OnBnClickedButtonEffClose()
{
	EffShow();
}

void CPageEtc::OnBnClickedButtonEffSave()
{
	if ( !m_pGLLand )	return;
	if ( !m_pGLLand->GetLevelEtcFunc() )	return;

	D3DXVECTOR3 vMin = CEditorLevelView::GetView()->m_DxEditBox.m_vMin;
	D3DXVECTOR3 vMax = CEditorLevelView::GetView()->m_DxEditBox.m_vMax;

	SLANDEFFECT landEff;
	landEff.emLandEffectType = (EMLANDEFFECT_TYPE)GetWin_Combo_Sel( this, IDC_COMBO_EFF_TYPE );
	landEff.fValue = GetWin_Num_float( this, IDC_EDIT_EFF_VAL );
	landEff.vMinPos.x = vMin.x;
	landEff.vMinPos.y = vMin.z;
	landEff.vMaxPos.x = vMax.x;
	landEff.vMaxPos.y = vMax.z;

	int nINDEX = GetWin_Num_int( this, IDC_EDIT_EFF_NUM );
	if ( nINDEX == -1 )
	{
		m_pGLLand->GetLevelEtcFunc()->m_vecLandEffect.push_back( landEff );
	}else{
		if ( nINDEX < (int)m_pGLLand->GetLevelEtcFunc()->m_vecLandEffect.size () )
			m_pGLLand->GetLevelEtcFunc()->m_vecLandEffect[nINDEX] = landEff;
	}

	EffShow();
}

void CPageEtc::OnBnClickedButtonSave()
{
	if ( DataSave() )
	{
		if ( m_pSheetTab )
			m_pSheetTab->PageActive( LEVELEDIT_PAGE_MAIN );
	}
}

void CPageEtc::OnBnClickedButtonClose()
{
	if ( m_pSheetTab )
		m_pSheetTab->PageActive( LEVELEDIT_PAGE_MAIN );
}
